#pragma once
#include "../lib/PID/include/PID.h"

struct rpmState{
    float rpmCurr;
    bool rpmCurr_isNew;
    float rpmDes;
    bool rpmDes_isNew;
};

struct controlData_ptr{
    uint16_t* pwmDes_ptr;
    rpmState* rpmState_ptr;
    PID* pid_ptr;
};

void controlTask(void*);
void sendTask(void*);
void readRPMTask(void*);