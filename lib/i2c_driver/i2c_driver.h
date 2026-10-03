/*
    Author: Henry Centeno
    Description:
        I2C implementation for my stm32 nucleo-g474re.
*/

#ifndef I2C_DRIVER_H
#define I2C_DRIVER_H

#include "common_includes/common_includes.h"

typedef enum i2c_mode{
    standard, //up to 100 khz
    fast, //up to 400 khz
    fastPlus //up to 1 Mhz
} i2c_mode;

//initiates the i2c handling the peripheral, rcc, control registers configs
int init_i2c(i2c_mode mode, bool interruptEN, uint32_t priority);
static inline void disable_i2c(void); //handles the i2c reset, called at the end
void reset_i2c(void);
/*
    uint8_t maddr: master address
    uint8_t saddr: slave address
    uint8_t regaddr: register address
    uint8_t data: data
*/

int i2c_master_read(uint8_t maddr, uint8_t saddr, uint8_t regaddr, uint8_t data);
int i2c_master_write(uint8_t maddr, uint8_t saddr, uint8_t regaddr, uint8_t data);

int i2c_slave_read(uint8_t maddr, uint8_t saddr, uint8_t regaddr, uint8_t data);
int i2c_slave_write(uint8_t maddr, uint8_t saddr, uint8_t regaddr, uint8_t data);

#endif