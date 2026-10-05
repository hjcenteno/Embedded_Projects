/*
    Author: Henry Centeno
    Description:
        The purpose of this file is to test the implementation of the i2c_driver to correctly detect the mpu-6050 sensor
        The peripheral setup is handled by the i2c_driver for pb8/9.

    Components | pin | peripheral
    sda        | d15 | pb9
    scl        | d14 | pb8
*/

#include "i2c_driver/i2c_driver.h"
#include "uart_driver/lpuart_driver.h"
#include "system_time/mcu_time.h"

int main(void){
    init_lpuart();
    init_tim2();
    init_i2c(standard); //initiate it in standard mode

    uint8_t whoAmI_reg = 0x75;
    uint8_t mpu6050 = 0x68;
    uint8_t actual;

    while(1){
        if(i2c_master_read(mpu6050, whoAmI_reg, &actual, sizeof(actual)) == 0){
            client_transmit(&actual, sizeof(actual));
            delay_ms(500);
        }
    }
}
