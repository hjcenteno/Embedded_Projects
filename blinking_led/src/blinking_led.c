/*
    Author: Henry Centeno
    Description: This file contains the implementation of the blinking LED for the nucleo_g474re board.
*/

#include "stm32g474xx.h"
#include <inttypes.h>

#define GPIOA_BSRR (*((volatile uint32_t *)0x48000018ul)) //bsrr register for GPIOA
#define RCC_AHB2ENR (*((volatile uint32_t *)0x4002104Cul)) //gpioa's clock
#define GPIOA_MODER (*((volatile uint32_t *)0x48000000ul)) //moder register for GPIOA

void delay(uint32_t time){
    while(time--){}
}

int main(void){
    //setup
    RCC_AHB2ENR |= (1 << 0); //enable the clock bit while leaving every other bit untouched
    GPIOA_MODER &= ~(0b11 << 10); //mode bits 11 and 10 of PA5, clear both of these bits
    GPIOA_MODER |= (0b01 << 10);  //set the mode bits to 01

    
    uint8_t pinBit = 5; //PA5
    uint8_t pa_offset = 16;

    while(1){
        GPIOA_BSRR = (1 << pinBit); //set the pin high
        delay(1000000);
        GPIOA_BSRR = (1 << (pinBit + pa_offset)); //set the pin low
        delay(500000);
    }
}