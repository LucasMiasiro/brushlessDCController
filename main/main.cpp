#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "gpio-handler.h"
#include "serial-logger.h"
#include "main.h"
#include "config.h"
#include "BT.cpp"
#include "esp_timer.h"
#include "freq-count.h"
#include "encoder-reader.h"
#include "step-motor.h"
#include "PID.h"
#include "SMC2.h"
#include "baro.h"

extern "C" void app_main(void)
{
    #if LOG_MAIN
    serialLogger::header();
    #endif
    builtin_led led;
    led.blink(5, false);

    static TaskHandle_t sendTask_h = NULL, controlTask_h = NULL;
    static float currAngle = {0.0f}, desAngle = {0.0f};
    static float T = {0.0f}, p = {0.0f};
    static bool setZero = false, killSwitch = true, bypassAngMax = false,
                homeWasSet = false;
    static uint16_t pwmDes0 = 0, pwmDes1 = 0, pwmDes2 = 0;

    static rpmState rpmState0 = {0.0f, false, 0.0f, false}, rpmState1 = {0.0f, false, 0.0f, false}, rpmState2 = {0.0f, false, 0.0f, false};

    static controlData_ptr controlData = {.currAngle_ptr = &currAngle,
                                          .desAngle_ptr = &desAngle,
                                          .p_ptr = &p,
                                          .T_ptr = &T,
                                          .setZero_ptr = &setZero,
                                          .killSwitch_ptr = &killSwitch,
                                          .bypassAngMax_ptr = &bypassAngMax,
                                          .homeWasSet_ptr = &homeWasSet,
                                          .controlMode_ptr = NULL,
                                          .rpmState0_ptr = &rpmState0,
                                          .rpmState1_ptr = &rpmState1,
                                          .rpmState2_ptr = &rpmState2,
                                          .pwmDes0_ptr = &pwmDes0,
                                          .pwmDes1_ptr = &pwmDes1,
                                          .pwmDes2_ptr = &pwmDes2,
                                          };

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

    static serialBTLogger::BTData_ptr BTData = {.controlData = &controlData,
                                            .controlTask_h = &controlTask_h};

    serialBTLogger::startBT(&BTData);

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
        serialLogger::logFloat(controlData->desAngle_ptr, "DESANG");
        serialLogger::logUInt8((uint8_t*)(controlData->controlMode_ptr), "CTRLMODE");
        serialLogger::logUInt8((uint8_t*)(controlData->killSwitch_ptr), "KILLSWITCH");
        serialLogger::logUInt8((uint8_t*)(controlData->homeWasSet_ptr), "HOMESET");
        serialLogger::logFloat(&(controlData->rpmState0_ptr->rpmCurr), "CURRRPM0");
        serialLogger::logFloat(&(controlData->rpmState0_ptr->rpmDes), "DESRPM0");
        serialLogger::logUInt16(controlData->pwmDes0_ptr, "PWM0");
        serialLogger::logFloat(&(controlData->rpmState1_ptr->rpmCurr), "CURRRPM1");
        serialLogger::logFloat(&(controlData->rpmState1_ptr->rpmDes), "DESRPM1");
        serialLogger::logUInt16(controlData->pwmDes1_ptr, "PWM1");
        serialLogger::logFloat(&(controlData->rpmState2_ptr->rpmCurr), "CURRRPM2");
        serialLogger::logFloat(&(controlData->rpmState2_ptr->rpmDes), "DESRPM2");
        serialLogger::logUInt16(controlData->pwmDes2_ptr, "PWM2");
#if ENABLE_BARO
        serialLogger::logFloat((controlData->p_ptr), "PRESSURE");
        serialLogger::logFloat((controlData->T_ptr), "TEMPERATURE");
#endif
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
    encoderReader ECDReader(true);
    stepMotor stepMotor;
    controlMode currMode = STOP, prevMode = STOP;
    controlData->controlMode_ptr = &currMode;
    float angError = 0.0f, prevMeas = 0.0f;
    const uint32_t watchdogCounter_max = SM_WATCHDOG_COUNTER_MAX;
    const float watchdogCounter_diffMin = SM_WATCHDOG_DIFF_MIN;
    uint32_t watchdogCounter = 0;
    rpmCounter RPM0(RPM0_GPIO), RPM1(RPM1_GPIO), RPM2(RPM2_GPIO);
    SMC2 SMC0, SMC1, SMC2;
    bldc BLDC0(BLDC0_GPIO), BLDC1(BLDC1_GPIO), BLDC2(BLDC2_GPIO);
    baro baro;

#if LOG_TIMER
    int64_t start = esp_timer_get_time();
    int64_t dt = 0;
#endif

    TickType_t startTimer = xTaskGetTickCount();

    while(1){
        RPM0.getRPM(controlData->rpmState0_ptr);
        RPM1.getRPM(controlData->rpmState1_ptr);
        RPM2.getRPM(controlData->rpmState2_ptr);

#if CL_CONTROL_BLDC
        SMC0.update(controlData->rpmState0_ptr->rpmDes,
                    controlData->rpmState0_ptr->rpmCurr,
                    controlData->rpmState0_ptr->rpmCurr_isNew);
        *(controlData->pwmDes0_ptr) = SMC0.get();
        controlData->rpmState0_ptr->rpmCurr_isNew = false;

        SMC1.update(controlData->rpmState1_ptr->rpmDes,
                    controlData->rpmState1_ptr->rpmCurr,
                    controlData->rpmState1_ptr->rpmCurr_isNew);
        *(controlData->pwmDes1_ptr) = SMC1.get();
        controlData->rpmState1_ptr->rpmCurr_isNew = false;

        SMC2.update(controlData->rpmState2_ptr->rpmDes,
                    controlData->rpmState2_ptr->rpmCurr,
                    controlData->rpmState2_ptr->rpmCurr_isNew);
        *(controlData->pwmDes2_ptr) = SMC2.get();
        controlData->rpmState2_ptr->rpmCurr_isNew = false;
#endif

        BLDC0.setPWM(*(controlData->pwmDes0_ptr));
        BLDC1.setPWM(*(controlData->pwmDes1_ptr));
        BLDC2.setPWM(*(controlData->pwmDes2_ptr));

        if (*(controlData->killSwitch_ptr)) {
            prevMode = STOP;
            currMode = STOP;
            watchdogCounter = 0;
        }

        prevMeas = *(controlData->currAngle_ptr);
        ECDReader.getCurrAngle(controlData->currAngle_ptr,
                              *(controlData->setZero_ptr),
                              controlData->homeWasSet_ptr);

#if CL_CONTROL_STEPMOTOR
        if (watchdogCounter > watchdogCounter_max) {
            *(controlData->killSwitch_ptr) = true;
            watchdogCounter = 0;
        }

        if (std::abs(prevMeas - *(controlData->currAngle_ptr)) > watchdogCounter_diffMin) {
            watchdogCounter = 0;
        }
#endif

        if (*(controlData->setZero_ptr)) {
            *(controlData->desAngle_ptr) = 0.0f;
            *(controlData->setZero_ptr) = false;
            stepMotor.stop();
            prevMode = STOP;
            currMode = STOP;
            watchdogCounter = 0;
        }

        angError = *(controlData->desAngle_ptr) - *(controlData->currAngle_ptr);

        switch (currMode) {
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
            if ( !*(controlData->bypassAngMax_ptr)
                && ( *(controlData->currAngle_ptr) < -SM_ANG_MAX
                     || *(controlData->currAngle_ptr) > SM_ANG_MAX )){
                stepMotor.stop();
                prevMode = GO;
                currMode = STOP;
                *(controlData->killSwitch_ptr) = true;
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
#if CL_CONTROL_STEPMOTOR
            watchdogCounter++;
#endif
            break;

        default:
            break;
        }

#if ENABLE_BARO
        baro.getData(controlData->p_ptr, controlData->T_ptr);
#endif

#if LOG_TIMER
        dt = esp_timer_get_time() - start;
        start = esp_timer_get_time();
        serialLogger::logInt64(&dt, "DTRPM");
#endif

        vTaskDelayUntil(&startTimer, SYSTEM_SAMPLE_PERIOD_MS/portTICK_PERIOD_MS);

    }
}