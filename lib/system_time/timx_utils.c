/*
    Author: Henry Centeno
    Description:
        Implementation of the timx_utils header file with similar logic to the mcu_time implementation
*/

#include "system_time/timx_utils.h"

/* This function is called to activate the hse clock at 24 MHz to be set up as the system
 * clock for the entire mcu
 */
int activate_SWS_RCC_HSE(void){
    //turn on the hse clock
    const uint32_t MAX_TIMEOUT = 0xffffffff; //amount of cpu cycles for the while loops since we do not have a delay at the moment
    uint32_t i = 0;

    RCC->CR &= ~RCC_CR_HSEON;
    RCC->CR |= RCC_CR_HSEON;
    //wait for the hse to stabilize 
    while((RCC->CR & RCC_CR_HSERDY) == 0){
        //break the loop if failed to stabilize
        if((++i) == MAX_TIMEOUT){
            return 1;
        }
    }
    i = 0; //reset i for the next loop
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

    //configure the hse as the system wide clock
    RCC->CFGR = (RCC->CFGR & ~RCC_CFGR_SW) | RCC_CFGR_SW_1;
    //wait until the mcu acknowledges the hse is the system clock
    while((RCC->CFGR & RCC_CFGR_SWS_HSE) == 0){
        //return if the mcu fails to acknowledge the hse as the sysclock
        if((++i) == MAX_TIMEOUT){
            return 1;
        }
    }

    return 0;
}

int init_tim(TIM_TypeDef *timX){
    //check if the parameter passed was a valid tim
    if(!IS_TIM_INSTANCE(timX)){
        return 1;
    }

    //determine what prescalar to use depending on the system clock
    if(RCC->CFGR & RCC_CFGR_SWS_HSE){
        timX->PSC = PRESCALAR_VALUE_24MHZ;
    }else{
        timX->PSC = PRESCALAR_VALUE_16MHZ;
    }

    //set the arr to max time
    const uint32_t mask = IS_TIM_32B_COUNTER_INSTANCE(timX) ? MAX_TIME_32 : MAX_TIME_16;
    timX->ARR = mask;
    //update the generation to start counting time
    timX->EGR |= TIM_EGR_UG;

    //enable the tim
    timX->CR1 &= ~TIM_CR1_CEN;
    timX->CR1 |= TIM_CR1_CEN;

    return 0;
}

void delay_us(TIM_TypeDef *timX, uint32_t microseconds){
    //amount of time to delay in microseconds (max of 2^32 -1 microseconds)
    /*
        max amount is simply because a uint32_t can only store a maximum of 2^32 - 1
    */
    //early return if tim not provided
    if(!IS_TIM_INSTANCE(timX)){
        return;
    }

    const uint32_t mask = IS_TIM_32B_COUNTER_INSTANCE(timX) ? MAX_TIME_32 : MAX_TIME_16;
    uint32_t startTime = getTime(timX);
    while(microseconds > mask){
        startTime = getTime(timX);
        while(((getTime(timX) - startTime) & mask) < mask){}
        microseconds -= mask;
    }

    //pause execution until we hit the time to delay
    startTime = getTime(timX);
    while(((getTime(timX) - startTime) & mask) < microseconds){}
} 

//delay is done in microseconds, so when adding new delay functions, convert the time to microseconds)
void delay_ms(TIM_TypeDef *timX, uint32_t milliseconds){
    //amount of time to delay in milliseconds (max 4294000 milliseconds)
    //the max amount comes from (2^32 -1) / 1000
    if(milliseconds > 4294000){
        milliseconds = 4294000; //capped at 4294000
    }


    uint32_t delay = milliseconds * MILLI_TO_MICRO;
    delay_us(timX, delay);
} 

void delay_s(TIM_TypeDef *timX, uint32_t seconds){
    //amount of time to delay in seconds (max one hour)
    //the max of one hour is because 2^32 - 1 microseconds is roughly 1.2 hours
    if(seconds > 3600){
        seconds = 3600; //capped at 60 seconds since 60 seconds is one hour
    }

    uint32_t delay = seconds * BASE_TO_MICRO;
    delay_us(timX, delay);
} 