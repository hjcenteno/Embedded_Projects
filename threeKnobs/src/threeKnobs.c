/*
    Author: Henry Centeno
    Description:
        Configure three rotary encoders and be able to read their inputs.

    knob 1:
    Components | pin | peripheral
        s1     | CN10.1 | pc9
        s2     | CN10.2 | pc8
        key    | CN10.4 | pc6

    knob 2:
    Components | pin | peripheral
        s1     | CN10.12 | pa12
        s2     | CN10.14 | pa11
        key    | CN10.21 | pa9

    knob 3:
    Components | pin | peripheral
        s1     | CN7.2 | pc11
        s2     | CN7.1 | pc10
        key    | CN7.3 | pc12
*/

#include "uart_driver/lpuart_driver.h"
#include "system_time/timx_utils.h"
#include "devices/knobs.h"

void init_GPIOPins(void){
    //activate the peripheral in the rcc
    RCC->AHB2ENR |= (RCC_AHB2ENR_GPIOAEN | RCC_AHB2ENR_GPIOCEN);

    //I am using the ky040 ec11 module, and the module is built with pull-up resistors
    //I will initially set the gpio pins to PU to avoid noise and uneccessary reads
    //first clear the moder registers and leave it as 00 for input
    GPIOA->MODER &= ~(GPIO_MODER_MODE9 | GPIO_MODER_MODE11 | GPIO_MODER_MODE12);
    GPIOC->MODER &= ~(
        GPIO_MODER_MODE6 | GPIO_MODER_MODE8 | GPIO_MODER_MODE9 |
        GPIO_MODER_MODE10 | GPIO_MODER_MODE11 | GPIO_MODER_MODE12 
    );
    //clear the pupdr
    GPIOA->PUPDR &= ~(GPIO_PUPDR_PUPD9 | GPIO_PUPDR_PUPD11 | GPIO_PUPDR_PUPD12);
    GPIOC->PUPDR &= ~(
        GPIO_PUPDR_PUPD6 | GPIO_PUPDR_PUPD8 | GPIO_PUPDR_PUPD9 |
        GPIO_PUPDR_PUPD10 | GPIO_PUPDR_PUPD11 | GPIO_PUPDR_PUPD12 
    );

    //set the pupdr to pu
    GPIOA->PUPDR |= (GPIO_PUPDR_PUPD9_0 | GPIO_PUPDR_PUPD11_0 | GPIO_PUPDR_PUPD12_0);
    GPIOC->PUPDR |= (
        GPIO_PUPDR_PUPD6_0 | GPIO_PUPDR_PUPD8_0 | GPIO_PUPDR_PUPD9_0 |
        GPIO_PUPDR_PUPD10_0 | GPIO_PUPDR_PUPD11_0 | GPIO_PUPDR_PUPD12_0 
    );
}

int main(void){
    //initializations
    RCC->APB1ENR1 |= RCC_APB1ENR1_TIM6EN; //planning to use tim6 as a basic timer
    init_tim(TIM6);

    init_lpuart();
    init_GPIOPins();

    //set up the knob machines
    KnobMachine km1 = {
        .pinMasks = 0,
        .prevS1 = 0,
        .q = idle
    };

    KnobMachine km2 = {
        .pinMasks = 0,
        .prevS1 = 0,
        .q = idle
    };

    KnobMachine km3 = {
        .pinMasks = 0,
        .prevS1 = 0,
        .q = idle
    };

    //km1 sits on gpioc
    km1.pinMasks |= (
        SET_PIN(9, S1_POS) | 
        SET_PIN(8, S2_POS) | 
        SET_PIN(6, KEY_POS) 
    );

    //km1 sits on gpioc
    km1.pinMasks |= (
        SET_PIN(9, S1_POS) | 
        SET_PIN(8, S2_POS) | 
        SET_PIN(6, KEY_POS) 
    );

    //km1 sits on gpioc
    km1.pinMasks |= (
        SET_PIN(9, S1_POS) | 
        SET_PIN(8, S2_POS) | 
        SET_PIN(6, KEY_POS) 
    );
    return 0;
}