/*
    Author: Henry Centeno
    Description:
        I2C implementation for my stm32 nucleo-g474re. This code is written to use i2c1 on the nucleo g474re, I will at some point
        add support for i2c 2-4, but at the moment, implementing those is simply not needed.
*/

#include "i2c_driver.h"

static void set_timingr_settings(i2c_mode mode){
    //sets the values in the timing settings table found in pages 1657-1658 of the reference manual
    I2C1->TIMINGR &= ~(I2C_TIMINGR_SCLL | I2C_TIMINGR_SCLH | I2C_TIMINGR_SDADEL | I2C_TIMINGR_SCLDEL | I2C_TIMINGR_PRESC); //clear the register first
    switch(mode){
        //following table 360 for 16 MHz
        case fast:
            I2C1->TIMINGR = (
                (0x1UL << I2C_TIMINGR_PRESC_Pos) | 
                (0x9UL << I2C_TIMINGR_SCLL_Pos) |
                (0x3UL << I2C_TIMINGR_SCLH_Pos) |
                (0x2UL << I2C_TIMINGR_SDADEL_Pos) |
                (0x3UL << I2C_TIMINGR_SCLDEL_Pos)
            );
            break;
        
        case fastPlus:
            I2C1->TIMINGR = (
                (0x0UL << I2C_TIMINGR_PRESC_Pos) | 
                (0x4UL << I2C_TIMINGR_SCLL_Pos) |
                (0x2UL << I2C_TIMINGR_SCLH_Pos) |
                (0x0UL << I2C_TIMINGR_SDADEL_Pos) |
                (0x2UL << I2C_TIMINGR_SCLDEL_Pos)
            );
            break;
        default: //default is treated as standard mode
            I2C1->TIMINGR = (
                (0x3UL << I2C_TIMINGR_PRESC_Pos) | 
                (0x13UL << I2C_TIMINGR_SCLL_Pos) |
                (0xFUL << I2C_TIMINGR_SCLH_Pos) |
                (0x2UL << I2C_TIMINGR_SDADEL_Pos) |
                (0x4UL << I2C_TIMINGR_SCLDEL_Pos)
            );
        break;
    }
}

/*
Before enabling the I2C peripheral, configure and enable its clock through 
the RCC, and initialize its control registers.
The I2C peripheral can then be enabled by setting the PE bit of the 
I2C_CR1 register.
*/

static inline void disable_i2c(void){
    //ensure i2c peripheral is not enabled
    I2C1->CR1 &= ~I2C_CR1_PE;
}

void reset_i2c(void){
    //reset the i2c by dsiabling it then wait for it to turn back on
    I2C1->CR1 &= ~I2C_CR1_PE;
    while(I2C1->CR1 & I2C_CR1_PE){}
    I2C1->CR1 |= I2C_CR1_PE;
}

int init_i2c(i2c_mode mode, bool interruptEN, uint32_t priority){
    //returns 0 on a successful initiation, 1 on failure

    //enable the rcc peripheral clock for the i2c and gpiob
    //I will be using PB8 for i2c serial clock, and PB9 for serial data
    RCC->AHB2ENR |= RCC_AHB2ENR_GPIOBEN; //enable the clock
    RCC->APB1ENR1 |= RCC_APB1ENR1_I2C1EN;

    //select the i2c clock at ccipr to use the 16 MHz hsi clock
    RCC->CR |= RCC_CR_HSION;
    while((RCC->CR & RCC_CR_HSIRDY) == 0){} //wait until the hsi is ready

    RCC->CCIPR &= ~(RCC_CCIPR_I2C1SEL);
    RCC->CCIPR |= (RCC_CCIPR_I2C1SEL_1);

    //setup the pb8/9 pins moder to AF
    GPIOB->MODER &= ~(GPIO_MODER_MODE8 |GPIO_MODER_MODE9);
    GPIOB->MODER |= (GPIO_MODER_MODE8_1 |GPIO_MODER_MODE9_1);

    //set the pins open-drain
    //I am using the external pull up in my wiring to keep the sda/scl lines high
    /*
        since the most the i2c can operate is up to 1MHZ, no need to set the ospeedr as '00'
        can handle up to 10 MHz at 10 pF per the datasheet at page 131. 
    */
    GPIOB->OTYPER &= ~(GPIO_OTYPER_OT8| GPIO_OTYPER_OT9);
    GPIOB->OTYPER |= (GPIO_OTYPER_OT8| GPIO_OTYPER_OT9); //set to open drain
    
    //setup each pins AF to af4 (pb8: i2c-scl, pb9: i2c-sda)
    GPIOB->AFR[1] &= ~(GPIO_AFRH_AFSEL8 | GPIO_AFRH_AFSEL9);
    GPIOB->AFR[1] |= (GPIO_AFRH_AFSEL8_2 | GPIO_AFRH_AFSEL9_2);

    //setting up the i2c following the flow from the reference manual at page 1633
    disable_i2c(); //ensure the i2c is properly reset before configuring
    
    //at the moment, only the analog filter is being used, I will add a function to configure the digital noise filter if I need it
    I2C1->CR1 &= ~(I2C_CR1_ANFOFF | I2C_CR1_DNF);
    
    //keeping the nostretch bit off as the mcu will act as the controller
    set_timingr_settings(mode);

    if(interruptEN){
        //configure for interrupt, else it'll be fore polling
        I2C1->CR1 &= ~(I2C_CR1_RXIE | I2C_CR1_TXIE);
        I2C1->CR1 |= (I2C_CR1_RXIE | I2C_CR1_TXIE); //interrupt on read and write
        NVIC_SetPriority(I2C1_EV_IRQn, priority);
        NVIC_EnableIRQ(I2C1_EV_IRQn);
    }

    I2C1->CR1 |= I2C_CR1_PE;
    return 0;
}