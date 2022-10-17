#include "nvs_flash.h"
#include "esp_log.h"
#include "esp_bt.h"
#include "esp_bt_main.h"
#include "esp_gap_bt_api.h"
#include "esp_bt_device.h"
#include "esp_spp_api.h"
#include "config.h"
#include "freertos/task.h"
#include "main.h"
#include <iostream>

namespace serialBTLogger{

struct BTData_ptr {
    controlData_ptr *controlData;
    TaskHandle_t *controlTask_h;
};
static BTData_ptr *__BTData_ptr;
static float __AngDes;

static char buffer[BT_BUFFERSIZE];
static char LF[] = "\n";

uint16_t readUInt16(char *in){
    char trashBuffer[BT_BUFFERSIZE];
    int out = 0;
    sscanf(in, "%s %d", trashBuffer, &out);
    return out;
}

// struct pwmConfig {
//     uint16_t pwmDes;
//     uint8_t n;
// };
// static pwmConfig pwmConfigDes;

// struct rpmConfig {
//     float rpmDes;
//     uint8_t n;
// };
// static rpmConfig rpmConfigDes;

// pwmConfig readPWM(char *in){
//     char trashBuffer[BT_BUFFERSIZE];
//     int PWM{0}, n_BLDC{0};
//     sscanf(in, "%s %d %d", trashBuffer, &PWM, &n_BLDC);
//     pwmConfig out = {.pwmDes = (uint16_t) PWM, .n = (uint8_t) n_BLDC};
//     return out;
// }

// rpmConfig readRPM(char *in){
//     char trashBuffer[BT_BUFFERSIZE];
//     int RPM{0}, n_BLDC{0};
//     sscanf(in, "%s %d %d", trashBuffer, &RPM, &n_BLDC);
//     rpmConfig out = {.rpmDes = (float) RPM, .n = (uint8_t) n_BLDC};
//     return out;
// }

int sizeofArray(char* array){
    int numberOfChars = 0;
    while (*array++){
        numberOfChars++;
    }
    return numberOfChars;
}

void logFloat(esp_spp_cb_param_t *param, const float *dataPtr, const int lenData, const float K, const char header[], const int headerSize){
    esp_spp_write(param->write.handle, headerSize, (uint8_t*) header);
    for (int j = 0; j < lenData; j++){
        sprintf(buffer, " %.3f", *(dataPtr + j)*K);
        esp_spp_write(param->write.handle, sizeofArray(buffer), (uint8_t *) buffer);
    }
    esp_spp_write(param->write.handle, 2, (uint8_t *)LF);
}

void logUInt16(esp_spp_cb_param_t *param, const uint16_t *dataPtr){
    sprintf(buffer, "%u", *dataPtr);
    esp_spp_write(param->write.handle, sizeofArray(buffer), (uint8_t *) buffer);
    esp_spp_write(param->write.handle, 1, (uint8_t *)LF);
}

void logAng(esp_spp_cb_param_t *param){
    sprintf(buffer, "CURRANG: %.3f", *(__BTData_ptr->controlData->currAngle_ptr));
    esp_spp_write(param->write.handle, sizeofArray(buffer), (uint8_t *) buffer);
    esp_spp_write(param->write.handle, 1, (uint8_t *)LF);
}

void logKillSwitch(esp_spp_cb_param_t *param){
    sprintf(buffer, "KILLSWITCH: %u", *(__BTData_ptr->controlData->killSwitch_ptr));
    esp_spp_write(param->write.handle, sizeofArray(buffer), (uint8_t *) buffer);
    esp_spp_write(param->write.handle, 1, (uint8_t *)LF);
}

void logControlMode(esp_spp_cb_param_t *param){
    sprintf(buffer, "CTRLMODE: %u", *(__BTData_ptr->controlData->controlMode_ptr));
    esp_spp_write(param->write.handle, sizeofArray(buffer), (uint8_t *) buffer);
    esp_spp_write(param->write.handle, 1, (uint8_t *)LF);
}

float readAngDes(char *in){
    char trashBuffer[BT_BUFFERSIZE];
    float AngDes{0.0f};
    sscanf(in, "%s %f", trashBuffer, &AngDes);
    return AngDes;
}

bool getAngDes(esp_spp_cb_param_t *param){
    __AngDes = readAngDes((char *) param->data_ind.data);
    if (__AngDes > 360 || __AngDes < -360){
        return false;
    }
    return true;
};

void logAngDesConfig(esp_spp_cb_param_t *param){
    sprintf(buffer, "Setting Desired Angle to: %.3f", __AngDes);
    esp_spp_write(param->write.handle, sizeofArray(buffer), (uint8_t *) buffer);
    esp_spp_write(param->write.handle, 1, (uint8_t *)LF);
}

void logRPM(esp_spp_cb_param_t *param){
    sprintf(buffer, "RPM0: %.3f", __BTData_ptr->controlData->rpmState_ptr->rpmCurr);
    esp_spp_write(param->write.handle, sizeofArray(buffer), (uint8_t *) buffer);
    esp_spp_write(param->write.handle, 1, (uint8_t *)LF);
    // sprintf(buffer, "RPM1: %.3f", __BTData_ptr->controlData->rpmState_ptr[1].rpmCurr);
    // esp_spp_write(param->write.handle, sizeofArray(buffer), (uint8_t *) buffer);
    // esp_spp_write(param->write.handle, 1, (uint8_t *)LF);
    // sprintf(buffer, "RPM2: %.3f", __BTData_ptr->controlData->rpmState_ptr[2].rpmCurr);
    // esp_spp_write(param->write.handle, sizeofArray(buffer), (uint8_t *) buffer);
    // esp_spp_write(param->write.handle, 1, (uint8_t *)LF);
}

// void logPWMConfig(esp_spp_cb_param_t *param){
//     sprintf(buffer, "Setting BLDC PWM %u to: %u", pwmConfigDes.n, pwmConfigDes.pwmDes);
//     esp_spp_write(param->write.handle, sizeofArray(buffer), (uint8_t *) buffer);
//     esp_spp_write(param->write.handle, 1, (uint8_t *)LF);
// }

// void logRPMConfig(esp_spp_cb_param_t *param){
//     sprintf(buffer, "Setting BLDC RPM %u to: %.3f", rpmConfigDes.n, rpmConfigDes.rpmDes);
//     esp_spp_write(param->write.handle, sizeofArray(buffer), (uint8_t *) buffer);
//     esp_spp_write(param->write.handle, 1, (uint8_t *)LF);
// }



bool isEqual(char *a, char *b, const int len){
    for(int i = 0; i < len; i++){
        if(*(a+i)!=*(b+i))
            return 0;
    }
    return 1;
}


// bool getPWM(esp_spp_cb_param_t *param){
//     pwmConfigDes = readPWM((char *) param->data_ind.data);
    // if (pwmConfigDes.pwmDes >= MAX_PWM){
    //     return false;
    // }
    // if (pwmConfigDes.n >= N_BLDC){
    //     return false;
    // }
//     return true;
// };

// bool getRPM(esp_spp_cb_param_t *param){
//     rpmConfigDes = readRPM((char *) param->data_ind.data);
//     if (rpmConfigDes.rpmDes >= RPM_MAX){
//         return false;
//     }
//     if (rpmConfigDes.n >= N_BLDC){
//         return false;
//     }
//     return true;
// };

static bool bWriteAfterOpenEvt = false;
static bool bWriteAfterWriteEvt = false;
static bool bWriteAfterSvrOpenEvt = true;
static bool bWriteAfterDataReceived = true;
 
static const esp_spp_mode_t esp_spp_mode = ESP_SPP_MODE_CB;
static const esp_spp_sec_t sec_mask = ESP_SPP_SEC_AUTHENTICATE;
static const esp_spp_role_t role_slave = ESP_SPP_ROLE_SLAVE;
 
static void esp_spp_cb(esp_spp_cb_event_t event, esp_spp_cb_param_t *param){
    //Used in app_main() to setup the BT configuration in the ESP32 and used for communication with device

    switch (event) {
    case ESP_SPP_INIT_EVT:
        esp_bt_dev_set_device_name(BT_DEVICE_NAME);
        esp_bt_gap_set_scan_mode(ESP_BT_CONNECTABLE, ESP_BT_GENERAL_DISCOVERABLE);
        esp_spp_start_srv(sec_mask,role_slave, 0, SPP_SERVER_NAME);
        break;
    case ESP_SPP_DISCOVERY_COMP_EVT:
        break;
    case ESP_SPP_OPEN_EVT:
        //When SPP Client connection open, the event comes

       

        if (bWriteAfterOpenEvt){
        }
        else{
        }

        break;
    case ESP_SPP_CLOSE_EVT:
        break;
    case ESP_SPP_START_EVT:                                         //Short before connection is established
        break;
    case ESP_SPP_CL_INIT_EVT:
        break;
    case ESP_SPP_DATA_IND_EVT:                                      //When SPP connection received data, the event comes, only for ESP_SPP_MODE_CB
 
        // PROCESSAMENTO DE MENSAGENS ------------------------------------------

        if (bWriteAfterDataReceived){
            const char c[] = BT_RECEIVED_MSG;
            esp_spp_write(param->srv_open.handle, sizeof(c)/sizeof(c[0])-1, (uint8_t*) c);
            esp_spp_write(param->write.handle, param->data_ind.len, param->data_ind.data);

            const char c0[] = BT_MSG_SET_ANGDES;
            if (isEqual((char *)param->data_ind.data, (char *)c0, sizeof(c0)/sizeof(c0[0]) - 1)){
                if (getAngDes(param)){
                    *(__BTData_ptr->controlData->desAngle_ptr) = __AngDes;
                    logAngDesConfig(param);
                }; 
            }

            const char c1[] = BT_MSG_SHUTDOWN, c1b[] = BT_MSG_SHUTDOWN_B;
            if (isEqual((char *)param->data_ind.data, (char *)c1, sizeof(c1)/sizeof(c1[0]) - 1) ||
                isEqual((char *)param->data_ind.data, (char *)c1b, sizeof(c1b)/sizeof(c1b[0]) - 1)){
                *(__BTData_ptr->controlData->killSwitch_ptr) = true;
            }

            const char c2[] = BT_MSG_ARM;
            if (isEqual((char *)param->data_ind.data, (char *)c2, sizeof(c2)/sizeof(c2[0]) - 1)){
                *(__BTData_ptr->controlData->killSwitch_ptr) = false;
            }

            const char c3[] = BT_MSG_SET_ZERO;
            if (isEqual((char *)param->data_ind.data, (char *)c3, sizeof(c3)/sizeof(c3[0]) - 1)){
                *(__BTData_ptr->controlData->setZero_ptr) = true;
            }

            const char c4[] = BT_MSG_BT_BP_ANGMAX;
            if (isEqual((char *)param->data_ind.data, (char *)c4, sizeof(c4)/sizeof(c4[0]) - 1)){
                *(__BTData_ptr->controlData->bypassAngMax_ptr) = true;
            }

            const char c5[] = BT_MSG_GET_ANG;
            if (isEqual((char *)param->data_ind.data, (char *)c5, sizeof(c5)/sizeof(c5[0]) - 1)){
                logAng(param);
            }

            const char c6[] = BT_MSG_GET_CONTROLMODE;
            if (isEqual((char *)param->data_ind.data, (char *)c6, sizeof(c6)/sizeof(c6[0]) - 1)){
                logKillSwitch(param);
                logControlMode(param);
            }

            const char c7[] = BT_MSG_GET_RPM;
            if (isEqual((char *)param->data_ind.data, (char *)c7, sizeof(c7)/sizeof(c7[0]) - 1)){
                logRPM(param);
            }

            // const char c3[] = BT_MSG_SET_RPMDES;
            // if (isEqual((char *)param->data_ind.data, (char *)c3, sizeof(c3)/sizeof(c3[0]) - 1)){
            //     if (getRPM(param)){
            //         __BTData_ptr->controlData->rpmState_ptr[rpmConfigDes.n].rpmDes = rpmConfigDes.rpmDes;
            //         __BTData_ptr->controlData->rpmState_ptr[rpmConfigDes.n].rpmDes_isNew = true;
            //         logRPMConfig(param);
            //     }; 
            // }

            // const char c4[] = BT_MSG_SET_RPMDES_ALL;
            // if (isEqual((char *)param->data_ind.data, (char *)c4, sizeof(c4)/sizeof(c4[0]) - 1)){
            //     if (getRPM(param)){
            //         for (uint8_t i = 0; i < N_BLDC; i++){
            //             rpmConfigDes.n = i;
            //             __BTData_ptr->controlData->rpmState_ptr[i].rpmDes = rpmConfigDes.rpmDes;
            //             logRPMConfig(param);
            //         }; 
            //     }; 
            // }

            // const char c5[] = BT_MSG_SET_PWMDES_ALL;
            // if (isEqual((char *)param->data_ind.data, (char *)c5, sizeof(c5)/sizeof(c5[0]) - 1)){
            //     if (getPWM(param)){
            //         for (uint8_t i = 0; i < N_BLDC; i++){
            //             pwmConfigDes.n = i;
            //             *(__BTData_ptr->controlData->pwmDes_ptr + i) = pwmConfigDes.pwmDes;
            //             logPWMConfig(param);
            //         }; 
            //     }; 
            // }

        }
 
        break;
    case ESP_SPP_CONG_EVT:
        break;
    case ESP_SPP_WRITE_EVT:
        //When SPP write operation completes, the event comes, only for ESP_SPP_MODE_CB
 
 
        if (param->write.cong == 0) {
            if (bWriteAfterWriteEvt){
            }
            else{
            }
        }
        else {
        }
        //Code copied from Initiator - End
 break;
    case ESP_SPP_SRV_OPEN_EVT:                                      //After connection is established, short before data is received
        //When SPP Server connection open, the event comes
        //In use in Acceptor
 
        //Added code from Initiator - Start
        if (bWriteAfterSvrOpenEvt){

            const char c[] = BT_INIT_MSG;
            esp_spp_write(param->srv_open.handle, sizeof(c)/sizeof(c[0]) - 1, (uint8_t*) c);
        }
        else{
        }
 
        break;
    default:
        break;
    }
}
 
void esp_bt_gap_cb(esp_bt_gap_cb_event_t event, esp_bt_gap_cb_param_t *param){
    //Used in app_main() to setup the BT configuration in the ESP32
    switch (event) {
    case ESP_BT_GAP_AUTH_CMPL_EVT:{
        if (param->auth_cmpl.stat == ESP_BT_STATUS_SUCCESS) {
            esp_log_buffer_hex(SPP_TAG, param->auth_cmpl.bda, ESP_BD_ADDR_LEN);
        } else {
            ESP_LOGE(SPP_TAG, "authentication failed, status:%d", param->auth_cmpl.stat);
        }
        break;
    }
    case ESP_BT_GAP_PIN_REQ_EVT:{
        if (param->pin_req.min_16_digit) {
            esp_bt_pin_code_t pin_code = {0};
            esp_bt_gap_pin_reply(param->pin_req.bda, true, 16, pin_code);
        } else {
            esp_bt_pin_code_t pin_code;
            pin_code[0] = '1';
            pin_code[1] = '2';
            pin_code[2] = '3';
            pin_code[3] = '4';
            esp_bt_gap_pin_reply(param->pin_req.bda, true, 4, pin_code);
        }
        break;
    }
 
    //Must be set in sdkconfig.h: CONFIG_BT_SSP_ENABLED == true
    //This enables the Secure Simple Pairing.
    case ESP_BT_GAP_CFM_REQ_EVT:
        esp_bt_gap_ssp_confirm_reply(param->cfm_req.bda, true);
        break;
    case ESP_BT_GAP_KEY_NOTIF_EVT:
        break;
    case ESP_BT_GAP_KEY_REQ_EVT:
        break;
 
 
    default: {
        //  0 ESP_BT_GAP_DISC_RES_EVT
        //  1 ESP_BT_GAP_DISC_STATE_CHANGED_EVT
        //  2 ESP_BT_GAP_RMT_SRVCS_EVT
        //  3 ESP_BT_GAP_RMT_SRVC_REC_EVT
        //  4 ESP_BT_GAP_AUTH_CMPL_EVT
        //  5 ESP_BT_GAP_PIN_REQ_EVT
        //  6 ESP_BT_GAP_CFM_REQ_EVT
        //  7 ESP_BT_GAP_KEY_NOTIF_EVT
        //  8 ESP_BT_GAP_KEY_REQ_EVT
        //  9 ESP_BT_GAP_READ_RSSI_DELTA_EVT
        // 10 ESP_BT_GAP_CONFIG_EIR_DATA_EVT
        // 11 ESP_BT_GAP_EVT_MAX
        break;
    }
    }
    return;
}
 
void startBT(BTData_ptr* BTData){
    __BTData_ptr = BTData;
 
    //Non-volatile storage (NVS) library is designed to store key-value pairs in flash.
    esp_err_t ret = nvs_flash_init();   //Initialize the default NVS partition.
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK( ret );
 
    ESP_ERROR_CHECK(esp_bt_controller_mem_release(ESP_BT_MODE_BLE));        //release the controller memory as per the mode
 
    esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
    if ((ret = esp_bt_controller_init(&bt_cfg)) != ESP_OK) {
        ESP_LOGE(SPP_TAG, "%s initialize controller failed: %s\n", __func__, esp_err_to_name(ret));
        return;
    }
    else{
    }
   
    if ((ret = esp_bt_controller_enable(ESP_BT_MODE_CLASSIC_BT)) != ESP_OK) {
        ESP_LOGE(SPP_TAG, "%s enable controller failed: %s\n", __func__, esp_err_to_name(ret));
        return;
    }
    else{
    }
   
    if ((ret = esp_bluedroid_init()) != ESP_OK) {
        ESP_LOGE(SPP_TAG, "%s initialize bluedroid failed: %s\n", __func__, esp_err_to_name(ret));
        return;
    }
    else{
    }
   
    if ((ret = esp_bluedroid_enable()) != ESP_OK) {
        ESP_LOGE(SPP_TAG, "%s enable bluedroid failed: %s\n", __func__, esp_err_to_name(ret));
        return;
    }
    else{
    }
   
    if ((ret = esp_bt_gap_register_callback(esp_bt_gap_cb)) != ESP_OK) {
        ESP_LOGE(SPP_TAG, "%s gap register failed: %s\n", __func__, esp_err_to_name(ret));
        return;
    }
    else{
    }
   
    if ((ret = esp_spp_register_callback(esp_spp_cb)) != ESP_OK) {
        ESP_LOGE(SPP_TAG, "%s spp register failed: %s\n", __func__, esp_err_to_name(ret));
        return;
    }
    else{
    }
   
    if ((ret = esp_spp_init(esp_spp_mode)) != ESP_OK) {
        ESP_LOGE(SPP_TAG, "%s spp init failed: %s\n", __func__, esp_err_to_name(ret));
        return;
    }
    else{
    }
   
    //Must be set in sdkconfig.h: CONFIG_BT_SSP_ENABLED == true
    //This enables the Secure Simple Pairing.
    /* Set default parameters for Secure Simple Pairing */
    esp_bt_sp_param_t param_type = ESP_BT_SP_IOCAP_MODE;
    esp_bt_io_cap_t iocap = ESP_BT_IO_CAP_IO;
    esp_bt_gap_set_security_param(param_type, &iocap, sizeof(uint8_t));
 
    /*
     * Set default parameters for Legacy Pairing
     * Use variable pin, input pin code when pairing
     */
    esp_bt_pin_type_t pin_type = ESP_BT_PIN_TYPE_VARIABLE;
    esp_bt_pin_code_t pin_code;
    esp_bt_gap_set_pin(pin_type, 0, pin_code);
}

};
