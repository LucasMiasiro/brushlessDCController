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

    static TaskHandle_t sendTask_h = NULL, controlTask_h = NULL;
    static float currAngle = {0.0f}, desAngle = {0.0f};
    static bool setZero = false, shouldStop = true;

    // static PID pid0;
    static controlData_ptr controlData = {.currAngle_ptr = &currAngle,
                                          .desAngle_ptr = &desAngle,
                                          .setZero_ptr = &setZero,
                                          .shouldStop_ptr = &shouldStop};

    vTaskDelay(1000/portTICK_PERIOD_MS);

    xTaskCreatePinnedToCore(controlTask,
                            "Control Task",
                            4*1024,
                            &controlData,
                            4,
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

void controlTask(void* Parameters){
    controlData_ptr* controlData = (controlData_ptr*) Parameters;
    encoderReader ECDReader;
    stepMotor stepMotor;
    controlMode currMode = STOP, prevMode = STOP;
    float angError = 0.0f;

#if LOG_TIMER
    int64_t start = esp_timer_get_time();
    int64_t dt = 0;
#endif

    TickType_t startTimer = xTaskGetTickCount();

    while(1){

        if (*(controlData->shouldStop_ptr)) {
            prevMode = STOP;
            currMode = STOP;
        }

        ECDReader.getCurrAngle(controlData->currAngle_ptr,
                              *(controlData->setZero_ptr));

        if (*(controlData->setZero_ptr)) {
            *(controlData->desAngle_ptr) = 0.0f;
            *(controlData->setZero_ptr) = false;
            stepMotor.stop();
            prevMode = STOP;
            currMode = STOP;
        }

        angError = *(controlData->desAngle_ptr) - *(controlData->currAngle_ptr);

        switch (currMode)
        {
        case STOP:
            stepMotor.stop();
            if (angError > SM_ANG_TOL) {
                prevMode = STOP;
                currMode = SHOULD_GO_UP;
            } else if (angError < - SM_ANG_TOL) {
                prevMode = STOP;
                currMode = SHOULD_GO_DOWN;
            }
            break;
        
        case SHOULD_GO_UP:
            // Configura o motor para subir
            stepMotor.setUp();
            prevMode = SHOULD_GO_UP;
            currMode = GO;
            break;

        case SHOULD_GO_DOWN:
            // Configura o motor para descer
            stepMotor.setDown();
            prevMode = SHOULD_GO_DOWN;
            currMode = GO;
            break;
        
        case GO:
            // Se passar do ângulo máximo de segurança, parar o motor
            if (*(controlData->currAngle_ptr) < -SM_ANG_MAX
                || *(controlData->currAngle_ptr) > SM_ANG_MAX){
                stepMotor.stop();
                prevMode = GO;
                currMode = STOP;
                break;
            }

            // Se atingir a tolerância, parar o motor
            if (angError < SM_ANG_TOL && angError > -SM_ANG_TOL) {
                stepMotor.stop();
                prevMode = GO;
                currMode = STOP;
                break;
            // Se o erro não se manter coerente, trocar a direção
            } else if (prevMode == SHOULD_GO_UP && angError < -SM_ANG_TOL) {
                stepMotor.stop();
                prevMode = GO;
                currMode = SHOULD_GO_DOWN;
                break;
            // Se o erro não se manter coerente, trocar a direção
            } else if (prevMode == SHOULD_GO_DOWN && angError > SM_ANG_TOL) {
                stepMotor.stop();
                prevMode = GO;
                currMode = SHOULD_GO_UP;
                break;
            }

            // Acionar o motor, caso esteja tudo de acordo e o erro tenha se
            // mantido coerente
            stepMotor.go();
            break;

        default:
            break;
        }

#if LOG_TIMER
        dt = esp_timer_get_time() - start;
        start = esp_timer_get_time();
        serialLogger::logInt64(&dt, "DTRPM");
#endif

        vTaskDelayUntil(&startTimer, SYSTEM_SAMPLE_PERIOD_MS/portTICK_PERIOD_MS);

    }
}