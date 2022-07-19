#pragma once

// Application Control
#define SYSTEM_SAMPLE_PERIOD_MS         100 // TODO
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
#define PWM_MIN                         1000.0f
#define PWM_DELTA                       1000.0f

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
#define RPM_MIN                         1200
#define RPM_MAX                         10000
#define RPM_COUNT                       3
#define RPM_WINDOW_MS                   11
#define RPM0_GPIO                       0
#define RPM1_GPIO                       1
#define RPM2_GPIO                       2

// PID
#define BLDC_KP                         0.08f
#define BLDC_KI                         0.3f
#define BLDC_IMAX                       0.35f
#define BLDC_FF                         0.0f
#define BLDC_RAMP                       200.0f
#define BLDC_FFMAX                      1.0f

// SMC2
#define BLDC_C1                         0.1f
#define BLDC_FSTAR                      3.0f
#define BLDC_FI                         100.0f
#define BLDC_ERROR_FILTER               0
#define BLDC_ERROR_FILTER_K             0.6f

// Bluetooth
#define SPP_TAG                         "PWM_CTRL"
#define SPP_SERVER_NAME                 "PWM_CTRL_SERVER"
#define BT_INIT_MSG                     "Connection stablished...\n"
#define BT_RECEIVED_MSG                 "Received: "
// #define BT_DEVICE_NAME                  "PWM Control"
#define BT_DEVICE_NAME                  "PWM Control 2"
#define BT_MSG_SET_PWMDES_DONE          "Setting PWM to: "
#define BT_MSG_SET_RPMDES_DONE          "Setting RPM to: "
#define BT_BUFFERSIZE                   32

#define BT_MSG_SHUTDOWN                 "X"
#define BT_MSG_GET_RPM                  "RGET"
#define BT_MSG_SET_PWMDES               "PSET"
#define BT_MSG_SET_RPMDES               "RSET"