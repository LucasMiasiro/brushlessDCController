#pragma once
#include "config.h"
#include "driver/pulse_cnt.h"
#include "freertos/queue.h"
#include "main.h"

class encoderReader {
private:
    void setup();
    const uint8_t GPIOA{ECD0A_GPIO};
    const uint8_t GPIOB{ECD0B_GPIO};
    QueueHandle_t queue;
public:
    encoderReader() {
        setup();
    };
    void getCurrAngle(float *currAngle, bool setZero);
};