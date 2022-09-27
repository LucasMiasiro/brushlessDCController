#pragma once
// #include "../lib/PID/include/PID.h"
// #include "../lib/SMC2/include/SMC2.h"


struct controlData_ptr{
    float* currAngle_ptr;
};

void sendTask(void*);
void readEncoderTask(void*);