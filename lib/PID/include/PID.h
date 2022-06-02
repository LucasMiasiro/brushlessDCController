#pragma once
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "config.h"
// #include "driver/gpio.h"

class PID{
private:
    float KP{BLDC_KP};
    float KI{BLDC_KI};
    float KD{0.0f};
    float FF{BLDC_FF};
    float RAMP{BLDC_RAMP};
    bool hasReachedMinIn = false;
    float dt{SYSTEM_SAMPLE_PERIOD_MS/1000.0f};
    float minOut{0.0f};
    float maxOut{PWM_DELTA};
    float minIn{RPM_MIN};
    float maxFF{10.0f};
    float Imax{BLDC_IMAX*PWM_DELTA};
    float ref{0.0f};
    float meas{0.0f};
    bool isNew = false;
    float out_prev = 0.0f;
    float out = 0.0f;
    float dt_accum{0.0f};
    float offset{PWM_MIN};
    float P{0.0f};
    float I{0.0f};
    float D{0.0f};
    float e{0.0f};
    float e_prev{0.0f};
    uint8_t boundOut();
    void resetI();
    void resetI(float);
public:
    PID(){};
    void update(float ref, float meas, float isNew);
    float get();
};