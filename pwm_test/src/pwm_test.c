/*
    Author: Henry Centeno
    Description:
        This is to test that my implementation for the pwm is set up correctly

    Version 0.2:
        This will test the channel configuration functions

    Components | pin | peripheral
        led1   | ld2 | pa5
        led2   | d7  | pa8
        led3   | d8  | pa9
*/  

#include "system_time/timx_utils.h"
#define DC_STEP (1.0f) 

void init_components(void){
    //set up the pa5 as the user led on the mcu (ld2)
    RCC->AHB2ENR |= RCC_AHB2ENR_GPIOAEN; //first turn on the peripheral clock
    GPIOA->MODER &= ~(GPIO_MODER_MODE5 | GPIO_MODER_MODE8 | GPIO_MODER_MODE9);
    GPIOA->MODER |= (GPIO_MODER_MODE5_0 | GPIO_MODER_MODE8_0 | GPIO_MODER_MODE9_1); //set to gp out, but pa9 as af

    //set up the af10 for pa9 for tim2_ch3
    GPIOA->AFR[1] &= ~GPIO_AFRH_AFSEL9;
    GPIOA->AFR[1] |= (10u << GPIO_AFRH_AFSEL9_Pos);
}

int main(void){
    init_components();

    //set up the sysclck with 24 Mhz HSE
    if(activate_SWS_RCC_HSE() != 0){
        GPIOA->BSRR = GPIO_BSRR_BS5;
    }

    //configure channel 3 as a pwm
    RCC->APB1ENR1 |= (RCC_APB1ENR1_TIM2EN | RCC_APB1ENR1_TIM3EN);
    init_tim(TIM2);
    init_tim(TIM3);
    //set pwm in mode 1 since I will just be dimming an led
    //pe is set to preload the value written in ccr3
    uint32_t pwm_mask = ((0b110 << TIM_CCMR2_OC3M_Pos) | TIM_CCMR2_OC3PE);
    uint32_t ccer_mask = (TIM_CCER_CC3E); //enable the output channel
    //write a duty cycle of 100% then step down till 0% then step back up
    cfgr_tim_ccmr2(TIM2, pwm_mask, ccer_mask);
    
    uint32_t freq = 10000; //target pwm frequency
    TIM2->ARR = freq;
    TIM2->CR1 |= TIM_CR1_ARPE; //buffer the new arr
    TIM2->EGR |= TIM_EGR_UG; //update the tim
    TIM2->CCR3 = TIM2->ARR; //(ccr2 / (arr)) * 100 = duty_cycle
    
    float dc_percent = 100;
    float dc_step = -DC_STEP;
    while(1){
        dc_percent += dc_step;
        
        //dim to 0% then increase brightness after that
        if(dc_percent < MIN_DC_PERCENT){
            dc_step = DC_STEP;
            dc_percent = MIN_DC_PERCENT;
        }else if(dc_percent >= MAX_DC_PERCENT){
            dc_step = -DC_STEP;
            dc_percent = MAX_DC_PERCENT;
        }

        TIM2->CCR3 = update_pwm_dutyCycle(TIM2, dc_percent);
        delay_ms(TIM3, 10);
    }

    return 0;
}