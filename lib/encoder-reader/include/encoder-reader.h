#pragma once
#include "config.h"
#include "driver/pulse_cnt.h"
#include "freertos/queue.h"
#include "main.h"

class encoderReader {
private:
    void setup();
    const bool zeroDetection;
    pcnt_unit_handle_t pcnt_unit = NULL, pcnt_unit_zero = NULL;
    int count = 0;
    const uint8_t GPIOA{ECD0A_GPIO};
    const uint8_t GPIOB{ECD0B_GPIO};
    const uint8_t GPIOZERO{ECD0ZERO_GPIO};
    QueueHandle_t queue;
public:
    encoderReader() : zeroDetection(false) {setup();};
    encoderReader(const bool zeroDetection_in) : zeroDetection(zeroDetection_in) {setup();};
    void getCurrAngle(float *currAngle, bool setZero);
};