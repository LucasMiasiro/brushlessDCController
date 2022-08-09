#pragma once
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "config.h"
// #include "driver/gpio.h"

class SMC2{
private:
    float dt{SYSTEM_SAMPLE_PERIOD_MS/1000.0f};
    float minIn{RPM_MIN};
    float minOut{0.0f};
    float maxOut{PWM_DELTA};
    float c1{BLDC_C1};
    float Fstar{BLDC_FSTAR};
    float FI{BLDC_FI};
    float ref{0.0f};
    float meas{0.0f};
    float e{0.0f};
    float e_prev{0.0f};
    float e_sign{0.0f};
    float sigma{0.0f};
    float F1{0.0f};
    float F2_dot{0.0f};
    float F2_dot_prev{0.0f};
    float F2{0.0f};
    float k_filter{BLDC_ERROR_FILTER_K};
    bool isNew = false;
    float out_prev = 0.0f;
    float out = 0.0f;
    // float dt_accum{0.0f};
    float offset{PWM_MIN};
    float pwm_idle{PWM_IDLE};
    uint16_t sanityCount = 0;
    uint8_t boundOut();
public:
    SMC2(){};
    void update(float ref, float meas, float isNew);
    float get();
};