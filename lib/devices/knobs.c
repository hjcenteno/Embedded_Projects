/*
    Author: Henry Centeno
    Description:
        Implemenation of the knobs header file. It is the responsibility of the user to 
        ensure each pin of the kc11 rotary encoder share the same gpio peripheral cause I don't
        feel like handling that case.
*/

#include "knobs.h"

/* Doing (km.pinMasks & S1_MASK) will give us the pin number in the gpio, and the same can
 * be repeated for s2 and key. The idea is that we are isolating the bits for the pins of the
 * kc11 module with the mask.
*/

//for each transition, assume a bad input unless proven otherwise
KnobStates transitionFrom_idle(const KnobMachine km, const uint32_t idr){
    //figure out how idle transitions to the other states

    //idle to invalid: key(s1 + s2)
    if((idr & (km.pinMasks & KEY_MASK)) && (idr & (km.pinMasks & (S1_MASK | S2_MASK)))){
        return invalid;
    }
    
    //idle to pressed: key(!s1(!s2)))
    if((idr & (km.pinMasks & KEY_MASK)) && (!(idr & (km.pinMasks & S1_MASK)) && !((km.pinMasks & S2_MASK)))){
        return pressed;
    }

    //idle to cw: prevS1(S1) + s1(!s2)
    if(
        (km.prevS1 && (idr & (km.pinMasks & S1_MASK))) || //prevS1(S1)
        ((idr & (km.pinMasks & S1_MASK)) && !((km.pinMasks & S2_MASK)))
    ){
        return cw;
    }

    //idle to ccw: !prevS1(s2) + !s1(s2)
    if(
        (!km.prevS1 && ((km.pinMasks & S2_MASK))) || //!prevS1(s2)
        (!(idr & (km.pinMasks & S1_MASK)) && ((km.pinMasks & S2_MASK)))
    ){
        return ccw;
    }

    //idle to idle: !key(!s1)(!s2)
    if(!(idr & (km.pinMasks & KEY_MASK)) && !(idr & (km.pinMasks & S1_MASK)) && (!((km.pinMasks & S2_MASK)))){
        return idle;
    }

    return invalid; //returning invalid since the other states have those specific transition conditions
}

KnobStates transitionFrom_invalid(const KnobMachine km, const uint32_t idr){
    //figure out how invalid transitions to the other states

    //invalid can only transition to idle
    if(!(idr & (km.pinMasks & KEY_MASK)) && !(idr & (km.pinMasks & S1_MASK)) && (!((km.pinMasks & S2_MASK)))){
        return idle;
    }

    return invalid;
}

KnobStates transitionFrom_pressed(const KnobMachine km, const uint32_t idr){
    //figure out how invalid pressed to the other states

    //pressed to idle: !key(!s1(!s2)))
    if(!(idr & (km.pinMasks & KEY_MASK)) && !(idr & (km.pinMasks & S1_MASK)) && (!((km.pinMasks & S2_MASK)))){
        return idle;
    }

    //pressed to invalid: s1 + s2
    if((idr & (km.pinMasks & S1_MASK)) || ((km.pinMasks & S2_MASK))){
        return invalid;
    }

    return invalid;
}

KnobStates transitionFrom_cw(const KnobMachine km, const uint32_t idr){
    //figure out how cw transitions to the other states

    //cw to idle: !key(!s1)(!s2)
    if(!(idr & (km.pinMasks & KEY_MASK)) && !(idr & (km.pinMasks & S1_MASK)) && (!((km.pinMasks & S2_MASK)))){
        return idle;
    }

    //cw to invalid: key
    if(idr & (km.pinMasks & KEY_MASK)){
        return invalid;
    }

    //cw to ccw: s2(!prevS1)
    if(((km.pinMasks & S2_MASK)) && (!km.prevS1)){
        return ccw;
    }

    //cw to cw: s1(!s2) + !s1(s2)(prevs1)
    if(
        ((idr & (km.pinMasks & S1_MASK)) && !((km.pinMasks & S2_MASK))) || //s1(!s2)
        (!(idr & (km.pinMasks & S1_MASK)) && ((km.pinMasks & S2_MASK)) && km.prevS1)
    ){
        return cw;
    }

    return invalid;
}

KnobStates transitionFrom_ccw(const KnobMachine km, const uint32_t idr){
    //figure out how ccw transitions to the other states

    //ccw to idle: !key(!s1)(!s2)
    if(!(idr & (km.pinMasks & KEY_MASK)) && !(idr & (km.pinMasks & S1_MASK)) && (!((km.pinMasks & S2_MASK)))){
        return idle;
    }

    //ccw to invalid: key
    if(idr & (km.pinMasks & KEY_MASK)){
        return invalid;
    }

    //ccw to cw: s1(s2) + !s2(s1)(prevS1)
    if(
        ((idr & (km.pinMasks & S1_MASK)) && ((km.pinMasks & S2_MASK))) ||
        (!((km.pinMasks & S2_MASK)) && (idr & (km.pinMasks & S1_MASK)) && km.prevS1)
    ){
        return cw;
    }

    //ccw to ccw: s2(!s1) + s2(!prevS1)
    if(
        (((km.pinMasks & S2_MASK)) && !(idr & (km.pinMasks & S1_MASK))) ||
        (((km.pinMasks & S2_MASK)) && !km.prevS1)
    ){
       return ccw; 
    }

    return invalid;
}