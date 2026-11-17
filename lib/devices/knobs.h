/*
    Author: Henry Centeno
    Description:
        Take the code from confirmKnobPins and make it into a library. 
        Allow for user specified pins for key, s1, s2. This will just handle the 
        state transitions of the knobs, the user is responsible for handling
*/

#ifndef KNOBS_H
#define KNOBS_H

#include "common_includes/common_includes.h"

#define S1_POS (0u)
#define S1_MASK (0xFu << S1_POS)
#define S1_MASK_0 (1u << S1_POS)
#define S1_MASK_1 (2u << S1_POS)
#define S1_MASK_2 (4u << S1_POS)
#define S1_MASK_3 (8u << S1_POS)

#define S2_POS (4u)
#define S2_MASK (0xFu << S2_POS)
#define S2_MASK_0 (1u << S2_POS)
#define S2_MASK_1 (2u << S2_POS)
#define S2_MASK_2 (4u << S2_POS)
#define S2_MASK_3 (8u << S2_POS)

#define KEY_POS (8u)
#define KEY_MASK (0xFu << KEY_POS)
#define KEY_MASK_0 (1u << KEY_POS)
#define KEY_MASK_1 (2u << KEY_POS)
#define KEY_MASK_2 (4u << KEY_POS)
#define KEY_MASK_3 (8u << KEY_POS)

typedef enum KnobStates : uint8_t{
    idle = 0,
    invalid = 1,
    pressed = 2, 
    cw = 3, //clockwise
    ccw = 4 //counter clockwise
}KnobStates;

/* For the pinMasks, each peripheral has a maximum of 16 possible pins. Each pin
 * ,therefore, can be represented as 4 bits. That would then be 3 knob pins * the 4 pins to be
 * 12 bits + and extra 4 for padding. 
 * bits [0:3]: s1 pin
 * bits [4:7]: s2 pin
 * bits [8:11]: key pin
 * bits [12:15]: Reserved
*/

//we can use the idr for the other inputs
typedef struct knobMachine{
    uint16_t pinMasks; //compact bit mask for s1, s2, and key
    uint8_t prevS1; //acts as the clock edge 
    KnobStates q;
} KnobMachine;

//ensure each knob pins share the same gpio peripheral
KnobStates transitionFrom_idle(const KnobMachine km, const uint32_t idr);
KnobStates transitionFrom_invalid(const KnobMachine km, const uint32_t idr);
KnobStates transitionFrom_pressed(const KnobMachine km, const uint32_t idr);
KnobStates transitionFrom_cw(const KnobMachine km, const uint32_t idr);
KnobStates transitionFrom_ccw(const KnobMachine km, const uint32_t idr);

#endif 