/*
    Author: Henry Centeno
    Description: This file contains the implementation of the blinking LED for the nucleo_g474re board.
*/

#include "stm32g474xx.h"
#include <inttypes.h>

void delay(uint32_t time){
    while(time--){}
}

int main(void){
    //setup
    GPIOA->MODER &= ~GPIO_MODER_MODE5; //mode bits 11 and 10 of PA5, clear both of these bits
    GPIOA->MODER |= GPIO_MODER_MODE5_0; //set the mode bits to 01
    GPIOA->MODER &= ~GPIO_MODER_MODE8; //mode bits 11 and 10 of PA5, clear both of these bits
    GPIOA->MODER |= GPIO_MODER_MODE8_0; //set the mode bits to 01

    while(1){
        // GPIOA_BSRR = (1 << pinBit) | (1 << ld2); //set the pin high
        GPIOA->BSRR = GPIO_BSRR_BS5 | GPIO_BSRR_BS8;
        delay(1000000);
        // GPIOA_BSRR = (1 << (pinBit + pa_offset)) | (1 << (ld2 + pa_offset)); //set the pin low
        GPIOA->BSRR = GPIO_BSRR_BR5 | GPIO_BSRR_BR8;
        delay(500000);
    }
}