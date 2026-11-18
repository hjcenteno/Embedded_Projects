/*
    Author: Henry Centeno
    Description: This is to test the knobs header implementation
*/

#include "devices/knobs.h"

int main(void){
    KnobMachine km1 = {
        .pinMasks = 0,
        .prevS1 = 0,
        .q = idle
    };
    
    //let's say that this knob is on pa5, pa6, pa7
    km1.pinMasks |= (
        SET_PIN(5u, S1_POS) |
        SET_PIN(6u, S2_POS) |
        SET_PIN(7u, KEY_POS)
    );

    //prove that (km.pinMasks & S1_MASK) will give us 5
    volatile uint32_t a_idr = 0; //going to simulate some idr for gpioa

    //going to use qemu to simulate the behaviors and using gdp set var to change a_idr
    while(1){
        KnobStates loopState = km1.q;
        switch (km1.q){
        case idle:
            loopState = transitionFrom_idle(&km1, a_idr);
            break;
        case invalid:
            loopState = transitionFrom_invalid(&km1, a_idr);
            break;
        case pressed:
            loopState = transitionFrom_pressed(&km1, a_idr);
            break;
        case cw:
            loopState = transitionFrom_cw(&km1, a_idr);
            break;
        case ccw:
            loopState = transitionFrom_ccw(&km1, a_idr);
            break;
        default:
            km1.q = loopState;
            break;
        }

        //this is to avoid scan-build from complaining about loopState not being used
        if((loopState == idle) || (loopState == invalid)){
            continue;
        }

    }

    return 0;
}