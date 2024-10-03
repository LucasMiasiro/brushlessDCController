#include "driver/gpio.h"
// #include "driver/mcpwm.h"
#include "gpio-handler.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/mcpwm_prelude.h"

builtin_led::builtin_led(){
    gpio_config_t io_conf = {};

    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pin_bit_mask = (1ULL << BUILTIN_LED);
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
    gpio_config(&io_conf);
}

void builtin_led::set_level(bool state){
    gpio_set_level(BUILTIN_LED, state);
}

void builtin_led::blink(int n, bool endHigh){
    if (endHigh){
        for (int i = 0; i < n; i++){
            set_level(0);
            // vTaskDelay(1000.0f/SYSTEM_SAMPLE_PERIOD_MS);
            vTaskDelay(10.f/portTICK_PERIOD_MS);
            set_level(1);
            // vTaskDelay(1000.0f/SYSTEM_SAMPLE_PERIOD_MS);
            vTaskDelay(10.f/portTICK_PERIOD_MS);
        }
    } else {
        for (int i = 0; i < n; i++){
            set_level(1);
            // vTaskDelay(1000.0f/SYSTEM_SAMPLE_PERIOD_MS);
            vTaskDelay(10.f/portTICK_PERIOD_MS);
            set_level(0);
            // vTaskDelay(1000.0f/SYSTEM_SAMPLE_PERIOD_MS);
            vTaskDelay(10.f/portTICK_PERIOD_MS);
        }
    }
}

uint8_t bldc::n_count = 0;

void bldc::setup(){
    uint8_t id = 0;
    n = n_count++;

    if (n >= 2) {
        id = 1;
    }


    mcpwm_timer_config_t timer_config = {
        .group_id = id,
        .clk_src = MCPWM_TIMER_CLK_SRC_DEFAULT,
        .resolution_hz = BLDC_TIMER_RESOLUTION,
        .count_mode = MCPWM_TIMER_COUNT_MODE_UP,
        .period_ticks = BLDC_TIMER_PERIOD,
    };
    mcpwm_new_timer(&timer_config, &timer);

    mcpwm_operator_config_t operator_config = {
        .group_id = id, // must be in the same group to the timer
    };
    mcpwm_new_operator(&operator_config, &oper);  
    mcpwm_operator_connect_timer(oper, timer);

    mcpwm_comparator_config_t comparator_config = {
        .flags = {.update_cmp_on_tez = true,},
    };
    mcpwm_new_comparator(oper, &comparator_config, &comparator);

    mcpwm_generator_config_t generator_config = {
        .gen_gpio_num = bldc_gpio,
    };
    mcpwm_new_generator(oper, &generator_config, &generator);

    mcpwm_comparator_set_compare_value(comparator, 0);

    // go high on counter empty
    mcpwm_generator_set_actions_on_timer_event(generator,
                    MCPWM_GEN_TIMER_EVENT_ACTION(MCPWM_TIMER_DIRECTION_UP, MCPWM_TIMER_EVENT_EMPTY, MCPWM_GEN_ACTION_HIGH),
                    MCPWM_GEN_TIMER_EVENT_ACTION_END());
    
    // go low on compare threshold
    mcpwm_generator_set_actions_on_compare_event(generator,
                    MCPWM_GEN_COMPARE_EVENT_ACTION(MCPWM_TIMER_DIRECTION_UP, comparator, MCPWM_GEN_ACTION_LOW),
                    MCPWM_GEN_COMPARE_EVENT_ACTION_END());
    
    mcpwm_timer_enable(timer);

    mcpwm_timer_start_stop(timer, MCPWM_TIMER_START_NO_STOP);
}

void bldc::setPWM(uint16_t pwmDes){
    mcpwm_comparator_set_compare_value(comparator, pwmDes);
}