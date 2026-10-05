/*
    Author: Henry Centeno
    version 1.0.0
    Description: Sets up a general purpose timer.

    notes:
    *I plan to in the future to add multi channel and multi tim support,
    but at the moment those features are not needed.
*/

#include "mcu_time.h"

static void setup_tim2(void){
    //setups the tim2, but ensures it is only set up once
    if(TIM2->CR1 & TIM_CR1_CEN){
        return; //no need to set it up again
    }

    //use a 24 MHz HSE
    RCC->CR &= ~RCC_CR_HSEON;
    RCC->CR |= RCC_CR_HSEON;
    while((RCC->CR & RCC_CR_HSERDY) == 0){} //wait until the hse is ready

    //select the 24 MHz hse
    /*
        Bits 3:2 SWS[1:0]: System clock switch status
        Set and cleared by hardware to indicate which clock source is used as system clock.
        00: Reserved, must be kept at reset value
        01: HSI16 oscillator used as system clock
        10: HSE used as system clock
        11: PLL used as system clock

        Bits 1:0 SW[1:0]: System clock switch
        Set and cleared by software to select system clock source (SYSCLK).
        Configured by hardware to force HSI16 oscillator selection when exiting Stop and Standby
        modes or in case of failure of the HSE oscillator.
        00: Reserved, must be kept at reset value
        01: HSI16 selected as system clock
        10: HSE selected as system clock
        11: PLL selected as system clock
    */
    /*
        one line since I was having issues where clearing the bits then setting it in two lines would reconfigure the
        HSI bit back to one. The one line ensures that the CFGR register is simply set to HSE in SW writig 1010 to bits[0:4] in the register.
        Earlier with the two line configuration, the bits would read 0111 meaning the software is requesting a PLL despite only setting the HSE bit.
        This is because the hardware was resetting the HSI bit on the next instruction when the bit was cleared.
    */
    RCC->CFGR = (RCC->CFGR & ~RCC_CFGR_SW) | RCC_CFGR_SW_1;
    while((RCC->CFGR & RCC_CFGR_SWS_HSE) == 0){} // wait while the status is set

    // //set up apb1 for 24 MHz
    RCC->APB1ENR1 &= ~RCC_APB1ENR1_TIM2EN;
    RCC->APB1ENR1 |= RCC_APB1ENR1_TIM2EN;

    //write the prescalar value to tim2
    // TIM2->PSC = PRESCALAR_VALUE_16MHZ;
    TIM2->PSC = PRESCALAR_VALUE_24MHZ;
    //write the arr reset value 
    TIM2->ARR = MAX_TIME;
    TIM2->EGR |= TIM_EGR_UG; //enable the ug bit to start the ug to reinitialize the counter register
}

static inline void enable_time2(void){
    //enable the tim
    TIM2->CR1 &= ~TIM_CR1_CEN;
    TIM2->CR1 |= TIM_CR1_CEN;
}

//todo: change this function to handle channels
void init_tim2(void){
    /* set up tim2 at 24 Mhz */
    setup_tim2();
    enable_time2();
}

//todo: make this funciton channel agnostic
void init_tim2_ch2_input(uint32_t priority){
    //set up tim2 channel 2 as input
    setup_tim2();

    //set up ch2 in input mode
    //following the procedure set up on page 1314 of the rm04
    TIM2->CCMR1 &= ~(TIM_CCMR1_CC2S);
    TIM2->CCMR1 |= TIM_CCMR1_CC2S_0; //01: CC2 channel is configured as input, tim_ic2 is mapped on tim_ti2.
    //enable the capture in the ccer register
    //CC1NP = 1, CC1P = 1:non-inverted/both edges. The circuit is sensitive to both TIxFP1 rising and falling edges
    TIM2->CCER &= ~(TIM_CCER_CC2P | TIM_CCER_CC2NP | TIM_CCER_CC2E);
    TIM2->CCER |= (TIM_CCER_CC2P | TIM_CCER_CC2NP | TIM_CCER_CC2E);
    
    //allow for interrupts
    TIM2->DIER &= ~(TIM_DIER_CC2IE);
    TIM2->DIER |= TIM_DIER_CC2IE;
    NVIC_EnableIRQ(TIM2_IRQn);
    NVIC_SetPriority(TIM2_IRQn, priority); //set the priority for the interrupt
    
    //enable the tim
    enable_time2();
}

uint32_t getTime(void){
    //return the cnt register of tim2
    return TIM2->CNT;
}

void delay_us(uint32_t microseconds){
    //amount of time to delay in microseconds (max of 2^32 -1 microseconds)
    /*
        max amount is simply because a uint32_t can only store a maximum of 2^32 - 1
    */
    if(microseconds > MAX_TIME){
        microseconds = MAX_TIME;
    }
    uint32_t startTime = getTime();
    uint32_t currentTime = startTime;

    //pause execution until we hit the time to delay 
    while((currentTime - startTime) < microseconds){
        currentTime = getTime();
    }
} 

//delay is done in microseconds, so when adding new delay functions, convert the time to microseconds)
void delay_ms(uint32_t milliseconds){
    //amount of time to delay in milliseconds (max 4294000 milliseconds)
    //the max amount comes from (2^32 -1) / 1000
    if(milliseconds > 4294000){
        milliseconds = 4294000; //capped at 4294000
    }


    uint32_t delay = milliseconds * MILLI_TO_MICRO;
    delay_us(delay);
} 

void delay_s(uint32_t seconds){
    //amount of time to delay in seconds (max one hour)
    //the max of one hour is because 2^32 - 1 microseconds is roughly 1.2 hours
    if(seconds > 3600){
        seconds = 3600; //capped at 60 seconds since 60 seconds is one hour
    }

    uint32_t delay = seconds * MICRO_TO_BASE;
    delay_us(delay);
} 
