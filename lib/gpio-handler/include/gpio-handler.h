#pragma once
#include "config.h"
// #include "driver/gpio.h"

class builtin_led{
public:
    builtin_led();
    void set_level(bool state);
    void blink(int n, bool endHigh);
};

// class bldc{
//     void setup();
//     const uint8_t n;
// public:
//     bldc(const uint8_t N) : n(N) {setup();};
//     void setPWM(uint16_t pwmDes);
// };