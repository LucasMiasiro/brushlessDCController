#pragma once

struct controlData_ptr{
    uint16_t* pwmDes_ptr;
};

void controlTask(void*);
void sendTask(void*);