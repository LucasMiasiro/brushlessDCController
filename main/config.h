#pragma once

// Application Control
#define SYSTEM_SAMPLE_PERIOD_MS         1
#define SEND_PERIOD_MS                  100
#define CL_CONTROL_BLDC                 0 //Close-loop BLDC Control

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
#define PWM_IDLE                        800.0f
#define PWM_MIN                         1200.0f
#define PWM_DELTA                       1000.0f

// #define N_BLDC                          3
#define BLDC_TIMER_RESOLUTION           1000000 // 1 MHz (1 us per tick)
#define BLDC_TIMER_PERIOD               20000 // 20000 ticks (20000 us) (50 Hz)
#define BLDC0_GPIO                      GPIO_NUM_21
// #define BLDC1_GPIO                      GPIO_NUM_19
// #define BLDC2_GPIO                      GPIO_NUM_21

#define RPM_MAX_COUNT                   1000
#define RPM_COUNT                       2
#define RPM_COUNT_PER_REV               1
#define RPM_MIN                         1000
#define RPM_MAX                         12000
#define RPM_DT_MIN                      RPM_COUNT*60*1000000/(RPM_MAX)
#define RPM_DT_MAX                      RPM_COUNT*60*1000000/(RPM_MIN)
#define RPM0_GPIO                       16

// Encoder
#define ECD_MAX_COUNT                   20000
#define ECD0A_GPIO                      4
#define ECD0B_GPIO                      18
#define ECD0ZERO_GPIO                   5
#define ECD_TICKS                       2000.0f

// Step Motor
#define SM_TIMER_RESOLUTION             1000000 // 1 MHz (1 us per tick)
#define SM_TIMER_PERIOD                 1000 // 1000 ticks (1000 us) (1000 Hz)
#define SM_PULSE_WIDTH                  100  // 100 ticks  (100 us)
#define SM_PULSE_GPIO                   19
#define SM_DIR_GPIO                     GPIO_NUM_17
#define SM_ANG_TOL                      0.12f
#define SM_ANG_MAX                      60.0f
#define SM_WATCHDOG_COUNTER_MAX         5000;

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
#define BLDC_MAX_SANITY_COUNTER         1000/SYSTEM_SAMPLE_PERIOD_MS // 1s

// Bluetooth
#define SPP_TAG                         "SM_CTRL"
#define SPP_SERVER_NAME                 "SM_CTRL_SERVER"
#define BT_INIT_MSG                     "Connection stablished...\n"
#define BT_RECEIVED_MSG                 "Received: "
#define BT_DEVICE_NAME                  "LAE - Stepper Motor Control"
#define BT_BUFFERSIZE                   32

#define BT_MSG_SHUTDOWN                 "X"
#define BT_MSG_SHUTDOWN_B               "DISARM"
#define BT_MSG_ARM                      "ARM"
#define BT_MSG_GET_RPM                  "RGET"
#define BT_MSG_SET_PWMDES               "PSET"
#define BT_MSG_SET_RPMDES               "RSET"
#define BT_MSG_SET_PWMDES_ALL           "ALLPSET"
#define BT_MSG_SET_RPMDES_ALL           "ALLRSET"

#define BT_MSG_SET_ZERO                 "ZEROSET"
#define BT_MSG_BT_BP_ANGMAX             "BPANGMAX"
#define BT_MSG_SET_ANGDES               "ANGSET"
#define BT_MSG_GET_ANG                  "ANGGET"
#define BT_MSG_GET_CONTROLMODE          "CTMDGET"