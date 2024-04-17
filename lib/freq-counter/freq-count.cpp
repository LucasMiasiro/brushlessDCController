#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_log.h"
#include "driver/pulse_cnt.h"
#include "freq-count.h"
#include "esp_timer.h"
#include <iostream>

static const char *TAG = "RPM";


// static pcnt_unit_handle_t pcnt_unit_RPM = NULL;

// static bool pcnt_on_reach(pcnt_unit_handle_t unit, pcnt_watch_event_data_t *edata, void *user_ctx)

// bool rpmCounter::pcnt_on_reach(pcnt_unit_t *unit, const pcnt_watch_event_data_t *edata, void *user_ctx)
// static bool pcnt_on_reach(pcnt_unit_t *unit, const pcnt_watch_event_data_t *edata, void *user_ctx)
// {
//     BaseType_t high_task_wakeup;
//     QueueHandle_t queue = (QueueHandle_t)user_ctx;
//     xQueueSendFromISR(queue, &(edata->watch_point_value), &high_task_wakeup);

//     dt_RPM = esp_timer_get_time() - start_RPM;
//     if (dt_RPM <= RPM_DT_MIN/RPM_COUNT_PER_REV) {
//         dt_RPM = RPM_DT_MIN/RPM_COUNT_PER_REV;
//     } else if (dt_RPM > RPM_DT_MAX/RPM_COUNT_PER_REV) {
//         dt_RPM = RPM_DT_MAX/RPM_COUNT_PER_REV;
//     }

//     start_RPM = esp_timer_get_time();
//     rpmCurr_isNew = true;
//     ESP_ERROR_CHECK(pcnt_unit_clear_count(unit));

//     return (high_task_wakeup == pdTRUE);
// }

static bool pcnt_on_reach(pcnt_unit_t *unit, const pcnt_watch_event_data_t *edata, void *user_ctx)
{
    BaseType_t high_task_wakeup;
    cbDataRPM* data = (cbDataRPM*) user_ctx;
    QueueHandle_t queue = data->queue;
    rpmCounter *self = data->rpm;

    xQueueSendFromISR(queue, &(edata->watch_point_value), &high_task_wakeup);

    self->dt_RPM = esp_timer_get_time() - self->start_RPM;
    if (self->dt_RPM <= RPM_DT_MIN/RPM_COUNT_PER_REV) {
        self->dt_RPM = RPM_DT_MIN/RPM_COUNT_PER_REV;
    } else if (self->dt_RPM > RPM_DT_MAX/RPM_COUNT_PER_REV) {
        self->dt_RPM = RPM_DT_MAX/RPM_COUNT_PER_REV;
    }

    self->start_RPM = esp_timer_get_time();
    self->rpmCurr_isNew = true;
    ESP_ERROR_CHECK(pcnt_unit_clear_count(unit));

    return (high_task_wakeup == pdTRUE);
}




void rpmCounter::setup()
{
    pcnt_unit_config_t unit_config = {.low_limit = -RPM_MAX_COUNT, .high_limit = RPM_MAX_COUNT,};
    ESP_ERROR_CHECK(pcnt_new_unit(&unit_config, &pcnt_unit_RPM));

    pcnt_glitch_filter_config_t filter_config = {.max_glitch_ns = 200};
    ESP_ERROR_CHECK(pcnt_unit_set_glitch_filter(pcnt_unit_RPM, &filter_config));

    pcnt_chan_config_t chan_a_config = {.edge_gpio_num = GPIO, .level_gpio_num = -1,};
    pcnt_channel_handle_t pcnt_chan_a = NULL;
    ESP_ERROR_CHECK(pcnt_new_channel(pcnt_unit_RPM, &chan_a_config, &pcnt_chan_a));

    ESP_ERROR_CHECK(pcnt_channel_set_edge_action(pcnt_chan_a, PCNT_CHANNEL_EDGE_ACTION_INCREASE, PCNT_CHANNEL_EDGE_ACTION_HOLD));
    ESP_ERROR_CHECK(pcnt_channel_set_level_action(pcnt_chan_a, PCNT_CHANNEL_LEVEL_ACTION_KEEP, PCNT_CHANNEL_LEVEL_ACTION_KEEP));

    ESP_ERROR_CHECK(pcnt_unit_add_watch_point(pcnt_unit_RPM, RPM_COUNT));

    pcnt_event_callbacks_t cbs = {.on_reach = pcnt_on_reach,};
    queue = xQueueCreate(10, sizeof(int));
    data = {.queue = queue, .rpm = this};
    ESP_ERROR_CHECK(pcnt_unit_register_event_callbacks(pcnt_unit_RPM, &cbs, &data));

    ESP_ERROR_CHECK(pcnt_unit_enable(pcnt_unit_RPM));
    ESP_ERROR_CHECK(pcnt_unit_clear_count(pcnt_unit_RPM));
    ESP_ERROR_CHECK(pcnt_unit_start(pcnt_unit_RPM));
}


void rpmCounter::getRPM(rpmState *rpm)
{
    rpm->rpmCurr = (RPM_COUNT*60.0f*1000000.0f)/(dt_RPM*RPM_COUNT_PER_REV);
    if (rpm->rpmCurr <= RPM_MIN*1.05) {
        rpm->rpmCurr = 0;
    }
    rpm->rpmCurr_isNew = rpmCurr_isNew;
    rpmCurr_isNew = false;
}
