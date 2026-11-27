/*
    Author: Henry Centeno
    Description:
        Implementation of the spi driver
*/

#include "spi_driver.h"

int init_spi(SPI_TypeDef *spiX, const SPI_Config spi_timing, const SPI_BR br, const bool lsb, const bool rxonly){
    //following the procedure of configuring the SPI per page 1838 of the reference manual
    //error check args
    if(spiX == NULL){
       return 1; 
    }
    
    //create a clean slate for the spiX registers
    spiX->CR1 = 0;

    //mcu will always be master
    spiX->CR1 |= SPI_CR1_MSTR;

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
    
    //now write the cr2
    //going to configure for 16-bit data size
    spiX->CR2 |= (SPI_CR2_DS_3 | SPI_CR2_DS_2 | SPI_CR2_DS_1 | SPI_CR2_DS_0);

    //enable the spi
    spiX->CR1 |= SPI_CR1_SPE;

    return 0;
}