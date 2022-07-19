#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "gpio-handler.h"
#include "serial-logger.h"
#include "main.h"
#include "config.h"
#include "BT.cpp"
#include "esp_timer.h"
#include "freq-count.h"
#include "PID.h"

#if APP_MODE==0
extern "C" void app_main(void)
{
    #if LOG_MAIN
    serialLogger::header();
    #endif
    builtin_led led;
    led.blink(5, false);

    static TaskHandle_t controlTask_h = NULL, sendTask_h = NULL, readRPMTask_h = NULL;
    static uint16_t pwmDes[N_BLDC] = {0};
    static rpmState rpmState0 = {.rpmCurr = 0.0f,
                                .rpmCurr_isNew = false,
                                .rpmDes = 0.0f,
                                .rpmDes_isNew = false};
    // static PID pid0;
    static SMC2 smc2_0;
    static controlData_ptr controlData = {.pwmDes_ptr = &pwmDes[0],
                                          .rpmState_ptr = &rpmState0,
                                        //   .pid_ptr = &pid0};
                                          .smc2_ptr = &smc2_0};

    vTaskDelay(1000/portTICK_PERIOD_MS);

    xTaskCreatePinnedToCore(readRPMTask,
                            "readRPM Task",
                            4*1024,
                            &controlData,
                            3,
                            &readRPMTask_h,
                            1);

    xTaskCreatePinnedToCore(controlTask,
                            "Control Task",
                            4*1024,
                            &controlData,
                            3,
                            &controlTask_h,
                            1);

    vTaskDelay(SYSTEM_SAMPLE_PERIOD_MS/portTICK_PERIOD_MS);

    xTaskCreatePinnedToCore(sendTask,
                            "Send Task",
                            4*1024,
                            &controlData,
                            2,
                            &sendTask_h,
                            0);

    static serialBTLogger::BTData_ptr BTData = {.controlData = &controlData,
                                                .controlTask_h = &controlTask_h};

    serialBTLogger::startBT(&BTData);

}

#elif APP_MODE==1
extern "C" void app_main(void)
{
}

#endif

void controlTask(void* Parameters){
    controlData_ptr* controlData = (controlData_ptr*) Parameters;
    bldc BLDC0(0), BLDC1(1), BLDC2(2);

#if LOG_TIMER
    int64_t start = esp_timer_get_time();
    int64_t dt = 0;
#endif

    TickType_t startTimer = xTaskGetTickCount();

    while(1){

        // controlData->pid_ptr->update(controlData->rpmState_ptr->rpmDes,
        //                             controlData->rpmState_ptr->rpmCurr,
        //                             controlData->rpmState_ptr->rpmCurr_isNew);

        controlData->smc2_ptr->update(controlData->rpmState_ptr->rpmDes,
                                    controlData->rpmState_ptr->rpmCurr,
                                    controlData->rpmState_ptr->rpmCurr_isNew);

        // *(controlData->pwmDes_ptr) = controlData->pid_ptr->get();
        *(controlData->pwmDes_ptr) = controlData->smc2_ptr->get();
        controlData->rpmState_ptr->rpmCurr_isNew = false;

        BLDC0.setPWM(*(controlData->pwmDes_ptr));
        BLDC1.setPWM(*(controlData->pwmDes_ptr + 1));
        BLDC2.setPWM(*(controlData->pwmDes_ptr + 2));

        vTaskDelayUntil(&startTimer, SYSTEM_SAMPLE_PERIOD_MS/portTICK_PERIOD_MS);

#if LOG_TIMER
        dt = esp_timer_get_time() - start;
        start = esp_timer_get_time();
        serialLogger::logInt64(&dt, "DTCTRL");
#endif

    }
}

void sendTask(void* Parameters){
    controlData_ptr* controlData = (controlData_ptr*) Parameters;
    TickType_t startTimer = xTaskGetTickCount();

#if LOG_TIMER
    int64_t start = esp_timer_get_time();
    int64_t dt = 0;
#endif

    while(1){

        serialLogger::logFloat(&(controlData->rpmState_ptr->rpmDes), "RPMDES");
        serialLogger::logUInt16(controlData->pwmDes_ptr, "PWMDES");
        serialLogger::logFloat(&(controlData->rpmState_ptr->rpmCurr), "RPMCURR");
        serialLogger::blank_lines(1);

        vTaskDelayUntil(&startTimer, SYSTEM_SAMPLE_PERIOD_MS/portTICK_PERIOD_MS);

// #if LOG_TIMER
//         dt = esp_timer_get_time() - start;
//         start = esp_timer_get_time();
//         serialLogger::logInt64(&dt, "DTSEND");
// #endif

    }
}

void readRPMTask(void* Parameters){
    controlData_ptr* controlData = (controlData_ptr*) Parameters;
    rpmCounter rpm;

#if LOG_TIMER
    int64_t start = esp_timer_get_time();
    int64_t dt = 0;
#endif

    TickType_t startTimer = xTaskGetTickCount();

    while(1){
        rpm.getRPM(controlData->rpmState_ptr);

#if LOG_TIMER
        dt = esp_timer_get_time() - start;
        start = esp_timer_get_time();
        serialLogger::logInt64(&dt, "DTRPM");
#endif

        vTaskDelayUntil(&startTimer, SYSTEM_SAMPLE_PERIOD_MS/portTICK_PERIOD_MS);

    }
}