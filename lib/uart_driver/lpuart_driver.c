/*
    Author: Henry Centeno
    Description:
       Refer to the readme
                    
*/

#include "stm32g474xx.h"
#include "lpuart_driver.h"
#include <inttypes.h>
#include "stdio.h"


void client_log(){
}

int server_init_lpuart(){
    /*
        initiates the MCU (client) transmit to the laptop (server).
        return 0 on success
    */

    //configure pa2 to use the alternate function (10) for the lpuart
    RCC->AHB2ENR |= RCC_AHB2ENR_GPIOAEN; //set the timer for gpioa
    
    GPIOA->MODER &= ~(GPIO_MODER_MODE2); //pa2 acts as lpuart tx; pa6 acts as the lpuart cts
    GPIOA->MODER |= GPIO_MODER_MODER2_1; //sets to moder to 10
    
    //from af12, pa2 is tx for lpuart. so configure the af registers to the corresponding pins
    GPIOA->AFR[0] &= ~(GPIO_AFRL_AFRL2); //clear the bits
    GPIOA->AFR[0] |= (GPIO_AFRL_AFSEL2_3 | GPIO_AFRL_AFSEL2_2); //af12 maps to pin 2 of gpioa
    
    //enable the pwr interface for backup domain access
    //refer to page 231 of the reference manual 0440 rev 9 for the stm32 g4 series
    RCC->APB1ENR1 |= RCC_APB1ENR1_PWREN;
    PWR->CR1 |= PWR_CR1_DBP;
    RCC->APB1ENR2 |= RCC_APB1ENR2_LPUART1EN;
    /*LSCOSEL: Low speed clock output selection
               Set and cleared by software.
               0: LSI clock selected
               1: LSE clock selected */
    RCC->BDCR &= ~(RCC_BDCR_LSEON);
    RCC->BDCR |= (RCC_BDCR_LSEON); //enable the lse
    //wait for the ready flag from the mcu
    while((RCC->BDCR & RCC_BDCR_LSERDY) == 0){} //loop for as long as the rdy flag is not set by the hardware

    //once lse is ready, configure the independent clock configuration to allow the baud for lpuart to work in low power mode
    /*LPUART1SEL[1:0]: LPUART1 clock source selection
        These bits are set and cleared by software to select the LPUART1 clock source.
        00: PCLK selected as LPUART1 clock
        01: System clock (SYSCLK) selected as LPUART1 clock
        10: HSI16 clock selected as LPUART1 clock
        11: LSE clock selected as LPUART1 clock*/
    RCC->CCIPR &= ~(RCC_CCIPR_LPUART1SEL);
    RCC->CCIPR |= (RCC_CCIPR_LPUART1SEL_1 | RCC_CCIPR_LPUART1SEL_0); //setting 11 to use the LSE
    //should I enable the rtc clock to use LSE
    
    //enable the lpuart peripheral clock

    return 0;
}