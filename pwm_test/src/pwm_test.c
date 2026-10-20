/*
    Author: Henry Centeno
    Description:
        This is to test that my implementation for the pwm is set up correctly
*/

#include "system_time/timx_utils.h"

void init_leds(void){
    //set up the pa5 as the user led on the mcu (ld2)
    RCC->AHB2ENR |= RCC_AHB2ENR_GPIOAEN; //first turn on the peripheral clock
    GPIOA->MODER &= ~(GPIO_MODER_MODE5 | GPIO_MODER_MODE8 | GPIO_MODER_MODE9);
    GPIOA->MODER |= (GPIO_MODER_MODE5_0 | GPIO_MODER_MODE8_0 | GPIO_MODER_MODE9_0); //set to gp out
}

int main(void){
    // tim2_init_pwm(frequency, duty_cycle, arr);
    init_leds();
    
    // if(activate_SWS_RCC_HSE() == 1){ //test if the function works
    //     //turn the led on if it fails
    //     GPIOA->BSRR = GPIO_BSRR_BS5;
    // }

    //since tim2 has been set up before, this will test tim3
    //enable tim3
    // RCC->APB1ENR1 |= RCC_APB1ENR1_TIM5EN;
    RCC->APB2ENR |= RCC_APB2ENR_TIM17EN;
    if(init_tim(TIM17) == 1){
        //keep the led on if it fails
        GPIOA->BSRR = GPIO_BSRR_BS8;
    }

    while(1){
        //blink it if it works
        GPIOA->BSRR = GPIO_BSRR_BS9;
        delay_s(TIM17, 3);
        GPIOA->BSRR = GPIO_BSRR_BR9;
        delay_s(TIM17, 2);
    }

    return 0;
}