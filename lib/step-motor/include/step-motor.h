#pragma once
#include "driver/mcpwm_prelude.h"
#include "driver/gpio.h"
#include "config.h"

class stepMotor{
    void setup();
    mcpwm_timer_handle_t timer = NULL;
    mcpwm_oper_handle_t oper = NULL;
    mcpwm_cmpr_handle_t comparator = NULL;
    mcpwm_gen_handle_t generator = NULL;
    gpio_num_t dir_gpio = SM_DIR_GPIO;
public:
    stepMotor() {setup();};
    void setUp();
    void setDown();
    void go();
    void stop();
};