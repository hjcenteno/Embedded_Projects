/*
    Author: Henry Centeno
    Description:
        The user presses a button and the corresponding led is lit up.
        Then through the lpuart, the computer is told which led is lit up by which button.

    Components: [Component | Pin | Peripheral]:
    *button 1 | d0  | pc5 [pa3 is reserved for the lpuart_rx]
    *button 2 | d1  | pc4 [pa3 is reserved for the lpuart_tx]
    *button 3 | d2  | pa10
    *LED 1    | d4  | pb5
    *LED 2    | d5  | pb4
    *LED 3    | d6  | pb10
*/

#include "uart_driver/lpuart_driver.h"

void components_setup(void){
    /*set up the peripherals and gpio pins for the components*/\
    //activate the timers for gpioa, gpiob, gpioc
    RCC->AHB2ENR |= (RCC_AHB2ENR_GPIOAEN | RCC_AHB2ENR_GPIOBEN | RCC_AHB2ENR_GPIOCEN);

    //configure the moders for the pins
    GPIOA->MODER &= ~(GPIO_MODER_MODE10); //for pa10
    GPIOB->MODER &= ~(GPIO_MODER_MODE4 | GPIO_MODER_MODE5 | GPIO_MODER_MODE10); //for pb4, 5, 10
    GPIOC->MODER &= ~(GPIO_MODER_MODE4 | GPIO_MODER_MODE5); //for pc4, 5

    //configure the buttons as a pulldown
    //the moder can be left as 00, but the pupdr is sent to '10'
    //clear the bits to ensure we set the ones we want
    //set to '10' then repeat for the other buttons
    GPIOA->PUPDR &= ~(GPIO_PUPDR_PUPD10); //pa10
    GPIOA->PUPDR |= (GPIO_PUPDR_PUPD10_1);
    GPIOC->PUPDR &= ~(GPIO_PUPDR_PUPD4 | GPIO_PUPDR_PUPD5); //pc4, 5
    GPIOC->PUPDR |= (GPIO_PUPDR_PUPD4_1 | GPIO_PUPDR_PUPD5_1);

    //configure the leds pb4, 5, 10
    //using moder '01' for gp output
    GPIOB->MODER |= (GPIO_MODER_MODE4_0 | GPIO_MODER_MODE5_0 | GPIO_MODER_MODE10_0);
}

int main(void){
    components_setup(); //set up the components on the mcu

    while(1){
        //get the state of each button pc4, pc5, pa10
        uint32_t button_state = (GPIO_IDR_IDR_4 | GPIO_IDR_IDR_5 | GPIO_IDR_IDR_10); //idr register is a uint32, so I am using a uint32 for the button state to match the idr

        //whenever the button is high (1), turn the corresponding led
        if(GPIOC->IDR & GPIO_IDR_IDR_5){ //button 1 is pressed
            GPIOB->BSRR = GPIO_BSRR_BS_5; //turn on led 1
        }else{
            GPIOB->BSRR = GPIO_BSRR_BR_5; //turn off led 1
        }

        if(GPIOC->IDR & GPIO_IDR_IDR_4){ //button 2 is pressed
            GPIOB->BSRR = GPIO_BSRR_BS_4; //turn on led 2
        }else{
            GPIOB->BSRR = GPIO_BSRR_BR_4; //turn off led 2
        }

        if(GPIOA->IDR & GPIO_IDR_IDR_10){ //button 3 is pressed
            GPIOB->BSRR = GPIO_BSRR_BS_10; //turn on led 3
        }else{
            GPIOB->BSRR = GPIO_BSRR_BR_10; //turn off led 3
        }
    }

    return 0;
}
