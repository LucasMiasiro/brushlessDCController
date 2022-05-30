#include "driver/gpio.h"
#include "driver/mcpwm.h"
#include "gpio-handler.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

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
            vTaskDelay(1000.0f/SYSTEM_SAMPLE_PERIOD_MS);
            set_level(1);
            vTaskDelay(1000.0f/SYSTEM_SAMPLE_PERIOD_MS);
        }
    } else {
        for (int i = 0; i < n; i++){
            set_level(1);
            vTaskDelay(1000.0f/SYSTEM_SAMPLE_PERIOD_MS);
            set_level(0);
            vTaskDelay(1000.0f/SYSTEM_SAMPLE_PERIOD_MS);
        }
    }
}

bldc::bldc(){
    setup();
}

void bldc::setup(){
    mcpwm_gpio_init(BLDC0_UNIT, BLDC0_IO, BLDC0_GPIO);

    mcpwm_config_t pwm_config0 = {
        .frequency = BLDC0_FREQ,
        .cmpr_a = 0,     // Initial duty cycle of PWM0A
        .cmpr_b = 0,     // Initial duty cycle of PWM0B
        .duty_mode = MCPWM_DUTY_MODE_0,
        .counter_mode = MCPWM_UP_COUNTER,
    };

    mcpwm_init(BLDC0_UNIT, BLDC0_TIMER, &pwm_config0);
}

void bldc::setPWM(uint16_t pwmDes){
    mcpwm_set_duty_in_us(BLDC0_UNIT, BLDC0_TIMER, MCPWM_OPR_A, pwmDes);
}