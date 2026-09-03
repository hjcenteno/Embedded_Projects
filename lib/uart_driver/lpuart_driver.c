/*
    Author: Henry Centeno
    Description:
       Refer to the readme
                    
*/

// #include "stm32g474xx.h"
#include "lpuart_driver.h"

int init_lpuart(void){
    /*
        initiates the MCU (client) transmit to the laptop (server).
        ONLY call this function server side!!!!!
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
    
    //set up the lpuart by following the character transmission procedure laid out on page 1783 of the reference manual
    //program the M bits to define the word length, going to use (M = 00) for the 8 bit word length
    LPUART1->CR1 &= ~(USART_CR1_M0 | USART_CR1_M1 | USART_CR1_FIFOEN | USART_CR1_UE | USART_CR1_TE); //clear it first, also acts as the set since '00' is the 8-bit character length
    //utilize the fifo for transmission by setting the fifoen bit (bit 29)
    LPUART1->CR1 |= (USART_CR1_FIFOEN);
    //set up the buad rate register, going to use 9600 baud
    //from table 383, to get 9600 buad, the value programmed is 0x369 when using the LSE clock
    LPUART1->BRR = 0x369;
    //going to use 1 stop bit
    LPUART1->CR2 &= ~(USART_CR2_STOP_0 | USART_CR2_STOP_1); //since'00' is defined as 1 stop bit, this clear ensures that the LPUART will use the 1 stop bit
    //enable the lpuart by writing '1' to the us bit in cr1
    LPUART1->CR1 |= (USART_CR1_UE | USART_CR1_TE); //send the first transmission as well
    while((LPUART1->ISR & USART_ISR_TC) == 0){} //wait until tc is set to 1 to indicate the end of the transmission
    
    return 0;
}

/*
    From the reference manual at page 1783:
    When FIFO mode is enabled, writing a data in the LPUART_TDR adds one data to
    the TXFIFO. Write operations to the LPUART_TDR are performed when TXFNF flag
    is set. This flag remains set until the TXFIFO is full.
*/

void client_byte_transmit(uint8_t data){
    /* MCU transmit one byte */

    //early return if the lpuart is not enabled
    if((LPUART1->CR1 & USART_CR1_UE) == 0){
        return;
    }

    //wait to transmit until the txfifo flag is set
    /*
        0: Data register is full/Transmit FIFO is full.
        1: Data register/Transmit FIFO is not full.
    */
    while((LPUART1->ISR & USART_ISR_TXE_TXFNF) == 0){}

    //write data to the tdr
    LPUART1->TDR = data;

}

void client_transmit(uint8_t *data, const uint8_t length){
    /* 
        MCU transmits N (length) bytes. It is important that the server should know how to interpret the bytes
        as this function only sends raw bytes.
        This function first sends a start byte, a length byte, then the data, and finally a error checking byte.
        The error checking byte is simply an xor of all the bytes
    */

    //early return if the data is null or length is o
    if((data == NULL) || (length == 0)){
        return;
    }

    uint8_t checksum = start_byte;
    //let server know that we are going to transmit
    client_byte_transmit(start_byte);
    
    //let server know how many bytes to read
    client_byte_transmit(length);
    checksum ^= length;

    //send the data one byte at a time
    for(uint8_t i = 0; i < length; i++){
        client_byte_transmit(data[i]);
        checksum ^= data[i];
    }

    client_byte_transmit(checksum);

    //wait for the transmission to have been completed
    while((LPUART1->ISR & USART_ISR_TC) == 0){}
}