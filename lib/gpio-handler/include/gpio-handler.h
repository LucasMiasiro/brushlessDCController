#pragma once
#include "config.h"
#include "driver/mcpwm_prelude.h"
#include "driver/gpio.h"

class builtin_led{
public:
    builtin_led();
    void set_level(bool state);
    void blink(int n, bool endHigh);
};

class bldc{
    void setup();
    uint8_t n;
    static uint8_t n_count;
    mcpwm_timer_handle_t timer = NULL;
    mcpwm_oper_handle_t oper = NULL;
    mcpwm_cmpr_handle_t comparator = NULL;
    mcpwm_gen_handle_t generator = NULL;
public:
    gpio_num_t bldc_gpio = BLDC0_GPIO;
    bldc(gpio_num_t new_gpio) : bldc_gpio(new_gpio) {setup();};
    bldc() {setup();};
    void setPWM(uint16_t pwmDes);
};