#pragma once
#include "config.h"
#include "driver/pulse_cnt.h"
#include "freertos/queue.h"
#include "main.h"

class rpmCounter {
private:
    void setup();
    const uint8_t GPIO{RPM0_GPIO};
    const uint8_t GPIO1{RPM1_GPIO};
    const uint8_t GPIO2{RPM2_GPIO};
    QueueHandle_t queue;
    // static bool pcnt_on_reach(pcnt_unit_handle_t unit, pcnt_watch_event_data_t *edata, void *user_ctx);
public:
    rpmCounter() {
        setup();
    };
    void getRPM(rpmState *rpm);
};