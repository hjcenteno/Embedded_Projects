/*
    Author: Henry Centeno
    Description: This is to test the knobs header implementation
*/

#include "devices/knobs.h"

int main(void){
    KnobMachine km1 = {
        .pinMasks = 0,
        .prevS1 = 0,
        .q = 0  
    };
    
    //let's say that this knob is on pa5, pa6, pa7
    km1.pinMasks |= (
        (5u << S1_POS) |
        (6u << S2_POS) |
        (7u << KEY_POS)
    );


    //prove that (km.pinMasks & S1_MASK) will give us 5
    km1.pinMasks & S1_MASK;
}