/*
    Author: Henry Centeno
    Description:
        This project is to test/calibrate the mpu6050 sensor for the x,y, and z axises
        of the accelerometer and the gyrometer present in the sensor

    components | pin | peripheral | meaning
        led 1 | d7 | pa8 | reception_err
        led 2 | d8 | pa9 | transmission_err

*/

#include "sensors/mpu6050_driver.h"
#include "uart_driver/lpuart_driver.h"
#include "i2c_driver/i2c_driver.h"
#include "system_time/mcu_time.h"

void init_leds(void){
    /*  Setups the leds to show errors
            led 1 | reception_err | MCU failed to read 
            led 2 | transmission_err | MCU failed to write
    */

    //enable the peripehral clock
    RCC->AHB2ENR |= RCC_AHB2ENR_GPIOAEN;

    //set the led modes to digital output '01'
    GPIOA->MODER &= ~(GPIO_MODER_MODE8 | GPIO_MODER_MODE9); //reception_err
    GPIOA->MODER |= (GPIO_MODER_MODE8_0 | GPIO_MODER_MODE9_0); //transmission_err
}

bool init_mpu6050(mpu6050_ptr *sensor){
    //procedure to start the sensor up
    uint8_t whoThis = 0;
    
    //transmit to confirm transmission/read of the mpu6050 sensor
    if(i2c_master_read(mpu6050_saddr_0, MPU6050_WHO_AM_I_REG, &whoThis, sizeof(whoThis)) == 0){
        client_transmit(&whoThis, sizeof(whoThis)); //transmit regardless
        client_transmit(PTR_TO_BYTES(sensor->raw_ptr), sizeof(*sensor->raw_ptr));
        client_transmit(PTR_TO_BYTES(sensor->cal_ptr), sizeof(*sensor->cal_ptr));
    }

    if(whoThis != mpu6050_saddr_0){
        //turn on the reception_err led
        GPIOA->BSRR = GPIO_BSRR_BS8;
        return false;
    }

    //wake up the sensor from low power mode
    uint8_t pwr_mgmt_mask = 0 & (~MPU6050_SLEEP_BIT); //initializing the mask to set the sensor not in sleep mode
    if(i2c_master_write(mpu6050_saddr_0, MPU6050_PWR_MGMT_1, &pwr_mgmt_mask, sizeof(pwr_mgmt_mask)) != 0){
        //turn on the transmission_err led upon failure
        GPIOA->BSRR = GPIO_BSRR_BS9;
        return false;
    }
    delay_ms(100);
    
    return true; 
}

int main(void){
    //init some things
    mpu6050_ptr mainSensor;
    raw_mpu6050_t mainSensor_raw;
    cal_mpu6050_t mainSensor_cal;
    mainSensor.raw_ptr = &mainSensor_raw;
    mainSensor.cal_ptr = &mainSensor_cal;

    init_lpuart();
    init_tim2();
    zero_out_mpu6050(&mainSensor); //set each axis and the temp to 0
    init_i2c(standard);
    init_leds();
    
    //first wake up the sensor
    //read its data
    bool readSensorEn = false;
    while(1){
        if(!readSensorEn){ //on start/restart
            readSensorEn = init_mpu6050(&mainSensor);
            delay_ms(500);
        }

        //read the registers from the ACCX_OUT_REG_H (0x3b) to GYROZ_OUT_REG_L (0x48)
        accelf_t calibratedAccAxis;
        gyrof_t calibratedGyroAxis;
        if(i2c_master_read(mpu6050_saddr_0, MPU6050_DATA_START_ADDR, PTR_TO_BYTES(mainSensor.raw_ptr), sizeof(*mainSensor.raw_ptr)) != 0){
            //turn on the reception_err led
            GPIOA->BSRR = GPIO_BSRR_BS8;
            recover_i2c(); //take back control of the sda/scl lines
            readSensorEn = false; //wake up the sensor agains
            continue;
        }
        else{
            GPIOA->BSRR = (GPIO_BSRR_BR8 | GPIO_BSRR_BR9); //keep the led off if reception is good
        }

        purify_read_lsb(mainSensor.raw_ptr); //from MSB to LSB
        
        //calibrate the accelerometer data
        calibratedAccAxis.x = (float)mainSensor.raw_ptr->accX / MPU6050_ACC_SENSITIVITY_0;
        calibratedAccAxis.y = (float)mainSensor.raw_ptr->accY / MPU6050_ACC_SENSITIVITY_0;
        calibratedAccAxis.z = (float)mainSensor.raw_ptr->accZ / MPU6050_ACC_SENSITIVITY_0;
        applyAccCalibration(&calibratedAccAxis);
        mainSensor_cal.accX = calibratedAccAxis.x;
        mainSensor_cal.accY = calibratedAccAxis.y;
        mainSensor_cal.accZ = calibratedAccAxis.z;

        //calibrate the gyrometer data
        calibratedGyroAxis.x = (float)mainSensor.raw_ptr->gyroX / MPU6050_GYRO_SENSITIVITY_0;
        calibratedGyroAxis.y = (float)mainSensor.raw_ptr->gyroY / MPU6050_GYRO_SENSITIVITY_0;
        calibratedGyroAxis.z = (float)mainSensor.raw_ptr->gyroZ / MPU6050_GYRO_SENSITIVITY_0;
        applyGyroCalibration(&calibratedGyroAxis);
        mainSensor_cal.gyroX = calibratedGyroAxis.x;
        mainSensor_cal.gyroY = calibratedGyroAxis.y;
        mainSensor_cal.gyroZ = calibratedGyroAxis.z;

        client_transmit(TO_BYTE_ARRAY(mainSensor_cal), sizeof(mainSensor_cal));
        delay_ms(500);
    }

    return 0;
}