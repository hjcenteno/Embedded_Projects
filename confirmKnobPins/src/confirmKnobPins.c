/*
    Author: Henry Centeno
    Description:
        The purpose of this program is to let me know what exactly are the s1, s2, and key pins
        for the potentiometer I bought off of AliExpress. The pins I chose are because they are 
        specified as 5v tolerant in the datasheet.

    Components | Pin | peripheral
        s1 | D3 | pb3
        s2 | D4 | pb4
        key | D5 | pb5
*/

#include "uart_driver/lpuart_driver.h"
#include "system_time/mcu_time.h" //already has tim2 set up

void init_gpioPins(void){
    RCC->AHB2ENR |= RCC_AHB2ENR_GPIOBEN;

    //set their modes to gp input with pull down for logic high 
    GPIOB->MODER &= ~(GPIO_MODER_MODE3 | GPIO_MODER_MODE4 | GPIO_MODER_MODE5);
    GPIOB->PUPDR &= ~(GPIO_PUPDR_PUPD3 | GPIO_PUPDR_PUPD4 | GPIO_PUPDR_PUPD5);
}

int main(void){
    init_tim2();
    init_gpioPins();
    init_lpuart();
    
    uint32_t lastState = 0;
    while (1){
        //transmit the idr register
        //the pins on the module read high
        uint32_t idr_b = GPIOB->IDR;
        if(lastState != idr_b){
            delay_us(500);
            client_transmit(TO_BYTE_ARRAY(idr_b), sizeof(idr_b));
        }

        lastState = idr_b;
    }

    return 0;
}