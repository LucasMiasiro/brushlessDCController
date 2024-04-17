#include "SMC2.h"
#include <iostream>
#include <math.h>

template <typename T> int sgn(T val) {
    return (T(0) < val) - (val < T(0));
}

uint8_t SMC2::boundOut(){
    if (out <= minOut) {
        out = minOut;
        return 1;
    } else if (out >= maxOut) {
        out = maxOut;
        return 2;
    }
    return 0;
}

float SMC2::get(){
    return out+offset;
}

void SMC2::update(float refin, float measin, float isNewin){
    this->ref = refin;
    this->meas = measin;
    this->isNew = isNewin;

    if (ref < minIn) {
        if (ref < 0.9f) {
            out = -offset;
        } else {
            out = -offset + pwm_idle;
        }
        e_prev = 0;
        F2_dot_prev = 0;
        F2 = 0;
        return;
    }

#if ENABLE_BLDC_SAN_CHECK
    if (isNew) {
        sanityCount = 0;
    } else {
        sanityCount++;
    }

    // OBS: É necessário que a hélice esteja rodando quando se envia PWM_SPOOLUP
    if (sanityCount >= BLDC_MAX_SANITY_COUNTER) {
        sanityCount = BLDC_MAX_SANITY_COUNTER; 
        out = -offset + pwm_spoolup;
        e_prev = 0;
        F2_dot_prev = 0;
        F2 = 0;
        return;
    }
#endif

    // Phase 1 - Sliding Surface
    e = ref - meas;

#if ENABLE_BLDC_TOL
    if ((e < tol) && (e > -tol)) {
        boundOut();
        return;
    }
#endif

#if BLDC_ERROR_FILTER
    e = k_filter*(e - e_prev) + e_prev;
#endif
    sigma = e + c1*(e - e_prev)/dt;

    // Phase 2 - Super Twisting
    e_sign = sgn(e);
    F1 = sqrt(Fstar * abs(e)) * e_sign;
    F2_dot = Fstar * FI * e_sign;
    
    switch (boundOut()){
        case 0:
            F2 += (F2_dot + F2_dot_prev) * dt/2;
            break;
        case 1:
        case 2:
            F2 = F2*0.98;
            break;
    }

    out = F1 + F2;
    
    e_prev = e;
    F2_dot_prev = F2_dot;
    boundOut();
}