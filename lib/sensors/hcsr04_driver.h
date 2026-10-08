/*
    Author: Henry Centeno
    Description:
        Handles the implementation of the code that handles setting up, reading, and triggering the hcsr04 sensor. 
        Refer to the below chart of where to wire the sensors and to what pins.

    Components | Pins | Peripherals
    HC-SR04 VCC | 5V 
    HC-SR04 GND | GND
    HC-SR04 Trig | D2 | PA10
    HC-SR04 Echo | D3 | PB3
*/

#ifndef HCSR04_DRIVER_H
#define HCSR04_DRIVER_H

#include "common_includes/common_includes.h"
#include "system_time/mcu_time.h" //does not includes the .c file, so main still needs this include

typedef struct hcsr04_t{
    volatile uint32_t echoRisingEdge;
    volatile uint32_t echoFallingEdge;
}hcsr04_t;

void init_HCSR04(hcsr04_t *hcsr04, uint32_t priority);
void trigger_HCSR04(void);
float getDistance_HCSR04(const hcsr04_t *hcsr04);
static inline void hcsr04_IRQHandler(hcsr04_t *hcsr04){
    //inline to avoid adding an extra function call
    //tim2 read a rising/falling edge
    if(TIM2->SR & TIM_SR_CC2IF){ //check for CC2IF
        //check for a rising edge at the pin
        if(GPIOB->IDR & GPIO_IDR_ID3){
            hcsr04->echoRisingEdge = TIM2->CCR2; //get the count on the rising edge
        }else{
            hcsr04->echoFallingEdge = TIM2->CCR2; //get the count on the falling edge
        }
    }

    //clear the bit at the end
    TIM2->SR &= ~TIM_SR_CC2IF;
}

#endif