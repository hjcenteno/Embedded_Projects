/*
    Author: Henry Centeno
    Description:
        Implemenation of the knobs header file. It is the responsibility of the user to 
        ensure each pin of the kc11 rotary encoder share the same gpio peripheral cause I don't
        feel like handling that
*/

#include "knobs.h"

/* Doing (km.pinMasks & S1_MASK) will give us the pin number in the gpio, and the same can
 * be repeated for s2 and key. The idea is that we are isolating the bits for the pins of the
 * kc11 module with the mask.
*/

//for each transition, assume a bad input unless proven otherwise
KnobStates transitionFrom_idle(const KnobMachine *km, const uint32_t idr){
    //figure out how idle transitions to the other states

    //idle to invalid: key(s1 + s2)
    if(KNOB_KEY(idr, km->pinMasks) && (KNOB_S1(idr, km->pinMasks) || KNOB_S2(idr, km->pinMasks))){
        return invalid;
    }
    
    //idle to pressed: key(!s1(!s2)))
    if(KNOB_KEY(idr, km->pinMasks) && (!(KNOB_S1(idr, km->pinMasks)) && !(KNOB_S2(idr, km->pinMasks)))){
        return pressed;
    }

    //idle to cw: prevS1(s1) + s1(!s2)
    if(
        ((km->prevS1) && (KNOB_S1(idr, km->pinMasks))) || //prevS1(s1)
        ((KNOB_S1(idr, km->pinMasks)) && !(KNOB_S2(idr, km->pinMasks))) //s1(!s2)
    ){
        return cw;
    }

    //idle to ccw: !prevS1(s2) + !s1(s2)
    if(
        (!(km->prevS1) && (KNOB_S2(idr, km->pinMasks))) || 
        (!(KNOB_S1(idr, km->pinMasks)) && (KNOB_S2(idr, km->pinMasks)))
    ){
        return ccw;
    }

    //idle to idle: !key(!s1)(!s2)
    if(!KNOB_KEY(idr, km->pinMasks) && !KNOB_S1(idr, km->pinMasks) && !KNOB_S2(idr, km->pinMasks)){
        return idle;
    }

    return invalid; //returning invalid since the other states have those specific transition conditions
}

KnobStates transitionFrom_invalid(const KnobMachine *km, const uint32_t idr){
    //figure out how invalid transitions to the other states

    //invalid can only transition to idle
    if(!KNOB_KEY(idr, km->pinMasks) && !KNOB_S1(idr, km->pinMasks) && !KNOB_S2(idr, km->pinMasks)){
        return idle;
    }

    return invalid;
}

KnobStates transitionFrom_pressed(const KnobMachine *km, const uint32_t idr){
    //figure out how invalid pressed to the other states

    //pressed to idle: !key(!s1(!s2)))
    if(!KNOB_KEY(idr, km->pinMasks) && !KNOB_S1(idr, km->pinMasks) && !KNOB_S2(idr, km->pinMasks)){
        return idle;
    }

    //pressed to invalid: s1 + s2
    if(KNOB_S1(idr, km->pinMasks) || KNOB_S2(idr, km->pinMasks)){
        return invalid;
    }

    return invalid;
}

KnobStates transitionFrom_cw(const KnobMachine *km, const uint32_t idr){
    //figure out how cw transitions to the other states

    //cw to idle: !key(!s1)(!s2)
    if(!KNOB_KEY(idr, km->pinMasks) && !KNOB_S1(idr, km->pinMasks) && !KNOB_S2(idr, km->pinMasks)){
        return idle;
    }

    //cw to invalid: key
    if(KNOB_KEY(idr, km->pinMasks)){
        return invalid;
    }

    //cw to ccw: s2(!prevS1)
    if((KNOB_S2(idr, km->pinMasks)) && !(km->prevS1)){
        return ccw;
    }

    //cw to cw: s1(!s2) + !s1(s2)(prevs1)
    if(
        ((KNOB_S1(idr, km->pinMasks)) && !(KNOB_S2(idr, km->pinMasks))) || 
        (!(KNOB_S1(idr, km->pinMasks)) && (KNOB_S2(idr, km->pinMasks)) && (km->prevS1))
    ){
        return cw;
    }

    return invalid;
}

KnobStates transitionFrom_ccw(const KnobMachine *km, const uint32_t idr){
    //figure out how ccw transitions to the other states

    //ccw to idle: !key(!s1)(!s2)
    if(!KNOB_KEY(idr, km->pinMasks) && !KNOB_S1(idr, km->pinMasks) && !KNOB_S2(idr, km->pinMasks)){
        return idle;
    }

    //ccw to invalid: key
    if(KNOB_KEY(idr, km->pinMasks)){
        return invalid;
    }

    //ccw to cw: s1(s2) + !s2(s1)(prevS1)
    if(
        ((KNOB_S1(idr, km->pinMasks)) && (KNOB_S2(idr, km->pinMasks))) ||
        (!(KNOB_S2(idr, km->pinMasks)) && (KNOB_S1(idr, km->pinMasks)) && (km->prevS1))
    ){
        return cw;
    }

    //ccw to ccw: s2(!s1) + s2(!prevS1)
    if(
        ((KNOB_S2(idr, km->pinMasks)) && !(KNOB_S1(idr, km->pinMasks))) ||
        ((KNOB_S2(idr, km->pinMasks)) && !(km->prevS1))
    ){
       return ccw; 
    }

    return invalid;
}