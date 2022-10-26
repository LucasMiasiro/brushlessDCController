#include "driver/gpio.h"
#include "driver/mcpwm_prelude.h"
#include "step-motor.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

void stepMotor::setup(){
    mcpwm_timer_config_t timer_config = {
        .group_id = 0,
        .clk_src = MCPWM_TIMER_CLK_SRC_DEFAULT,
        .resolution_hz = SM_TIMER_RESOLUTION,
        .count_mode = MCPWM_TIMER_COUNT_MODE_UP,
        .period_ticks = SM_TIMER_PERIOD,
    };
    mcpwm_new_timer(&timer_config, &timer);

    mcpwm_operator_config_t operator_config = {
        .group_id = 0, // must be in the same group to the timer
    };
    mcpwm_new_operator(&operator_config, &oper);  
    mcpwm_operator_connect_timer(oper, timer);

    mcpwm_comparator_config_t comparator_config = {
        .flags = {.update_cmp_on_tez = true,},
    };
    mcpwm_new_comparator(oper, &comparator_config, &comparator);

    mcpwm_generator_config_t generator_config = {
        .gen_gpio_num = SM_PULSE_GPIO,
    };
    mcpwm_new_generator(oper, &generator_config, &generator);

    mcpwm_comparator_set_compare_value(comparator, SM_PULSE_WIDTH);

    // go high on counter empty
    mcpwm_generator_set_actions_on_timer_event(generator,
                    MCPWM_GEN_TIMER_EVENT_ACTION(MCPWM_TIMER_DIRECTION_UP, MCPWM_TIMER_EVENT_EMPTY, MCPWM_GEN_ACTION_HIGH),
                    MCPWM_GEN_TIMER_EVENT_ACTION_END());
    
    // go low on compare threshold
    mcpwm_generator_set_actions_on_compare_event(generator,
                    MCPWM_GEN_COMPARE_EVENT_ACTION(MCPWM_TIMER_DIRECTION_UP, comparator, MCPWM_GEN_ACTION_LOW),
                    MCPWM_GEN_COMPARE_EVENT_ACTION_END());
    
    mcpwm_timer_enable(timer);

    gpio_config_t io_conf = {};
    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pin_bit_mask = (1ULL << SM_DIR_GPIO);
    io_conf.pull_down_en = GPIO_PULLDOWN_ENABLE;
    io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
    gpio_config(&io_conf);

    // mcpwm_timer_start_stop(timer, MCPWM_TIMER_START_NO_STOP);
}

void stepMotor::setUp(){
    gpio_set_level(dir_gpio, 0);
}

void stepMotor::setDown(){
    gpio_set_level(dir_gpio, 1);
}

void stepMotor::go(){
    mcpwm_timer_start_stop(timer, MCPWM_TIMER_START_NO_STOP);
}

void stepMotor::stop(){
    mcpwm_timer_start_stop(timer, MCPWM_TIMER_STOP_EMPTY);
    gpio_set_level(dir_gpio, 0);
}