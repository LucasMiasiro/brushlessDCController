#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_log.h"
#include "driver/pulse_cnt.h"
#include "encoder-reader.h"
#include "esp_timer.h"
#include <iostream>

static const char *TAG = "ECD";

static pcnt_unit_handle_t pcnt_unit = NULL;
static int count;

static bool pcnt_on_reach(pcnt_unit_t *unit, const pcnt_watch_event_data_t *edata, void *user_ctx)
{
    BaseType_t high_task_wakeup;
    QueueHandle_t queue = (QueueHandle_t)user_ctx;
    xQueueSendFromISR(queue, &(edata->watch_point_value), &high_task_wakeup);

    // dt[0] = esp_timer_get_time() - start[0];
    // if (dt[0] <= RPM_DT_MIN/RPM_COUNT_PER_REV) {
    //     dt[0] = RPM_DT_MIN/RPM_COUNT_PER_REV;
    // } else if (dt[0] > RPM_DT_MAX/RPM_COUNT_PER_REV) {
    //     dt[0] = RPM_DT_MAX/RPM_COUNT_PER_REV;
    // }
    // start[0] = esp_timer_get_time();
    // rpmCurr_isNew[0] = true;
    // ESP_ERROR_CHECK(pcnt_unit_clear_count(unit));

    return (high_task_wakeup == pdTRUE);
}

void encoderReader::setup()
{
    pcnt_unit_config_t unit_config = {.low_limit = -ECD_MAX_COUNT, .high_limit = ECD_MAX_COUNT,};
    ESP_ERROR_CHECK(pcnt_new_unit(&unit_config, &pcnt_unit));

    pcnt_glitch_filter_config_t filter_config = {.max_glitch_ns = 200};
    ESP_ERROR_CHECK(pcnt_unit_set_glitch_filter(pcnt_unit, &filter_config));

    pcnt_chan_config_t chan_a_config = {.edge_gpio_num = GPIOA, .level_gpio_num = GPIOB,};
    pcnt_chan_config_t chan_b_config = {.edge_gpio_num = GPIOB, .level_gpio_num = GPIOA,};

    pcnt_channel_handle_t pcnt_chan_a = NULL, pcnt_chan_b = NULL;

    ESP_ERROR_CHECK(pcnt_new_channel(pcnt_unit, &chan_a_config, &pcnt_chan_a));
    ESP_ERROR_CHECK(pcnt_new_channel(pcnt_unit, &chan_b_config, &pcnt_chan_b));

    // RPM Mode //TODO RETOMAR
    ESP_ERROR_CHECK(pcnt_channel_set_edge_action(pcnt_chan_a, PCNT_CHANNEL_EDGE_ACTION_INCREASE, PCNT_CHANNEL_EDGE_ACTION_HOLD));
    ESP_ERROR_CHECK(pcnt_channel_set_level_action(pcnt_chan_a, PCNT_CHANNEL_LEVEL_ACTION_KEEP, PCNT_CHANNEL_LEVEL_ACTION_KEEP));
    ESP_ERROR_CHECK(pcnt_channel_set_edge_action(pcnt_chan_b, PCNT_CHANNEL_EDGE_ACTION_HOLD, PCNT_CHANNEL_EDGE_ACTION_HOLD));
    ESP_ERROR_CHECK(pcnt_channel_set_level_action(pcnt_chan_b, PCNT_CHANNEL_LEVEL_ACTION_KEEP, PCNT_CHANNEL_LEVEL_ACTION_KEEP));

    // // Encoder Mode
    // ESP_ERROR_CHECK(pcnt_channel_set_edge_action(pcnt_chan_a, PCNT_CHANNEL_EDGE_ACTION_DECREASE, PCNT_CHANNEL_EDGE_ACTION_INCREASE));
    // ESP_ERROR_CHECK(pcnt_channel_set_level_action(pcnt_chan_a, PCNT_CHANNEL_LEVEL_ACTION_KEEP, PCNT_CHANNEL_LEVEL_ACTION_INVERSE));
    // ESP_ERROR_CHECK(pcnt_channel_set_edge_action(pcnt_chan_b, PCNT_CHANNEL_EDGE_ACTION_INCREASE, PCNT_CHANNEL_EDGE_ACTION_DECREASE));
    // ESP_ERROR_CHECK(pcnt_channel_set_level_action(pcnt_chan_b, PCNT_CHANNEL_LEVEL_ACTION_KEEP, PCNT_CHANNEL_LEVEL_ACTION_INVERSE));

    // ESP_ERROR_CHECK(pcnt_unit_add_watch_point(pcnt_unit, RPM_COUNT));
    // pcnt_event_callbacks_t cbs = {.on_reach = pcnt_on_reach,};
    // queue = xQueueCreate(10, sizeof(int));
    // ESP_ERROR_CHECK(pcnt_unit_register_event_callbacks(pcnt_unit, &cbs, queue));

    ESP_ERROR_CHECK(pcnt_unit_enable(pcnt_unit));
    ESP_ERROR_CHECK(pcnt_unit_clear_count(pcnt_unit));
    ESP_ERROR_CHECK(pcnt_unit_start(pcnt_unit));

}


void encoderReader::getCurrAngle(float *currAngle, bool setZero)
{
    if (setZero) {
        pcnt_unit_clear_count(pcnt_unit);
    }
    pcnt_unit_get_count(pcnt_unit, &count);
    *currAngle = (float)count/4.0f*(360.0f/ECD_TICKS);
}