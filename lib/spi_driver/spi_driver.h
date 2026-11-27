/*
    Author: Henry Centeno
    Description:
        This will be a generic SPI driver that will act similar to the timx_utils driver.
*/

#ifndef SPI_DRIVER_H
#define SPI_DRIVER_H

#include "common_includes/common_includes.h"
#include "stm32g474xx.h"

//this defines how the user wants to define when to sample the bit for the CPOl and CPHA
typedef enum spi_config_t : uint8_t{
    CPOL_L_CPHA_L, //low clock idle state, sample on rising
    CPOL_L_CPHA_H, //low clock idle state, sample on falling
    CPOL_H_CPHA_L, //high clock idle state, sample on rising
    CPOL_H_CPHA_H //high clock idle state, sample on falling
} SPI_Config;

//the g474re BR is dependent on the system clock, and with the current timx_utils, the system clock is either 16 Mhz HSI or 24 Mhz HSE
typedef enum spi_br_t : uint8_t{
    SPI_BR_2, //@16Mhz HSI: 8 Mz, @24 Mhz HSE: 12 Mhz
    SPI_BR_4, //@16Mhz HSI: 4 Mz, @24 Mhz HSE: 6 Mz
    SPI_BR_8, //@16Mhz HSI: 2 Mhz, @24 Mhz HSE: 3 Mz
    SPI_BR_16, //@16Mhz HSI: 1 Mz, @24 Mhz HSE: 1.5 Mhz
    SPI_BR_32, //@16Mhz HSI: 0.5 Mhz, @24 Mhz HSE: 0.75 Mhz
    SPI_BR_64, //@16Mhz HSI: 0.24 Mhz, @24 Mhz HSE: 0.375 Mhz
    SPI_BR_128, //@16Mhz HSI: 0.12 Mhz, @24 Mhz HSE: .1875 Mhz
    SPI_BR_256 //@16Mhz HSI: 0.06 Mhz, @24 Mhz HSE: 0.09375 Mhz
}SPI_BR;

/*
    The user must enable SPI in the RCC, define the gpio pins, 
    but the function itself will handle the register stuff for 
    half and full duplex modes. I am only going to write the mcu for 
    master only as that's what I need at the moment.
    For simplicity, the spi can either be full duplex or simplex.
    Then rxonly determines whether for simplex to be either transmit-only or recieve-only
    the frame definition in cr2 and enabling the SPE bit in cr1 are left to the user
*/
int init_spi(SPI_TypeDef *spiX, const SPI_Config spi_timing, const SPI_BR br, const bool lsb, const bool rxonly);

/*
    For both the miso and mosi functions, the user is responsible for creating the 
    idle state for communicating, the dma functions will handle the dma stuff
*/
int spi_mosi_8bit(SPI_TypeDef *spiX, const uint8_t* data, const uint32_t dataLen);
int spi_mosi_16bit(SPI_TypeDef *spiX, const uint16_t* data, const uint32_t dataLen);
int spi_miso_8bit(SPI_TypeDef *spiX, const uint8_t* buffer, const uint32_t bufLen);
int spi_miso_16bit(SPI_TypeDef *spiX, const uint16_t* buffer, const uint32_t bufLen);

int spi_dma_mosi_8bit(SPI_TypeDef *spiX, const uint8_t* data, const uint32_t dataLen);
int spi_dma_mosi_16bit(SPI_TypeDef *spiX, const uint16_t* data, const uint32_t dataLen);
int spi_dma_miso_8bit(SPI_TypeDef *spiX, const uint8_t* buffer, const uint32_t bufLen);
int spi_dma_miso_16bit(SPI_TypeDef *spiX, const uint16_t* buffer, const uint32_t bufLen);


#endif