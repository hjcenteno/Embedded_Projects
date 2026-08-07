/*
    Author: Henry Centeno
    Description: This file contains the implementation of the blinking LED for the nucleo_g474re board.
*/

#include "stm32g474xx.h"
#include <inttypes.h>

#define ld2 (*((volatile uint32_t *)0x48000018ul)) //LD2 is connected to PA5, this is the address of the bssr register for GPIOA
#define RCC_AHB2ENR (*((volatile uint32_t *)0x4002104Cul)) //gpioa's clock
#define ld2_moder (*((volatile uint32_t *)0x48000000ul)) //the moder for ld2, which is pa5

void delay(uint32_t time){
    while(time--){}
}

int main(void){
    //setup
    RCC_AHB2ENR |= (1 << 0); //enable the clock bit while leaving every other bit untouched
    ld2_moder &= ~(0b11 << 10); //mode bits 11 and 10 of pa5, clear both of these bits
    ld2_moder |= (0b01 << 10); //set the mode bits to 01

    while(1){
        ld2 = (1 << 5); //set the pa5 pin to high
        delay(1000000); //delay for 5 seconds
        ld2 = (1 << (5 + 16)); //set the higher bits to low
        delay(500000); //delay for 5 seconds
    }
}