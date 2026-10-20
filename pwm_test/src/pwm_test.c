/*
    Author: Henry Centeno
    Description:
        This is to test that my implementation for the pwm is set up correctly
*/

#include "system_time/timx_utils.h"

int main(void){
    // tim2_init_pwm(frequency, duty_cycle, arr);
    
    //set up the pa5 as the user led on the mcu (ld2)
    RCC->AHB2ENR |= RCC_AHB2ENR_GPIOAEN; //first turn on the peripheral clock
    GPIOA->MODER &= ~GPIO_MODER_MODE5;
    GPIOA->MODER |= GPIO_MODER_MODE5_0; //set to gp out

    if(activate_SWS_RCC_HSE() == 0){ //test if the function works
        //turn the led on if it works
        GPIOA->BSRR = GPIO_BSRR_BS5;
    }

    return 0;
}