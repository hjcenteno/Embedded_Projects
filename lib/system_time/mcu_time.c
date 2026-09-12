/*
    Author: Henry Centeno
    Description: Sets up a general purpose timer
*/

#include "mcu_time.h"

void init_tim2(void){
    /* set up tim2 at 16 Mhz */

    //set up apb1 for 16 MHz
    RCC->APB1ENR1 &= ~RCC_APB1ENR1_TIM2EN;
    RCC->APB1ENR1 |= RCC_APB1ENR1_TIM2EN;

    //write the prescalar value to tim2
    TIM2->PSC = PRESCALAR_VALUE_16MHZ;
    //write the arr reset value 
    TIM2->ARR = MAX_TIME;
    TIM2->EGR |= TIM_EGR_UG; //enable the ug bit to start the ug to reinitialize the counter register

    //enable the tim
    TIM2->CR1 &= ~TIM_CR1_CEN;
    TIM2->CR1 |= TIM_CR1_CEN;
}

uint32_t getTime(void){
    //return the cnt register of tim2
    return TIM2->CNT;
}

void delay_us(uint32_t microseconds){
    //amount of time to delay in microseconds
    uint32_t startTime = getTime();
    uint32_t currentTime = startTime;

    //pause execution until we hit the time to delay 
    while((currentTime - startTime) < microseconds){
        currentTime = getTime();
    }
} 

//delay is done in microseconds, so when adding new delay functions, convert the time to microseconds)
void delay_ms(uint32_t milliseconds){
    //amount of time to delay in milliseconds
    uint32_t delay = milliseconds * MILLI_TO_MICRO;
    delay_us(delay);
} 

void delay_s(uint32_t seconds){
    //amount of time to delay in seconds
    uint32_t delay = seconds * MICRO_TO_BASE;
    delay_us(delay);
} 
