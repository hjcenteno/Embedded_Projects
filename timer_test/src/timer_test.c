/*
    Author: Henry Centeno
    Description: Test the mcu_time implementation
*/

#include "uart_driver/lpuart_driver.h"
#include "system_time/mcu_time.h"

void init_ld2(void){
    RCC->AHB2ENR &= ~RCC_AHB2ENR_GPIOAEN;
    RCC->AHB2ENR |= RCC_AHB2ENR_GPIOAEN;

    GPIOA->MODER &= ~GPIO_MODER_MODER5;
    GPIOA->MODER |= GPIO_MODER_MODE5_0;
    
}

int main(void){
    init_tim2();
    init_lpuart();

    init_ld2(); //initiates the built in ld2 on the board
    char *onMsg = "ld2 is on.";
    char *offMsg = "ld2 is off.";

    while(1){
        //turn the ld2 on for 3 seconds
        GPIOA->BSRR = GPIO_BSRR_BS5;
        client_transmit((uint8_t *)onMsg, 12); //transmit the raw string
        delay_s(3);
        //turn the ld2 off for 3 seconds
        GPIOA->BSRR = GPIO_BSRR_BR5;
        delay_s(3);
        client_transmit((uint8_t *)offMsg, 13); //transmit the raw string
    }

    return 0;
}