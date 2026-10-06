/*
    Author: Henry Centeno
    Description:
        Implementation of the hcsr04 driver

    Components | Pins | Peripherals
    HC-SR04 VCC | 5V 
    HC-SR04 GND | GND
    HC-SR04 Trig | D2 | PA10
    HC-SR04 Echo | D3 | PB3
*/

#include "sensors/hcsr04_driver.h"

void init_HCSR04(hcsr04_t *hcsr04, uint32_t priority){
    //sets up the echo and trig pins, and sets the rising/falling edges to 0
    hcsr04->echoFallingEdge = 0;
    hcsr04->echoFallingEdge = 0;

    //activate the gpiob peripheral
    RCC->AHB2ENR |= RCC_AHB2ENR_GPIOBEN;

    //set up the trig and echo pins
    //trig configuration (pa10) set to gp output to drive it high then low to trigger
    GPIOA->MODER &= ~GPIO_MODER_MODE10;
    GPIOA->MODER |= GPIO_MODER_MODE10_0; //set to gp output
    
    //echo location (pb3) to tim_ch2 for input capture mode
    /*
    RM04 page 1315
    In input capture mode, the capture/compare registers (TIMx_CCRx) are used to latch the
    value of the counter after a transition detected by the corresponding ICx signal.
    */
    GPIOB->MODER &= ~GPIO_MODER_MODE3; //set pb3 to AF1
    GPIOB->MODER |= GPIO_MODER_MODE3_1; //'10' for AF
    GPIOB->AFR[0] &= ~GPIO_AFRL_AFRL3; //clearing the pin 3 bits in AFRL
    GPIOB->AFR[0] |= GPIO_AFRL_AFSEL3_0; //write 1 to use AF1
    
    //configure channel 2 for the tim2
    init_tim2_ch2_input(priority);
}

void trigger_HCSR04(void){
    //sets the trig pin to high
    //early return if the gpiob peripheral is not enabled
    if((RCC->AHB2ENR & RCC_AHB2ENR_GPIOBEN) == 0){
        return;
    }

    GPIOA->BSRR = GPIO_BSRR_BS10; //set it to high
    delay_us(10); //delay for 10 us per the manual for the HCSR04
    GPIOA->BSRR = GPIO_BSRR_BR10; //set it low
}

float getDistance_HCSR04(hcsr04_t *hcsr04){
    /*
        gets the distance in inches following this formula from the hcsr04 user manual
        distance = pulse width(uS) / 148 for inches
    */
    float distance = 0;
    uint32_t edgeDiff = hcsr04->echoFallingEdge - hcsr04->echoRisingEdge; //keep it as a uint32_t to avoid the conversion if it's not needed
    if(edgeDiff > 0){
        distance = (float)(edgeDiff) / 148;
    }

    return distance;
}