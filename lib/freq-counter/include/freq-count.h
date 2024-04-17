#pragma once
#include "config.h"
#include "esp_timer.h"
#include "driver/pulse_cnt.h"
#include "freertos/queue.h"
#include "main.h"

class rpmCounter;

struct cbDataRPM {
    QueueHandle_t queue;
    rpmCounter* rpm;
};

class rpmCounter {
private:
    void setup();
    QueueHandle_t queue;
    // static bool pcnt_on_reach(pcnt_unit_handle_t unit, pcnt_watch_event_data_t *edata, void *user_ctx);
public:
    cbDataRPM data = {.queue = NULL, .rpm = this};
    // bool pcnt_on_reach(pcnt_unit_t *unit, const pcnt_watch_event_data_t *edata, void *user_ctx);
    pcnt_unit_handle_t pcnt_unit_RPM = NULL;
    int64_t start_RPM = esp_timer_get_time();
    int64_t dt_RPM = RPM_DT_MAX;
    bool rpmCurr_isNew = false;
    uint8_t GPIO{RPM0_GPIO};
    rpmCounter() {
        setup();
    };
    rpmCounter(const uint8_t gpio) {
        this->GPIO = gpio;
        setup();
    };
    void getRPM(rpmState *rpm);
};

