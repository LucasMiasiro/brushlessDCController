#pragma once
#include "config.h"
#include "esp_err.h"


class baro {
private:
    esp_err_t write(uint8_t*, const size_t, const uint8_t, const uint8_t);
    esp_err_t read(uint8_t*, const size_t, const uint8_t, const uint8_t);

    bool setup_i2c();
    bool readData();
    void getBaroCal();
    void calcTruePressure();

    uint8_t buffer[24];
    float fbuffer;
    uint8_t opt;

    uint16_t T1, P1;
    int16_t T2, T3, P2, P3, P4, P5, P6, P7, P8, P9;

    int32_t T = 0, B_raw = 0, T_raw = 0, t_fine = 0;
    uint32_t p = 0;

public:

    baro();
    bool getData(float* p, float* T);
    bool accumulateData();
};
