#pragma once
#include "config.h"
#include "driver/pulse_cnt.h"
#include "freertos/queue.h"
#include "main.h"

class rpmCounter {
private:
    void setup();
    QueueHandle_t queue;
    // static bool pcnt_on_reach(pcnt_unit_handle_t unit, pcnt_watch_event_data_t *edata, void *user_ctx);
public:
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