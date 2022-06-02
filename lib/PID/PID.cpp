#include "PID.h"
#include <iostream>

uint8_t PID::boundOut(){
    if (out <= minOut) {
        out = minOut;
        return 1;
    } else if (out >= maxOut) {
        out = maxOut;
        return 2;
    }
    return 0;
}

void PID::resetI(){
    I = 0;
}

void PID::resetI(float value){
    I = value;
}

float PID::get(){
    return out+offset;
}

void PID::update(float refin, float measin, float isNewin){
    this->ref = refin;
    this->meas = measin;
    this->isNew = isNewin;

    if (ref < minIn) {
        out = -offset;
        out_prev = -offset;
        e = 0.0f;
        e_prev = 0.0f;
        hasReachedMinIn = false;
        resetI();
        return;
    }

    if (out < 0.0f) {
       out = 0.0f;
       out_prev = 0.0f; 
    }

    if (!isNew && hasReachedMinIn){
        dt_accum += dt;
        resetI();
        return;
    } else {
        dt_accum = dt;
    }
    
    if (meas < minIn) {
        out += dt_accum*RAMP;
        boundOut();
        hasReachedMinIn = false;
        resetI();
        return;
    } else if (!hasReachedMinIn) {
        if (out > 1.0f) {
            FF = out/meas;
        } 
        if (FF > maxFF) {
            FF = maxFF;
        }
        hasReachedMinIn = true;
    }
    FF = 200.0f/2000.0f;

    e = ref - meas;
    P = e*KP;
    
    switch (boundOut()){
        case 0:
            I += (e + e_prev)/2 * dt_accum * KI;
            break;
        case 1:
        case 2:
            I = I - 0.1f * I;
            break;
    }

    if (I > Imax) {
        I = Imax;
    } else if (I < -Imax) {
        I = -Imax;
    }

    // std::cout << "P" << P << std::endl;
    // std::cout << "I" << I << std::endl;
    // std::cout << "FF" << (ref*FF) << std::endl;
    out_prev = out;
    out = P + I + ref*FF;
    e_prev = e;
    boundOut();
}