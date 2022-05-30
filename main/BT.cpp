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

static char buffer[BT_BUFFERSIZE];
static char LF[] = "\n";

uint16_t readUInt16(char *in){
    char trashBuffer[BT_BUFFERSIZE];
    int out = 0;
    sscanf(in, "%s %d", trashBuffer, &out);
    return out;
}



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

bool isEqual(char *a, char *b, const int len){
    for(int i = 0; i < len; i++){
        if(*(a+i)!=*(b+i))
            return 0;
    }
    return 1;
}

struct BTData_ptr {
    controlData_ptr *controlData;
    TaskHandle_t *controlTask_h;
};
static BTData_ptr *__BTData_ptr;

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
 
        if (bWriteAfterDataReceived){
            const char c[] = BT_RECEIVED_MSG;
            esp_spp_write(param->srv_open.handle, sizeof(c)/sizeof(c[0])-1, (uint8_t*) c);
            esp_spp_write(param->write.handle, param->data_ind.len, param->data_ind.data);

            const char c0[] = BT_MSG_SET_PWMDES;
            if (isEqual((char *)param->data_ind.data, (char *)c0, sizeof(c0)/sizeof(c0[0]) - 1)){
                std::cout<< "How" << std::endl;
                uint16_t pwmDes_new = readUInt16((char *) param->data_ind.data);
                if (pwmDes_new >= MAX_PWM){
                    pwmDes_new = MAX_PWM;
                }
                *(__BTData_ptr->controlData->pwmDes_ptr) = pwmDes_new;
                const char c1[] = BT_MSG_SET_PWMDES_DONE;
                esp_spp_write(param->srv_open.handle, sizeof(c1)/sizeof(c1[0])-1, (uint8_t*) c1);
                logUInt16(param, &pwmDes_new);
            }

            // if (isEqual((char *)param->data_ind.data, (char *)c2, sizeof(c2)/sizeof(c2[0]) - 1)){
            //     logFloat(param, __navDataBT_ptr->navData->eulerAngles_ptr,
            //             3, 1/DEG2RAD, "ATT", 4);
            // } else if (isEqual((char *)param->data_ind.data, (char *)c3, sizeof(c3)/sizeof(c3[0]) - 1)){
            //     logFloat(param, __navDataBT_ptr->navData->M_ptr,
            //             3, 1, "MAG", 4);
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
