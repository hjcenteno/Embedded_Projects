/*
    Author: Henry Centeno
    Description:
        LPUART client that will tell the computer whenever the button has been pressed

    component | pins | gpio:
    led         d4     pb5
    led         d5     pb4
    button      d6     pb10 
    led         d7     pa8
*/

#include "stm32g474xx.h"
#include "uart_driver/lpuart_driver.h"
#include <inttypes.h>

void delay(uint32_t time){
    while(time--){}
}

int main(void){
    //setup
    RCC->AHB2ENR |= RCC_AHB2ENR_GPIOAEN; //activate the timer
    RCC->AHB2ENR |= RCC_AHB2ENR_GPIOBEN;
    
    /*
        Because the button is wired as reading a signal from the 3.3V, then sending the signal to D10 through the resistor,
        the moder can be left as 00 when we set the bits.
    */
    //setting the moder for pb10, this will act as the button
    GPIOB->MODER &= ~GPIO_MODER_MODE10;
    GPIOB->PUPDR &= ~GPIO_MODER_MODE10;
    GPIOB->PUPDR |= GPIO_MODER_MODE10_1; //button reads pull down resistor to determine the change of button state

    //setting the moder for pa8, this will act as the led
    GPIOA->MODER &= ~GPIO_MODER_MODE8; //moder &= ~(0b11 << N), where n is the pin number
    GPIOA->MODER |= GPIO_MODER_MODE8_0; //moder |= (0b01 << N), where n is the pin numberm and 0b01 is for general purpose

    //d5 -> pb4, d4 -> pb5
    GPIOB->MODER &= ~(GPIO_MODER_MODE4 | GPIO_MODER_MODE5); //clear pb4 and pb5 together
    GPIOB->MODER |= (GPIO_MODER_MODE4_0 | GPIO_MODER_MODE5_0); //set pb4 and pb5 together to general io (0b10)

    init_lpuart();

    while(1){
        uint16_t button_state = (uint16_t)GPIOB->IDR;
        if(GPIOB->IDR & GPIO_IDR_IDR_10){ //button state is high
            GPIOA->BSRR = GPIO_BSRR_BS8; //set d7 to high
            delay(500000);
            GPIOB->BSRR = GPIO_BSRR_BS_4; //set d5 to high
            delay(500000);
            GPIOB->BSRR = GPIO_BSRR_BS_5; //set d4 to high
            client_transmit((uint8_t *)&button_state, sizeof(GPIOB->IDR));
        }

        else{ //state is low
            GPIOA->BSRR = GPIO_BSRR_BR8; //set d7 to low
            delay(500000);
            GPIOB->BSRR = GPIO_BSRR_BR4; //set d5 to low
            delay(500000);
            GPIOB->BSRR = GPIO_BSRR_BR5; //set d4 to low
            client_transmit((uint8_t *)&button_state, sizeof(GPIOB->IDR));
        }
    }
}