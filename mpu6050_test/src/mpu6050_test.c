/*
    Author: Henry Centeno
    Description:
        This project is to test/calibrate the mpu6050 sensor for the x,y, and z axises
        of the accelerometer and the gyrometer present in the sensor

    components | pin | peripheral | meaning
        led 1 | d7 | pa8 | whoAmI_error
        led 2 | d8 | pa9 | wakeUp_error

*/

#include "sensors/mpu6050_driver.h"
#include "uart_driver/lpuart_driver.h"
#include "i2c_driver/i2c_driver.h"
#include "system_time/mcu_time.h"

mpu6050_t mainSensor;

void init_leds(void){
    /*  Setups the leds to show errors
            led 1 | whoAmI_error | MCU failed to read the correct whoAmI value
            led 2 | wakeUp_error | MCU failed to wake up the sensor
    */

    //enable the peripehral clock
    RCC->AHB2ENR |= RCC_AHB2ENR_GPIOAEN;

    //set the led modes to digital output '01'
    GPIOA->MODER &= ~(GPIO_MODER_MODE8 | GPIO_MODER_MODE9); //whoAmI_error
    GPIOA->MODER |= (GPIO_MODER_MODE8_0 | GPIO_MODER_MODE9_0); //wakeUp_error
}

int main(void){
    //init some things
    init_lpuart();
    init_tim2();
    zero_out_mpu6050(&mainSensor); //set each axis and the temp to 0
    init_i2c(standard);
    init_leds();
    
    uint8_t whoThis;
    bool readSensorEn = true;
    
    //transmit to confirm transmission/read of the mpu6050 sensor
    if(i2c_master_read(mpu6050_saddr_0, MPU6050_WHO_AM_I_REG, &whoThis, sizeof(whoThis)) == 0){
        client_transmit(&whoThis, sizeof(whoThis)); //transmit regardless
    }

    client_transmit((uint8_t *)&mainSensor, sizeof(mainSensor));

    if(whoThis != mpu6050_saddr_0){
        //turn on the whoAmI_error led
        readSensorEn = false;
        GPIOA->BSRR = GPIO_BSRR_BS8;
    }

    //wake up the sensor from low power mode
    uint8_t pwr_mgmt_mask = 0 & (~MPU6050_SLEEP_BIT); //initializing the mask to set the sensor not in sleep mode
    if(i2c_master_write(mpu6050_saddr_0, MPU6050_PWR_MGMT_1, &pwr_mgmt_mask, sizeof(pwr_mgmt_mask)) != 0){
        //turn on the wakeUp_error led upon failure
        readSensorEn = false;
        GPIOA->BSRR = GPIO_BSRR_BS9;
    }

    //read the raw data first
    while(readSensorEn){ //NOLINT(bugprone-infinite-loop): tell clang this is intentional
        accelf_t accTx;
        //read the registers from the ACCX_OUT_REG_H (0x38) to GYROZ_OUT_REG_L (0x48)
        if(i2c_master_read(mpu6050_saddr_0, MPU6050_DATA_START_ADDR, TO_BYTE_ARRAY(mainSensor), sizeof(mainSensor)) == 0){
            purify_read_lsb(&mainSensor);
            client_transmit(TO_BYTE_ARRAY(mainSensor), sizeof(mainSensor));
            accTx.x = (float)mainSensor.accX / MPU6050_ACC_SENSITIVITY_0;
            accTx.y = (float)mainSensor.accY / MPU6050_ACC_SENSITIVITY_0;
            accTx.z = (float)mainSensor.accZ / MPU6050_ACC_SENSITIVITY_0;
            client_transmit(TO_BYTE_ARRAY(accTx), sizeof(accTx));
            delay_s(1);
        }
    }

    return 0;
}