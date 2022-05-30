#pragma once
#include "config.h"
// #include "driver/gpio.h"

class builtin_led{
public:
    builtin_led();
    void set_level(bool state);
    void blink(int n, bool endHigh);
};


class bldc{
    void setup();
public:
    bldc();
    void setPWM(uint16_t pwmDes);
};
