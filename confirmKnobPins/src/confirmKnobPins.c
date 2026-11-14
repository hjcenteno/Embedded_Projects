/*
    Author: Henry Centeno
    Description:
        The purpose of this program is to let me know what exactly are the s1, s2, and key pins
        for the potentiometer I bought off of AliExpress. The pins I chose are because they are 
        specified as 5v tolerant in the datasheet.

    Components | Pin | peripheral
        s1 | D3 | pb3
        s2 | D4 | pb4
        key | D5 | pb5
*/

#include "uart_driver/lpuart_driver.h"
#include "system_time/mcu_time.h" //already has tim2 set up

/*
    This will hold the inputs for the state machine that describes how to differentiate
    between the behaviors of the knob. The state machine will act as a d flip flop with 
    the while loop acting in as the clock and using one of the pins of the knobs to define
    the edge of the clock, in this case s1 will act as the edge.

    The states of the knob are defined as:
        idle: nothing is happening
        invalid: user pressed and turned the knob, I don't want to handle that
        pressed: key is low
        clockwise: defined as pin A connecting with C, or s1 to s2
        counter clockwise: defined as pin B connecting with C, or s2 to s1

    The inputs of the state machine would then be key, s1, s2, prevS1, and q to output nextQ
    and the direction of the knob turning.
*/
typedef enum KnobStates : uint8_t{
    idle = 0,
    invalid = 1,
    pressed = 2, 
    cw = 3, //clockwise
    ccw = 4 //counter clockwise
}KnobStates;

//we can use the idr for the other inputs
static struct KnobMachine{
    uint8_t prevS1; //acts as the clock edge 
    KnobStates q;
}knobMachine;

void init_gpioPins(void){
    RCC->AHB2ENR |= RCC_AHB2ENR_GPIOBEN;

    //set their modes to gp input with pull down for logic high 
    GPIOB->MODER &= ~(GPIO_MODER_MODE3 | GPIO_MODER_MODE4 | GPIO_MODER_MODE5);
    GPIOB->PUPDR &= ~(GPIO_PUPDR_PUPD3 | GPIO_PUPDR_PUPD4 | GPIO_PUPDR_PUPD5);
}

int main(void){
    init_tim2();
    init_gpioPins();
    init_lpuart();
    
    //knob initialization on start up
    knobMachine.prevS1 = 0;
    knobMachine.q = idle;
    
    float rotationAmt = 0; //amount changed by the rotation of the knob
    float delta = 0; //the quantity that rotationAmt changes by

    while (1){
        //since the pins are pull-ups, a 0 reads that its an input, so flipping the idr will make the bits read 1 on input
        uint32_t b_idr = ~GPIOB->IDR; //get a note of the idr
        KnobStates nextState = knobMachine.q; //to avoid state mutations from handling

        //determine the state for this iteration of the loop
        switch(nextState){
            case idle:
                nextState = transitionFrom_idle(b_idr);
                break;
            case invalid:
                nextState = transitionFrom_invalid(b_idr);
                break;
            case pressed:
                nextState = transitionFrom_pressed(b_idr);
                break;
            case cw:
                nextState = transitionFrom_cw(b_idr);
                break;
            case ccw:
                nextState = transitionFrom_ccw(b_idr);
                break;
            default:
                break;
        }

        //update the knob machines
        knobMachine.q = nextState;
        knobMachine.prevS1 = b_idr & GPIO_IDR_ID3; //pins use a pull-up for logic low, so a 0 on a pin means its an input

        //handle the state behavior
        switch(nextState){
            case idle:
            case invalid:
                continue; //do nothing
            case pressed:
                nextState = transitionFrom_pressed(b_idr);
                break;
            case cw:
                nextState = transitionFrom_cw(b_idr);
                break;
            case ccw:
                nextState = transitionFrom_ccw(b_idr);
                break;
            default:
                break;
        }

        //output
    }

    return 0;
}