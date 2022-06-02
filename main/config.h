#pragma once

// Application Control
#define SYSTEM_SAMPLE_PERIOD_MS         1000
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

// GPIO & PWM
#define BUILTIN_LED                     GPIO_NUM_2
#define MAX_PWM                         2500
#define N_BLDC                          3

#define BLDC0_UNIT                      MCPWM_UNIT_0
#define BLDC0_TIMER                     MCPWM_TIMER_0
#define BLDC0_GPIO                      18
#define BLDC0_IO                        MCPWM0A
#define BLDC0_FREQ                      50 // Hz
#define BLDC0_OPR                       MCPWM_GEN_A

#define BLDC1_UNIT                      MCPWM_UNIT_0
#define BLDC1_TIMER                     MCPWM_TIMER_1
#define BLDC1_GPIO                      19
#define BLDC1_IO                        MCPWM1A
#define BLDC1_FREQ                      50 // Hz
#define BLDC1_OPR                       MCPWM_GEN_A

#define BLDC2_UNIT                      MCPWM_UNIT_0
#define BLDC2_TIMER                     MCPWM_TIMER_2
#define BLDC2_GPIO                      21
#define BLDC2_IO                        MCPWM2A
#define BLDC2_FREQ                      50 // Hz
#define BLDC2_OPR                       MCPWM_GEN_A

#define RPM_MAX_COUNT                   1000
#define RPM_COUNT                       4
#define RPM_WINDOW_MS                   1000
#define RPM0_GPIO                       0

// Bluetooth
#define SPP_TAG                         "PWM_CTRL"
#define SPP_SERVER_NAME                 "PWM_CTRL_SERVER"
#define BT_INIT_MSG                     "Connection stablished...\n"
#define BT_RECEIVED_MSG                 "Received: "
#define BT_DEVICE_NAME                  "PWM Control"
#define BT_MSG_SET_PWMDES               "P"
#define BT_MSG_SET_PWMDES_DONE          "Setting PWM to: "
#define BT_MSG_SHUTDOWN                 "X"
#define BT_MSG_GET_RPM                  "RPM"
#define BT_BUFFERSIZE                   32