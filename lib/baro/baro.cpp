#include "baro.h"
#include "driver/i2c.h"
#include "config.h"
#include <iostream>

bool baro::getData(float* p_out, float* T_out){
    if (readData()){
        return 1;
    }

    calcTruePressure();

    *p_out = (float)p;
    *T_out = (float)T/100.0f;
    return 0;
}

baro::baro(){
    setup_i2c();
    getBaroCal();
    opt = BMP280_CONFIG_01_OPT;
    write(&opt, 1, BMP280_ADD, BMP280_CONFIG_01_ADD);
    opt = BMP280_CONFIG_02_OPT;
    write(&opt, 1, BMP280_ADD, BMP280_CONFIG_02_ADD);
};

void baro::getBaroCal(){
    read(buffer, 24, BMP280_ADD, BMP280_CALIB_REG);
    T1 = (buffer[1] << 8 | buffer[0]);
    T2 = (buffer[3] << 8 | buffer[2]);
    T3 = (buffer[5] << 8 | buffer[4]);
    P1 = (buffer[7] << 8 | buffer[6]);
    P2 = (buffer[9] << 8 | buffer[8]);
    P3 = (buffer[11] << 8 | buffer[10]);
    P4 = (buffer[13] << 8 | buffer[12]);
    P5 = (buffer[15] << 8 | buffer[14]);
    P6 = (buffer[17] << 8 | buffer[16]);
    P7 = (buffer[19] << 8 | buffer[18]);
    P8 = (buffer[21] << 8 | buffer[20]);
    P9 = (buffer[23] << 8 | buffer[22]);
}

void baro::calcTruePressure(){

    int32_t var1, var2;

    var1 = ((((T_raw>>3) - ((int32_t)T1<<1))) * ((int32_t)T2)) >> 11;
    var2 = (((((T_raw>>4) - ((int32_t)T1)) * ((T_raw>>4) - ((int32_t)T1))) >> 12) *
    ((int32_t)T3)) >> 14;
    t_fine = var1 + var2;
    T = (t_fine * 5 + 128) >> 8;

    var1 = (((int32_t)t_fine)>>1) - (int32_t)64000;
    var2 = (((var1>>2) * (var1>>2)) >> 11 ) * ((int32_t)P6);
    var2 = var2 + ((var1*((int32_t)P5))<<1);
    var2 = (var2>>2)+(((int32_t)P4)<<16);
    var1 = (((P3 * (((var1>>2) * (var1>>2)) >> 13 )) >> 3) + ((((int32_t)P2) * var1)>>1))>>18;
    var1 =((((32768+var1))*((int32_t)P1))>>15);

    if (var1 == 0){
        return; // avoid exception caused by division by zero
    }

    p = (((uint32_t)(((int32_t)1048576)-B_raw)-(var2>>12)))*3125;
    if (p < 0x80000000){
        p = (p << 1) / ((uint32_t)var1);
    } else {
        p = (p / (uint32_t)var1) * 2;
    }

    var1 = (((int32_t)P9) * ((int32_t)(((p>>3) * (p>>3))>>13)))>>12;
    var2 = (((int32_t)(p>>2)) * ((int32_t)P8))>>13;
    p = (uint32_t)((int32_t)p + ((var1 + var2 + P7) >> 4));

}

bool baro::readData(){

    // read(buffer, 2, BMP280_ADD, BMP280_TEMP_REG);
    // T_raw = (buffer[0] << 8 | buffer[1]);
    // read(buffer, 2, BMP280_ADD, BMP280_PRES_REG);
    // B_raw = (buffer[0] << 8 | buffer[1]);

    read(buffer, 3, BMP280_ADD, BMP280_TEMP_REG);
    T_raw = (buffer[0] << 12 | buffer[1] << 4 | buffer[2] >> 4);
    read(buffer, 3, BMP280_ADD, BMP280_PRES_REG);
    B_raw = (buffer[0] << 12 | buffer[1] << 4 | buffer[2] >> 4);

    return 0;
}

bool baro::setup_i2c(){
    i2c_config_t conf;
    conf.mode = I2C_MODE_MASTER;
    conf.sda_io_num = BMP280_SDA_IO;
    conf.scl_io_num = BMP280_SCL_IO;
    conf.scl_pullup_en = GPIO_PULLUP_ENABLE;
    conf.sda_pullup_en = GPIO_PULLUP_ENABLE;
    conf.master.clk_speed = BMP280_MASTER_FREQ_HZ;
    conf.clk_flags = 0;

    i2c_param_config(BMP280_I2C_PORT, &conf);
    i2c_driver_install(BMP280_I2C_PORT, conf.mode, 0, 0, 0);

    return 0;
};

esp_err_t baro::write(uint8_t* data_ptr, const size_t size, const uint8_t address, const uint8_t register_address){

    i2c_cmd_handle_t command = i2c_cmd_link_create();
    esp_err_t i2c_OK = ESP_OK;

    i2c_master_start(command);
    i2c_master_write_byte(command, (address << 1) | BMP280_I2C_WRITE_BIT, BMP280_I2C_ACK_CHECK_EN);
    i2c_master_write_byte(command, register_address, BMP280_I2C_ACK_CHECK_EN);
    i2c_master_write(command, data_ptr, size, BMP280_I2C_ACK_CHECK_EN);
    i2c_master_stop(command);

    i2c_OK = i2c_master_cmd_begin(BMP280_I2C_PORT, command, 50/portTICK_PERIOD_MS);
    i2c_cmd_link_delete(command);
    return i2c_OK;
};

esp_err_t baro::read(uint8_t* buffer_ptr, const size_t size, const uint8_t address, const uint8_t register_address){

    i2c_cmd_handle_t command = i2c_cmd_link_create();
    esp_err_t i2c_OK = ESP_OK;

    i2c_master_start(command);
    i2c_master_write_byte(command, (address << 1) | BMP280_I2C_WRITE_BIT, BMP280_I2C_ACK_CHECK_EN);
    i2c_master_write_byte(command, register_address, BMP280_I2C_ACK_CHECK_EN);

    i2c_master_start(command);
    i2c_master_write_byte(command, (address << 1) | BMP280_I2C_READ_BIT, BMP280_I2C_ACK_CHECK_EN);
    for (auto i = 0; i < size - 1; i++)
    {
        i2c_master_read_byte(command, buffer_ptr + i, BMP280_I2C_ACK_VAL);
    }
    i2c_master_read_byte(command, buffer_ptr + size - 1, BMP280_I2C_NACK_VAL);
    i2c_master_stop(command);

    i2c_OK = i2c_master_cmd_begin(BMP280_I2C_PORT, command, 50/portTICK_PERIOD_MS);
    i2c_cmd_link_delete(command);
    return i2c_OK;
};
