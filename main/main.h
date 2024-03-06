#pragma once
// #include "../lib/PID/include/PID.h"
// #include "../lib/SMC2/include/SMC2.h"

enum controlMode {
    STOP,
    SHOULD_GO_UP,
    SHOULD_GO_DOWN,
    GO
};

struct rpmState{
    float rpmCurr;
    bool rpmCurr_isNew;
    float rpmDes;
    bool rpmDes_isNew;
};

struct controlData_ptr{
    float* currAngle_ptr;
    float* desAngle_ptr;
    float* p_ptr;
    float* T_ptr;
    bool* setZero_ptr;
    bool* killSwitch_ptr;
    bool* bypassAngMax_ptr;
    bool* homeWasSet_ptr;
    bool* shouldUse_CL_ptr;
    controlMode* controlMode_ptr;
    rpmState* rpmState0_ptr;
    rpmState* rpmState1_ptr;
    rpmState* rpmState2_ptr;
    uint16_t* pwmDes0_ptr;
    uint16_t* pwmDes1_ptr;
    uint16_t* pwmDes2_ptr;
};


void sendTask(void*);
void controlTask(void*);