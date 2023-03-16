#include "baro.h"
#include "driver/i2c.h"
#include "config.h"

bool baro::accumulateData(){
    if (readData()){
        return 1;
    }
    return 0;
}

bool baro::getData(float* p_out, float* T_out){

    baroState = SHOULD_READ_TEMP;

    if (N_samples == 0){
        calcTruePressure();
        return 1;
    }

    calcTruePressureAccum();
    *p_out = (float)p;
    *T_out = (float)T;
    cleanAccum();
    return 0;
}

bool baro::cleanAccum(){
    B_raw_accum = 0;
    N_samples = 0;
    return 0;
}

baro::baro(){
    setup_i2c();
    getBaroCal();
    setPressureMeas();
};

void baro::getBaroCal(){
    read(buffer, 22, BMP280_ADD, BMP280_CALIB_REG);
    AC1 = (buffer[0] << 8 | buffer[1]);
    AC2 = (buffer[2] << 8 | buffer[3]);
    AC3 = (buffer[4] << 8 | buffer[5]);
    AC4 = (buffer[6] << 8 | buffer[7]);
    AC5 = (buffer[8] << 8 | buffer[9]);
    AC6 = (buffer[10] << 8 | buffer[11]);
    B1 = (buffer[12] << 8 | buffer[13]);
    B2 = (buffer[14] << 8 | buffer[15]);
    MB = (buffer[16] << 8 | buffer[17]);
    MC = (buffer[18] << 8 | buffer[19]);
    MD = (buffer[20] << 8 | buffer[21]);
}

void baro::calcTruePressureAccum(){
    X1 = (T_raw - AC6) * AC5 / 32768;
    X2 = MC * 2048 / (X1 + MD);
    B5 = X1 + X2;
    T = (B5 + 8) / 16;

    B6 = B5 - 4000;
    X1 = (B2 * (B6 * B6 / 4096)) / 2048;
    X2 = AC2 * B6 / 2048;
    X3 = X1 + X2;
    B3 = (AC1 * 4 + X3 + 2) / 4;
    X1 = AC3 * B6 / 8192;
    X2 = (B1 * (B6 * B6 / 4096)) / 65536;
    X3 = (X1 + X2 + 2) / 4;
    B4 = AC4 * (uint32_t)(X3 + 32768) / 32768;
    B7 = ((uint32_t)(B_raw_accum/N_samples) - B3) * 50000;
    if (B7 < 0X80000000) {
        p = (B7 * 2) / B4;
    } else {
        p = (B7 / B4) * 2;
    }
    X1 = (p / 256) * (p / 256);
    X1 = (X1 * 3038) / 65536;
    X2 = (-7357 * p) / 65536;
    p = p + (X1 + X2 + 3791) / 16;
}

void baro::calcTruePressure(){
    X1 = (T_raw - AC6) * AC5 / 32768;
    X2 = MC * 2048 / (X1 + MD);
    B5 = X1 + X2;
    T = (B5 + 8) / 16;

    B6 = B5 - 4000;
    X1 = (B2 * (B6 * B6 / 4096)) / 2048;
    X2 = AC2 * B6 / 2048;
    X3 = X1 + X2;
    B3 = (AC1 * 4 + X3 + 2) / 4;
    X1 = AC3 * B6 / 8192;
    X2 = (B1 * (B6 * B6 / 4096)) / 65536;
    X3 = (X1 + X2 + 2) / 4;
    B4 = AC4 * (uint32_t)(X3 + 32768) / 32768;
    B7 = ((uint32_t)(B_raw) - B3) * 50000;
    if (B7 < 0X80000000) {
        p = (B7 * 2) / B4;
    } else {
        p = (B7 / B4) * 2;
    }
    X1 = (p / 256) * (p / 256);
    X1 = (X1 * 3038) / 65536;
    X2 = (-7357 * p) / 65536;
    p = p + (X1 + X2 + 3791) / 16;
}

void baro::setPressureMeas(){
    opt = BMP280_CONFIG_01_OPT;
    write(&opt, 1, BMP280_ADD, BMP280_CONFIG_01_ADD);
}

void baro::setTemperatureMeas(){
    opt = BMP280_CONFIG_02_OPT;
    write(&opt, 1, BMP280_ADD, BMP280_CONFIG_02_ADD);
}

bool baro::readData(){

    switch (baroState) {
        case (SHOULD_READ_TEMP):
            setTemperatureMeas();
            baroState = WAIT_FOR_TEMP;
            break;

        case (WAIT_FOR_TEMP):
            baroState = READ_TEMP;
            break;

        case (READ_TEMP):
            read(buffer, 2, BMP280_ADD, BMP280_REG);
            T_raw = (buffer[0] << 8 | buffer[1]);
            baroState = SHOULD_READ_PRESSURE;
            break;

        case (SHOULD_READ_PRESSURE):
            setPressureMeas();
            baroState = WAIT_FOR_PRESSURE;
            break;

        case (WAIT_FOR_PRESSURE):
            baroState = READ_PRESSURE;
            break;

        case (READ_PRESSURE):
            read(buffer, 2, BMP280_ADD, BMP280_REG);
            B_raw = (buffer[0] << 8 | buffer[1]);
            B_raw_accum += B_raw;
            N_samples++;
            baroState = SHOULD_READ_PRESSURE;
            break;
    }

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
