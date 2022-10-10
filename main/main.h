#pragma once
// #include "../lib/PID/include/PID.h"
// #include "../lib/SMC2/include/SMC2.h"


struct controlData_ptr{
    float* currAngle_ptr;
    float* desAngle_ptr;
    bool* setZero_ptr;
    bool* shouldStop_ptr;
};

enum controlMode {
    SHOULD_GO_UP,
    SHOULD_GO_DOWN,
    GO,
    STOP
};

void sendTask(void*);
void controlTask(void*);