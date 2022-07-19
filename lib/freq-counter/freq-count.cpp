#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_log.h"
#include "driver/pulse_cnt.h"
#include "freq-count.h"
#include "esp_timer.h"
#include <iostream>

static const char *TAG = "RPM";

static int64_t start = esp_timer_get_time();
static int64_t dt = 1000000;

static pcnt_unit_handle_t pcnt_unit = NULL;

//TODO: resolver o fato dessa função não funcionar pra múltiplos motores
static bool pcnt_on_reach(pcnt_unit_handle_t unit, pcnt_watch_event_data_t *edata, void *user_ctx)
{
    BaseType_t high_task_wakeup;
    QueueHandle_t queue = (QueueHandle_t)user_ctx;
    // send event data to queue, from this interrupt callback
    xQueueSendFromISR(queue, &(edata->watch_point_value), &high_task_wakeup);

    dt = esp_timer_get_time() - start;
    if (dt <= 1) {
        dt = 1;
    } else if (dt > 1000000) {
        dt = 1000000;
    }

    start = esp_timer_get_time();
    ESP_ERROR_CHECK(pcnt_unit_clear_count(pcnt_unit));

    return (high_task_wakeup == pdTRUE);
}

void rpmCounter::setup()
{
    pcnt_unit_config_t unit_config = {
        .low_limit = -RPM_MAX_COUNT,
        .high_limit = RPM_MAX_COUNT,
    };
    ESP_ERROR_CHECK(pcnt_new_unit(&unit_config, &pcnt_unit));

    pcnt_glitch_filter_config_t filter_config = {.max_glitch_ns = 500};
    ESP_ERROR_CHECK(pcnt_unit_set_glitch_filter(pcnt_unit, &filter_config));

    pcnt_chan_config_t chan_a_config = {
        .edge_gpio_num = GPIO,
        .level_gpio_num = -1,
    };
    pcnt_channel_handle_t pcnt_chan_a = NULL;
    ESP_ERROR_CHECK(pcnt_new_channel(pcnt_unit, &chan_a_config, &pcnt_chan_a));

    ESP_ERROR_CHECK(pcnt_channel_set_edge_action(pcnt_chan_a, PCNT_CHANNEL_EDGE_ACTION_INCREASE, PCNT_CHANNEL_EDGE_ACTION_HOLD));
    ESP_ERROR_CHECK(pcnt_channel_set_level_action(pcnt_chan_a, PCNT_CHANNEL_LEVEL_ACTION_KEEP, PCNT_CHANNEL_LEVEL_ACTION_KEEP));

    ESP_ERROR_CHECK(pcnt_unit_add_watch_point(pcnt_unit, RPM_COUNT));

    pcnt_event_callbacks_t cbs = {.on_reach = pcnt_on_reach,};
    queue = xQueueCreate(10, sizeof(int));
    ESP_ERROR_CHECK(pcnt_unit_register_event_callbacks(pcnt_unit, &cbs, queue));

    ESP_ERROR_CHECK(pcnt_unit_enable(pcnt_unit));
    ESP_ERROR_CHECK(pcnt_unit_clear_count(pcnt_unit));
    ESP_ERROR_CHECK(pcnt_unit_start(pcnt_unit));
}


void rpmCounter::getRPM(rpmState *rpm)
{
    rpm->rpmCurr = (RPM_COUNT*60.0f*1000000.0f)/dt;
    if (rpm->rpmCurr < 200) {
        rpm->rpmCurr = 0;
    }
    rpm->rpmCurr_isNew = true;
}
