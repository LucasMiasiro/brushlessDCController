#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_log.h"
#include "driver/pulse_cnt.h"
#include "encoder-reader.h"
#include "esp_timer.h"
#include <iostream>

static bool pcnt_on_reach(pcnt_unit_t *unit, const pcnt_watch_event_data_t *edata, void *user_ctx)
{
    // BaseType_t high_task_wakeup;
    cbData* data = (cbData*) user_ctx;
    // QueueHandle_t queue = data->queue;
    // xQueueSendFromISR(queue, &(edata->watch_point_value), &high_task_wakeup);

    ESP_ERROR_CHECK(pcnt_unit_clear_count(unit));
    ESP_ERROR_CHECK(pcnt_unit_clear_count(data->unit));
    data->homeWasSet = true;
    // std::cout << "Teste ---------------------------\n";

    // return (high_task_wakeup == pdTRUE);
    return true;
}

static bool pcnt_on_reach_loopCount(pcnt_unit_t *unit, const pcnt_watch_event_data_t *edata, void *user_ctx)
{
    // BaseType_t high_task_wakeup;
    cbData* data2 = (cbData*) user_ctx;
    // QueueHandle_t queue = data->queue;
    // xQueueSendFromISR(queue, &(edata->watch_point_value), &high_task_wakeup);
    int count = 0;

    pcnt_unit_get_count(unit, &count);
    ESP_ERROR_CHECK(pcnt_unit_clear_count(unit));

    if (count > 0){
        data2->loopCount = data2->loopCount + 1;
    } else {
        data2->loopCount = data2->loopCount - 1;
    }

    // return (high_task_wakeup == pdTRUE);
    return true;
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

    // // RPM Mode //TODO RETOMAR
    // ESP_ERROR_CHECK(pcnt_channel_set_edge_action(pcnt_chan_a, PCNT_CHANNEL_EDGE_ACTION_INCREASE, PCNT_CHANNEL_EDGE_ACTION_HOLD));
    // ESP_ERROR_CHECK(pcnt_channel_set_level_action(pcnt_chan_a, PCNT_CHANNEL_LEVEL_ACTION_KEEP, PCNT_CHANNEL_LEVEL_ACTION_KEEP));
    // ESP_ERROR_CHECK(pcnt_channel_set_edge_action(pcnt_chan_b, PCNT_CHANNEL_EDGE_ACTION_HOLD, PCNT_CHANNEL_EDGE_ACTION_HOLD));
    // ESP_ERROR_CHECK(pcnt_channel_set_level_action(pcnt_chan_b, PCNT_CHANNEL_LEVEL_ACTION_KEEP, PCNT_CHANNEL_LEVEL_ACTION_KEEP));

    #if CL_CONTROL_STEPMOTOR
    // Encoder Mode
    ESP_ERROR_CHECK(pcnt_channel_set_edge_action(pcnt_chan_a, PCNT_CHANNEL_EDGE_ACTION_DECREASE, PCNT_CHANNEL_EDGE_ACTION_INCREASE));
    ESP_ERROR_CHECK(pcnt_channel_set_level_action(pcnt_chan_a, PCNT_CHANNEL_LEVEL_ACTION_KEEP, PCNT_CHANNEL_LEVEL_ACTION_INVERSE));
    ESP_ERROR_CHECK(pcnt_channel_set_edge_action(pcnt_chan_b, PCNT_CHANNEL_EDGE_ACTION_INCREASE, PCNT_CHANNEL_EDGE_ACTION_DECREASE));
    ESP_ERROR_CHECK(pcnt_channel_set_level_action(pcnt_chan_b, PCNT_CHANNEL_LEVEL_ACTION_KEEP, PCNT_CHANNEL_LEVEL_ACTION_INVERSE));
    #else
    // Count Pulses Mode
    ESP_ERROR_CHECK(pcnt_channel_set_edge_action(pcnt_chan_a, PCNT_CHANNEL_EDGE_ACTION_INCREASE, PCNT_CHANNEL_EDGE_ACTION_HOLD));
    ESP_ERROR_CHECK(pcnt_channel_set_level_action(pcnt_chan_a, PCNT_CHANNEL_LEVEL_ACTION_INVERSE, PCNT_CHANNEL_LEVEL_ACTION_KEEP));
    ESP_ERROR_CHECK(pcnt_channel_set_edge_action(pcnt_chan_b, PCNT_CHANNEL_EDGE_ACTION_HOLD, PCNT_CHANNEL_EDGE_ACTION_HOLD));
    ESP_ERROR_CHECK(pcnt_channel_set_level_action(pcnt_chan_b, PCNT_CHANNEL_LEVEL_ACTION_KEEP, PCNT_CHANNEL_LEVEL_ACTION_KEEP));
    #endif

    ESP_ERROR_CHECK(pcnt_unit_add_watch_point(pcnt_unit, ECD_LOOPCOUNT));
    ESP_ERROR_CHECK(pcnt_unit_add_watch_point(pcnt_unit, -ECD_LOOPCOUNT));
    pcnt_event_callbacks_t cbs2 = {.on_reach = pcnt_on_reach_loopCount,};
    queue2 = xQueueCreate(10, sizeof(int));
    data2 = {.queue = queue2, .unit = pcnt_unit, .homeWasSet = false, .loopCount = 0};
    ESP_ERROR_CHECK(pcnt_unit_register_event_callbacks(pcnt_unit, &cbs2, &data2));

    if (zeroDetection) {

        ESP_ERROR_CHECK(pcnt_new_unit(&unit_config, &pcnt_unit_zero));
        ESP_ERROR_CHECK(pcnt_unit_set_glitch_filter(pcnt_unit_zero, &filter_config));
        pcnt_chan_config_t chan_zero_config = {.edge_gpio_num = GPIOZERO, .level_gpio_num = -1,};
        pcnt_channel_handle_t pcnt_chan_zero = NULL;
        ESP_ERROR_CHECK(pcnt_new_channel(pcnt_unit_zero, &chan_zero_config, &pcnt_chan_zero));
        ESP_ERROR_CHECK(pcnt_channel_set_edge_action(pcnt_chan_zero, PCNT_CHANNEL_EDGE_ACTION_INCREASE, PCNT_CHANNEL_EDGE_ACTION_HOLD)); // Count rising edge
        ESP_ERROR_CHECK(pcnt_channel_set_level_action(pcnt_chan_zero, PCNT_CHANNEL_LEVEL_ACTION_KEEP, PCNT_CHANNEL_LEVEL_ACTION_KEEP));

        ESP_ERROR_CHECK(pcnt_unit_add_watch_point(pcnt_unit_zero, 1));
        pcnt_event_callbacks_t cbs = {.on_reach = pcnt_on_reach,};
        queue = xQueueCreate(10, sizeof(int));
        data = {.queue = queue, .unit = pcnt_unit, .homeWasSet = false, .loopCount = 0};
        ESP_ERROR_CHECK(pcnt_unit_register_event_callbacks(pcnt_unit_zero, &cbs, &data));

        ESP_ERROR_CHECK(pcnt_unit_enable(pcnt_unit_zero));
        ESP_ERROR_CHECK(pcnt_unit_clear_count(pcnt_unit_zero));
        ESP_ERROR_CHECK(pcnt_unit_start(pcnt_unit_zero));

    }

    ESP_ERROR_CHECK(pcnt_unit_enable(pcnt_unit));
    ESP_ERROR_CHECK(pcnt_unit_clear_count(pcnt_unit));
    ESP_ERROR_CHECK(pcnt_unit_start(pcnt_unit));
}

void encoderReader::getCurrAngle(float *currAngle, bool setZero, bool* homeWasSet)
{
    getCurrAngle(currAngle, setZero);
    *homeWasSet = data.homeWasSet;
}

void encoderReader::getCurrAngle(float *currAngle, bool setZero)
{
    if (setZero) {
        pcnt_unit_clear_count(pcnt_unit);
        data2.loopCount = 0;
    }
    pcnt_unit_get_count(pcnt_unit, &count);
    #if CL_CONTROL_STEPMOTOR
    *currAngle = (float)count/4.0f*(360.0f/ECD_TICKS);
    #else
    *currAngle = (float)(count + (data2.loopCount * ECD_LOOPCOUNT))*STEPS_PER_DEG_STEPMOTOR;
    #endif
}