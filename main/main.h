#pragma once
// #include "../lib/PID/include/PID.h"
// #include "../lib/SMC2/include/SMC2.h"

enum controlMode {
    STOP,
    SHOULD_GO_UP,
    SHOULD_GO_DOWN,
    GO
};

struct controlData_ptr{
    float* currAngle_ptr;
    float* desAngle_ptr;
    bool* setZero_ptr;
    bool* killSwitch_ptr;
    bool* bypassAngMax_ptr;
    controlMode* controlMode_ptr;
};


void sendTask(void*);
void controlTask(void*);