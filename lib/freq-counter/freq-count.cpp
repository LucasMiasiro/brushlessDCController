#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_log.h"
#include "driver/pulse_cnt.h"
#include "freq-count.h"
#include "esp_timer.h"
#include <iostream>

static const char *TAG = "RPM";

static int64_t start[3] = {esp_timer_get_time(), esp_timer_get_time(), esp_timer_get_time()};
static int64_t dt[3] = {RPM_DT_MAX, RPM_DT_MAX, RPM_DT_MAX};
static bool rpmCurr_isNew[3] = {false, false, false};

static pcnt_unit_handle_t pcnt_unit = NULL, pcnt_unit1 = NULL, pcnt_unit2 = NULL;

// static bool pcnt_on_reach(pcnt_unit_handle_t unit, pcnt_watch_event_data_t *edata, void *user_ctx)
static bool pcnt_on_reach(pcnt_unit_t *unit, const pcnt_watch_event_data_t *edata, void *user_ctx)
{
    BaseType_t high_task_wakeup;
    QueueHandle_t queue = (QueueHandle_t)user_ctx;
    xQueueSendFromISR(queue, &(edata->watch_point_value), &high_task_wakeup);

    dt[0] = esp_timer_get_time() - start[0];
    if (dt[0] <= RPM_DT_MIN/RPM_COUNT_PER_REV) {
        dt[0] = RPM_DT_MIN/RPM_COUNT_PER_REV;
    } else if (dt[0] > RPM_DT_MAX/RPM_COUNT_PER_REV) {
        dt[0] = RPM_DT_MAX/RPM_COUNT_PER_REV;
    }

    start[0] = esp_timer_get_time();
    rpmCurr_isNew[0] = true;
    ESP_ERROR_CHECK(pcnt_unit_clear_count(unit));

    return (high_task_wakeup == pdTRUE);
}

// static bool pcnt_on_reach1(pcnt_unit_handle_t unit, pcnt_watch_event_data_t *edata, void *user_ctx)
static bool pcnt_on_reach1(pcnt_unit_t *unit, const pcnt_watch_event_data_t *edata, void *user_ctx)
{
    BaseType_t high_task_wakeup;
    QueueHandle_t queue = (QueueHandle_t)user_ctx;
    xQueueSendFromISR(queue, &(edata->watch_point_value), &high_task_wakeup);

    dt[1] = esp_timer_get_time() - start[1];
    if (dt[1] <= RPM_DT_MIN/RPM_COUNT_PER_REV) {
        dt[1] = RPM_DT_MIN/RPM_COUNT_PER_REV;
    } else if (dt[1] > RPM_DT_MAX/RPM_COUNT_PER_REV) {
        dt[1] = RPM_DT_MAX/RPM_COUNT_PER_REV;
    }

    start[1] = esp_timer_get_time();
    rpmCurr_isNew[1] = true;
    ESP_ERROR_CHECK(pcnt_unit_clear_count(unit));

    return (high_task_wakeup == pdTRUE);
}

// static bool pcnt_on_reach2(pcnt_unit_handle_t unit, pcnt_watch_event_data_t *edata, void *user_ctx)
static bool pcnt_on_reach2(pcnt_unit_t *unit, const pcnt_watch_event_data_t *edata, void *user_ctx)
{
    BaseType_t high_task_wakeup;
    QueueHandle_t queue = (QueueHandle_t)user_ctx;
    xQueueSendFromISR(queue, &(edata->watch_point_value), &high_task_wakeup);

    dt[2] = esp_timer_get_time() - start[2];
    if (dt[2] <= RPM_DT_MIN/RPM_COUNT_PER_REV) {
        dt[2] = RPM_DT_MIN/RPM_COUNT_PER_REV;
    } else if (dt[2] > RPM_DT_MAX/RPM_COUNT_PER_REV) {
        dt[2] = RPM_DT_MAX/RPM_COUNT_PER_REV;
    }

    start[2] = esp_timer_get_time();
    rpmCurr_isNew[2] = true;
    ESP_ERROR_CHECK(pcnt_unit_clear_count(unit));

    return (high_task_wakeup == pdTRUE);
}

void rpmCounter::setup()
{
    pcnt_unit_config_t unit_config = {.low_limit = -RPM_MAX_COUNT, .high_limit = RPM_MAX_COUNT,};
    ESP_ERROR_CHECK(pcnt_new_unit(&unit_config, &pcnt_unit));
    ESP_ERROR_CHECK(pcnt_new_unit(&unit_config, &pcnt_unit1));
    ESP_ERROR_CHECK(pcnt_new_unit(&unit_config, &pcnt_unit2));

    pcnt_glitch_filter_config_t filter_config = {.max_glitch_ns = 200};
    ESP_ERROR_CHECK(pcnt_unit_set_glitch_filter(pcnt_unit, &filter_config));
    ESP_ERROR_CHECK(pcnt_unit_set_glitch_filter(pcnt_unit1, &filter_config));
    ESP_ERROR_CHECK(pcnt_unit_set_glitch_filter(pcnt_unit2, &filter_config));

    pcnt_chan_config_t chan_a_config = {.edge_gpio_num = GPIO, .level_gpio_num = -1,};
    pcnt_chan_config_t chan_a_config1 = {.edge_gpio_num = GPIO1, .level_gpio_num = -1,};
    pcnt_chan_config_t chan_a_config2 = {.edge_gpio_num = GPIO2, .level_gpio_num = -1,};
    pcnt_channel_handle_t pcnt_chan_a = NULL, pcnt_chan_a1 = NULL, pcnt_chan_a2 = NULL;
    ESP_ERROR_CHECK(pcnt_new_channel(pcnt_unit, &chan_a_config, &pcnt_chan_a));
    ESP_ERROR_CHECK(pcnt_new_channel(pcnt_unit1, &chan_a_config1, &pcnt_chan_a1));
    ESP_ERROR_CHECK(pcnt_new_channel(pcnt_unit2, &chan_a_config2, &pcnt_chan_a2));

    ESP_ERROR_CHECK(pcnt_channel_set_edge_action(pcnt_chan_a, PCNT_CHANNEL_EDGE_ACTION_INCREASE, PCNT_CHANNEL_EDGE_ACTION_HOLD));
    ESP_ERROR_CHECK(pcnt_channel_set_level_action(pcnt_chan_a, PCNT_CHANNEL_LEVEL_ACTION_KEEP, PCNT_CHANNEL_LEVEL_ACTION_KEEP));

    ESP_ERROR_CHECK(pcnt_channel_set_edge_action(pcnt_chan_a1, PCNT_CHANNEL_EDGE_ACTION_INCREASE, PCNT_CHANNEL_EDGE_ACTION_HOLD));
    ESP_ERROR_CHECK(pcnt_channel_set_level_action(pcnt_chan_a1, PCNT_CHANNEL_LEVEL_ACTION_KEEP, PCNT_CHANNEL_LEVEL_ACTION_KEEP));

    ESP_ERROR_CHECK(pcnt_channel_set_edge_action(pcnt_chan_a2, PCNT_CHANNEL_EDGE_ACTION_INCREASE, PCNT_CHANNEL_EDGE_ACTION_HOLD));
    ESP_ERROR_CHECK(pcnt_channel_set_level_action(pcnt_chan_a2, PCNT_CHANNEL_LEVEL_ACTION_KEEP, PCNT_CHANNEL_LEVEL_ACTION_KEEP));

    ESP_ERROR_CHECK(pcnt_unit_add_watch_point(pcnt_unit, RPM_COUNT));
    ESP_ERROR_CHECK(pcnt_unit_add_watch_point(pcnt_unit1, RPM_COUNT));
    ESP_ERROR_CHECK(pcnt_unit_add_watch_point(pcnt_unit2, RPM_COUNT));

    pcnt_event_callbacks_t cbs = {.on_reach = pcnt_on_reach,};
    pcnt_event_callbacks_t cbs1 = {.on_reach = pcnt_on_reach1,};
    pcnt_event_callbacks_t cbs2 = {.on_reach = pcnt_on_reach2,};
    queue = xQueueCreate(10, sizeof(int));
    queue1 = xQueueCreate(10, sizeof(int));
    queue2 = xQueueCreate(10, sizeof(int));
    ESP_ERROR_CHECK(pcnt_unit_register_event_callbacks(pcnt_unit, &cbs, queue));
    ESP_ERROR_CHECK(pcnt_unit_register_event_callbacks(pcnt_unit1, &cbs1, queue1));
    ESP_ERROR_CHECK(pcnt_unit_register_event_callbacks(pcnt_unit2, &cbs2, queue2));

    ESP_ERROR_CHECK(pcnt_unit_enable(pcnt_unit));
    ESP_ERROR_CHECK(pcnt_unit_clear_count(pcnt_unit));
    ESP_ERROR_CHECK(pcnt_unit_start(pcnt_unit));

    ESP_ERROR_CHECK(pcnt_unit_enable(pcnt_unit1));
    ESP_ERROR_CHECK(pcnt_unit_clear_count(pcnt_unit1));
    ESP_ERROR_CHECK(pcnt_unit_start(pcnt_unit1));

    ESP_ERROR_CHECK(pcnt_unit_enable(pcnt_unit2));
    ESP_ERROR_CHECK(pcnt_unit_clear_count(pcnt_unit2));
    ESP_ERROR_CHECK(pcnt_unit_start(pcnt_unit2));
}


void rpmCounter::getRPM(rpmState *rpm)
{
    for (uint8_t i = 0; i < N_BLDC; i++){
        rpm[i].rpmCurr = (RPM_COUNT*60.0f*1000000.0f)/(dt[i]*RPM_COUNT_PER_REV);
        if (rpm[i].rpmCurr <= RPM_MIN*1.05) {
            rpm[i].rpmCurr = 0;
        }
        rpm[i].rpmCurr_isNew = rpmCurr_isNew[i];
        rpmCurr_isNew[i] = false;
    }
}
