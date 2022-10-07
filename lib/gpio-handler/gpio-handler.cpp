#include "driver/gpio.h"
// #include "driver/mcpwm.h"
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

// void bldc::setup(){

//     mcpwm_config_t pwm_config = {
//         .frequency = 50,
//         .cmpr_a = 0,     // Initial duty cycle of PWM0A
//         .cmpr_b = 0,     // Initial duty cycle of PWM0B
//         .duty_mode = MCPWM_DUTY_MODE_0,
//         .counter_mode = MCPWM_UP_COUNTER,
//     };

//     switch (n) {
//         case 0:
//             mcpwm_gpio_init(BLDC0_UNIT, BLDC0_IO, BLDC0_GPIO);
//             pwm_config.frequency = BLDC0_FREQ;
//             mcpwm_init(BLDC0_UNIT, BLDC0_TIMER, &pwm_config);
//             break;

//         case 1:
//             mcpwm_gpio_init(BLDC1_UNIT, BLDC1_IO, BLDC1_GPIO);
//             pwm_config.frequency = BLDC1_FREQ;
//             mcpwm_init(BLDC1_UNIT, BLDC1_TIMER, &pwm_config);
//             break;

//         case 2:
//             mcpwm_gpio_init(BLDC2_UNIT, BLDC2_IO, BLDC2_GPIO);
//             pwm_config.frequency = BLDC2_FREQ;
//             mcpwm_init(BLDC2_UNIT, BLDC2_TIMER, &pwm_config);
//             break;
//     }
// }

// void bldc::setPWM(uint16_t pwmDes){
//     switch (n) {
//         case 0:
//             mcpwm_set_duty_in_us(BLDC0_UNIT, BLDC0_TIMER, BLDC0_OPR, pwmDes);
//             break;
//         case 1:
//             mcpwm_set_duty_in_us(BLDC1_UNIT, BLDC1_TIMER, BLDC1_OPR, pwmDes);
//             break;
//         case 2:
//             mcpwm_set_duty_in_us(BLDC2_UNIT, BLDC2_TIMER, BLDC2_OPR, pwmDes);
//             break;
//     }
// }