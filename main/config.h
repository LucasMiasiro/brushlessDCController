#pragma once

// Application Control
#define SYSTEM_SAMPLE_PERIOD_MS         100
#define APP_MODE                        0

#define SHOULD_LOG                      1
#if SHOULD_LOG
    #define LOG_MAIN                    1
    #define LOG_TIMER                   0
#else
    #define LOG_MAIN                    0
    #define LOG_TIMER                   0
#endif

// _________________________________________________________________________

// GPIO
#define BUILTIN_LED                     GPIO_NUM_2

// PWM
#define MAX_PWM                         2500
#define BLDC0_UNIT                      MCPWM_UNIT_0
#define BLDC0_TIMER                     MCPWM_TIMER_0
#define BLDC0_GPIO                      18
#define BLDC0_IO                        MCPWM0A
#define BLDC0_FREQ                      50 // Hz
#define BLDC0_OPR                       MCPWM_OPR_A

// Bluetooth
#define SPP_TAG                         "PWM_CTRL"
#define SPP_SERVER_NAME                 "PWM_CTRL_SERVER"
#define BT_INIT_MSG                     "Connection stablished...\n"
#define BT_RECEIVED_MSG                 "Received: "
#define BT_DEVICE_NAME                  "PWM Control"
#define BT_MSG_SET_PWMDES               "P"
#define BT_MSG_SET_PWMDES_DONE          "Setting PWM to: "
#define BT_BUFFERSIZE                   20