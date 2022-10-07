#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "gpio-handler.h"
#include "serial-logger.h"
#include "main.h"
#include "config.h"
// #include "BT.cpp"
#include "esp_timer.h"
// #include "freq-count.h"
#include "encoder-reader.h"
#include "step-motor.h"
#include "PID.h"

extern "C" void app_main(void)
{
    #if LOG_MAIN
    serialLogger::header();
    #endif
    builtin_led led;
    led.blink(5, false);

    static TaskHandle_t sendTask_h = NULL, readEncoderTask_h = NULL;
    static float currAngle = {0.0f};

    // static PID pid0;
    static controlData_ptr controlData = {.currAngle_ptr = &currAngle};

    vTaskDelay(1000/portTICK_PERIOD_MS);

    xTaskCreatePinnedToCore(readEncoderTask,
                            "readEncoder Task",
                            4*1024,
                            &controlData,
                            4,
                            &readEncoderTask_h,
                            1);

    vTaskDelay(SYSTEM_SAMPLE_PERIOD_MS/portTICK_PERIOD_MS);

    xTaskCreatePinnedToCore(sendTask,
                            "Send Task",
                            4*1024,
                            &controlData,
                            2,
                            &sendTask_h,
                            0);

    // static serialBTLogger::BTData_ptr BTData = {.controlData = &controlData,
    //                                             .controlTask_h = &controlTask_h};

    // serialBTLogger::startBT(&BTData);

}

void sendTask(void* Parameters){
    controlData_ptr* controlData = (controlData_ptr*) Parameters;
    TickType_t startTimer = xTaskGetTickCount();

#if LOG_TIMER
    int64_t start = esp_timer_get_time();
    int64_t dt = 0;
#endif

    while(1){

        serialLogger::logFloat(controlData->currAngle_ptr, "CURRANG");
        serialLogger::blank_lines(1);

        vTaskDelayUntil(&startTimer, SEND_PERIOD_MS/portTICK_PERIOD_MS);

// #if LOG_TIMER
//         dt = esp_timer_get_time() - start;
//         start = esp_timer_get_time();
//         serialLogger::logInt64(&dt, "DTSEND");
// #endif

    }
}

void readEncoderTask(void* Parameters){
    controlData_ptr* controlData = (controlData_ptr*) Parameters;
    encoderReader ECDReader;
    stepMotor Motor;

#if LOG_TIMER
    int64_t start = esp_timer_get_time();
    int64_t dt = 0;
#endif

    TickType_t startTimer = xTaskGetTickCount();

    while(1){
        ECDReader.getCurrAngle(controlData->currAngle_ptr);
        Motor.up();

        // if (*(controlData->currAngle_ptr) < 20.0f){
        //     Motor.up();
        // } else {
        //     // Motor.down();
        // }

#if LOG_TIMER
        dt = esp_timer_get_time() - start;
        start = esp_timer_get_time();
        serialLogger::logInt64(&dt, "DTRPM");
#endif

        vTaskDelayUntil(&startTimer, SYSTEM_SAMPLE_PERIOD_MS/portTICK_PERIOD_MS);

    }
}