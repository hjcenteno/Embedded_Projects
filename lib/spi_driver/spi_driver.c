/*
    Author: Henry Centeno
    Description:
        Implementation of the spi driver
*/

#include "spi_driver.h"
#include "stm32g474xx.h"

int init_spi(SPI_TypeDef *spiX, const SPI_Config spi_timing, const SPI_BR br, const bool lsb, const bool rxonly){
    //following the procedure of configuring the SPI per page 1838 of the reference manual
    //error check args
    if(spiX == NULL){
       return 1; 
    }
    
    //create a clean slate for the spiX registers
    spiX->CR1 = 0;

    //mcu will always be master
    spiX->CR1 |= (SPI_CR1_MSTR | SPI_CR1_SSI | SPI_CR1_SSM);

    //configure the serial clock baud rate
    switch(br){
        case SPI_BR_2: //BR[0:2] 000
            break;
        case SPI_BR_4: //BR[0:2] 001
            spiX->CR1 |= SET_PIN(0b001, SPI_CR1_BR_Pos);
            break;
        case SPI_BR_8: //BR[0:2] 010
            spiX->CR1 |= SET_PIN(0b010, SPI_CR1_BR_Pos);
            break;
        case SPI_BR_16: //BR[0:2] 011
            spiX->CR1 |= SET_PIN(0b011, SPI_CR1_BR_Pos);
            break;
        case SPI_BR_32: //BR[0:2] 100
            spiX->CR1 |= SET_PIN(0b100, SPI_CR1_BR_Pos);
            break;
        case SPI_BR_64: //BR[0:2] 101
            spiX->CR1 |= SET_PIN(0b101, SPI_CR1_BR_Pos);
            break;
        case SPI_BR_128: //BR[0:2] 110
            spiX->CR1 |= SET_PIN(0b110, SPI_CR1_BR_Pos);
            break;
        case SPI_BR_256: //BR[0:2] 111
            spiX->CR1 |= SET_PIN(0b111, SPI_CR1_BR_Pos);
            break;
        default: //the default BR will be at half (f_Pclk/2)
            break;
    }

    //configure the cpol and cpha bits per the spi_timing
    switch(spi_timing){
        case CPOL_L_CPHA_L:
            break;
        case CPOL_L_CPHA_H:
            spiX->CR1 |= SPI_CR1_CPHA;
            break;
        case CPOL_H_CPHA_L:
            spiX->CR1 |= SPI_CR1_CPOL;
            break;
        case CPOL_H_CPHA_H:
            spiX->CR1 |= (SPI_CR1_CPOL | SPI_CR1_CPHA);
            break;
        default:
            return 1; //the spi config was something else
    }

    //spi is assumed to be msb
    if(lsb){
        spiX->CR1 |= SPI_CR1_LSBFIRST;
    }

    if(rxonly){ //rxonly and bidimmode cannot be set at the same time
        spiX->CR1 |= SPI_CR1_RXONLY;
    }

    return 0;
}

int spi_mosi_8bit(SPI_TypeDef *spiX, const uint8_t* data, const uint32_t dataLen){
    //following the procedure for transmission laid out in page 1839 of the RM
    //wait until the spi is not busy
    volatile uint16_t MAX_TMEOUT = 0xFFFF; //maximum cpu clock cycles to wait for the spi bus to not be busy
    while(spiX->SR & SPI_SR_BSY){
        if((--MAX_TMEOUT) == 0){
            return 1; //busy timeout
        }
    }

    //for each byte of the data, send it if its not consider full defined as 11 for the ftvlv
    for(uint32_t i = 0; i < dataLen; i++){
        MAX_TMEOUT = 0xFFFF; //reset the timeout for each data point
        //only write if the txfifo is half empty
        while((spiX->SR & SPI_SR_FTLVL) == SPI_SR_FTLVL){
            if((--MAX_TMEOUT) == 0){
                return 1; //busy timeout
            }
        }

        //since the DR is a uint32_t register, to avoid the data being zero-filled, treat the register as a uint8t
        *(volatile uint8_t *)&spiX->DR = data[i];
    }

    //wait until the txfifo is empty
    MAX_TMEOUT = 0xFFFF; //reset the timeout
    while((spiX->SR & SPI_SR_TXE) == 0){
        if((--MAX_TMEOUT) == 0){
            return 1; //txne timeout
        }
    }

    return 0;
}

int spi_mosi_16bit(SPI_TypeDef *spiX, const uint16_t* data, const uint32_t dataLen){
volatile uint16_t MAX_TMEOUT = 0xFFFF; //maximum cpu clock cycles to wait for the spi bus to not be busy
    while(spiX->SR & SPI_SR_BSY){
        if((--MAX_TMEOUT) == 0){
            return 1; //busy timeout
        }
    }

    //for each byte of the data, send it if its not consider full defined as 11 for the ftvlv
    for(uint32_t i = 0; i < dataLen; i++){
        //only write if the txfifo is half empty
        MAX_TMEOUT = 0xFFFF; //reset the timeout for each data point
        while((spiX->SR & SPI_SR_FTLVL) == SPI_SR_FTLVL){
            if((--MAX_TMEOUT) == 0){
                return 1; //busy timeout
            }
        }

        //since the DR is a uint32_t register, to avoid the data being zero-filled, treat the register as a uint8t
        *(volatile uint16_t *)&spiX->DR = data[i];
    }

    //wait until the txfifo is empty
    MAX_TMEOUT = 0xFFFF; //reset the timeout
    while((spiX->SR & SPI_SR_TXE) == 0){
        if((--MAX_TMEOUT) == 0){
            return 1; //busy timeout
        }
    }

    return 0;
}
// int spi_miso_8bit(SPI_TypeDef *spiX, const uint8_t* buffer, const uint32_t bufLen);
// int spi_miso_16bit(SPI_TypeDef *spiX, const uint16_t* buffer, const uint32_t bufLen);

// int spi_dma_mosi_8bit(SPI_TypeDef *spiX, const uint8_t* data, const uint32_t dataLen);
// int spi_dma_mosi_16bit(SPI_TypeDef *spiX, const uint16_t* data, const uint32_t dataLen);
// int spi_dma_miso_8bit(SPI_TypeDef *spiX, const uint8_t* buffer, const uint32_t bufLen);
// int spi_dma_miso_16bit(SPI_TypeDef *spiX, const uint16_t* buffer, const uint32_t bufLen);
