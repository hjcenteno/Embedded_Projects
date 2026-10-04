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
    //reset flags incase of the errors
    if(I2C1->ISR & (I2C_ISR_ARLO | I2C_ISR_BERR)){
        I2C1->ICR = (I2C_ICR_ARLOCF | I2C_ICR_BERRCF);
    }

    I2C1->CR1 &= ~I2C_CR1_PE;
    while(I2C1->CR1 & I2C_CR1_PE){}
    I2C1->CR1 |= I2C_CR1_PE;

}

int init_i2c(i2c_mode mode){
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
    I2C1->CR1 &= ~(I2C_CR1_ANFOFF | I2C_CR1_DNF | I2C_CR1_SBC);
    
    //keeping the nostretch bit off as the mcu will act as the controller
    set_timingr_settings(mode);

    //configuring cr2
    //configure for 7 bit addressing by leaving the add10 bit as 0
    I2C1->CR2 &= ~(I2C_CR2_ADD10 | I2C_CR2_AUTOEND | I2C_CR2_RELOAD); //create a clean slate 

    I2C1->CR1 |= I2C_CR1_PE;
    return 0;
}

static int i2c_transmit_addr(uint8_t addr){
    //writes the addr to the txdr of the i2c
    while((I2C1->ISR & (I2C_ISR_NACKF | I2C_ISR_TXIS)) == 0){}

    if(I2C1->ISR & I2C_ISR_NACKF){ //end when the nackf == 1
        return 1;
    }

    I2C1->TXDR = addr;
    
    return 0;
}

static int i2c_transmit_data(uint8_t *data, uint8_t length){
    //return 1 on nack, arlo, or berr 
    for(uint8_t i = 0; i < length; i++){
        while((I2C1->ISR & (I2C_ISR_NACKF | I2C_ISR_TXIS | I2C_ISR_ARLO | I2C_ISR_BERR)) == 0){} //wait until either the txis or nackf is set to 1

        if(I2C1->ISR & (I2C_ISR_NACKF | I2C_ISR_ARLO | I2C_ISR_BERR)){ //end when the nackf == 1 or an error occured
            return 1;
        }

        //write the data
        I2C1->TXDR = data[i];
    }
    
    return 0;
}

static int i2c_read_data(uint8_t *data, uint8_t length){
    //return 1 upon a bus error or arbitration loss
    for(uint8_t i = 0; i < length; i++){
        while((I2C1->ISR & (I2C_ISR_ARLO | I2C_ISR_BERR | I2C_ISR_RXNE)) == 0){} //wait until rxne == 1
    
        if(I2C1->ISR & (I2C_ISR_ARLO | I2C_ISR_BERR)){
            return 1;
        }

        data[i] = I2C1->RXDR;
    }

    return 0;
}

static void clean_i2c_cr2(uint8_t saddr, uint8_t nbytes, bool reading, bool autoend){
    //creating a clean slate for cr2
    I2C1->CR2 &= ~(
        I2C_CR2_SADD | I2C_CR2_RD_WRN | I2C_CR2_NBYTES |
        I2C_CR2_NACK | I2C_CR2_START | I2C_CR2_STOP |
        I2C_CR2_AUTOEND
    );

    //reading is set when the rd_wrn bit is set, otherwise 0 is writing
    if(reading){
        I2C1->CR2 |= I2C_CR2_RD_WRN;
    }

    if(autoend){
        I2C1->CR2 |= I2C_CR2_AUTOEND;
    }

    I2C1->CR2 |= ((saddr << 1) << I2C_CR2_SADD_Pos); //set the saddr leftshifted by 1 since sadd[0] is don't care for 7-bit addressing
    
    I2C1->CR2 |= (nbytes << I2C_CR2_NBYTES_Pos); //number of bytes to be transmitted
}

int i2c_master_write(uint8_t saddr, uint8_t regaddr, uint8_t *data, uint8_t dLength){
    int result = 0;
    
    if((I2C1->CR1 & I2C_CR1_PE) == 0){
        return 1; //return if the i2c is not configured
    }

    if(((data == NULL) || dLength == 0) || (dLength > 254)){
        return 1; //check the parameters, and return on invalid parameters
    }

    //only transmit if communication is not being processed
    while(I2C1->ISR & I2C_ISR_BUSY){}
    
    //creating a clean slate for cr2
    clean_i2c_cr2(saddr, (dLength + 1), false, true); //controller will automatically send a stop condition once nbytes are sent

    //begin transmission, following figure 551 for 12c controller transmitter for nbytes <= 255
    I2C1->CR2 |= I2C_CR2_START;

    //send the register address
    result = i2c_transmit_addr(regaddr);
    
    if(result == 0){
        result = i2c_transmit_data(data, dLength);
    }
    
    //wait until the transmission ends clear with the icr
    while((I2C1->ISR & I2C_ISR_STOPF) == 0){}
    I2C1->ICR = (I2C_ICR_STOPCF | I2C_ICR_NACKCF);

    return result;
}

int i2c_master_read(uint8_t saddr, uint8_t regaddr, uint8_t *data, uint8_t dLength){
    int result = 0;

    if((I2C1->CR1 & I2C_CR1_PE) == 0){
        return 1; //return if the i2c is not configured
    }

    if(((data == NULL) || dLength == 0)){
        return 1; //check the parameters, and return on invalid parameters
    }

    //only read if communication is not being processed
    while(I2C1->ISR & I2C_ISR_BUSY){}

    //following figure 554 of the reference manual for controller receiver, n <= 255
    //reading is split into first transmitting the register address, then reading
    //per table 357 row 2, reading needs autoend to be 0
    clean_i2c_cr2(saddr, 1, false, false);

    I2C1->CR2 |= I2C_CR2_START;
    
    result = i2c_transmit_addr(regaddr);
    if(result == 0){
        while((I2C1->ISR & (I2C_ISR_NACKF | I2C_ISR_TC)) == 0){} //wait until either the txis or nackf is set to 1
        if(I2C1->ISR & I2C_ISR_NACKF){ //end when the nackf == 1
            result = 1; //failed to send the register address
        }
    }

    //read from the register
    if(result == 0){
        clean_i2c_cr2(saddr, dLength, true, true);
        I2C1->CR2 |= I2C_CR2_START;
        result = i2c_read_data(data, dLength);
    }

    if(result == 1){
        reset_i2c();
        return 1;
    }

    //wait until the transmission ends clear with the icr
    while((I2C1->ISR & I2C_ISR_STOPF) == 0){}
    I2C1->ICR = (I2C_ICR_STOPCF | I2C_ICR_NACKCF);

    return result;
}