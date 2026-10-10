/*
    Author: Henry Centeno
    Description:
        The purpose of this program is to test the accuracy of the complimentary filter
*/

#include "sensors/mpu6050_driver.h"
#include "uart_driver/lpuart_driver.h"
#include "i2c_driver/i2c_driver.h"
#include "system_time/mcu_time.h"

typedef struct orientation{
    float roll;
    float pitch;
}orientation;

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

bool init_mpu6050(const mpu6050_ptr *sensor){
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
    //initializaions
    init_tim2();
    init_lpuart();
    init_i2c(fast); //operate at 400hz
    init_leds();
    
    mpu6050_ptr mainSensor;
    raw_mpu6050_t mainSensor_raw; //store the raw readings
    cal_mpu6050_t mainSensor_cal; //store the calibrated readings
    mainSensor.raw_ptr = &mainSensor_raw;
    mainSensor.cal_ptr = &mainSensor_cal;
    zero_out_mpu6050(&mainSensor);

    //first wake up the sensor
    //read its data
    bool readSensorEn = false;
    uint32_t startTime = getTime();
    orientation angles; //keep track of the angles
    angles.roll = 0;
    angles.pitch = 0;
    
    while(1){
        if(!readSensorEn){ //on start/restart
            readSensorEn = init_mpu6050(&mainSensor);
            delay_ms(500);
            startTime = getTime(); //update after initializing the sensor
        }
        
        //read the registers from the ACCX_OUT_REG_H (0x3b) to GYROZ_OUT_REG_L (0x48)
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
        
        //calibrate the accelerometer and gyrometer data
        mainSensor_cal.accX = (float)mainSensor.raw_ptr->accX / MPU6050_ACC_SENSITIVITY_0;
        mainSensor_cal.accY = (float)mainSensor.raw_ptr->accY / MPU6050_ACC_SENSITIVITY_0;
        mainSensor_cal.accZ = (float)mainSensor.raw_ptr->accZ / MPU6050_ACC_SENSITIVITY_0;
        mainSensor_cal.gyroX = (float)mainSensor.raw_ptr->gyroX / MPU6050_GYRO_SENSITIVITY_0;
        mainSensor_cal.gyroY = (float)mainSensor.raw_ptr->gyroY / MPU6050_GYRO_SENSITIVITY_0;
        mainSensor_cal.gyroZ = (float)mainSensor.raw_ptr->gyroZ / MPU6050_GYRO_SENSITIVITY_0;
        applyFullCalibrations(mainSensor.cal_ptr);
        uint32_t currentTime = getTime();

        client_transmit(TO_BYTE_ARRAY(mainSensor_cal), sizeof(mainSensor_cal));
        // delay_ms(300);

        //get the roll and pitch angles in radians
        angles.roll = calculate_roll(mainSensor.cal_ptr, angles.roll, ((currentTime - startTime) * MICRO_TO_BASE)); //getTime gets the time in us
        angles.pitch = calculate_pitch(mainSensor.cal_ptr, angles.pitch, ((currentTime - startTime) * MICRO_TO_BASE)); 
        client_transmit(TO_BYTE_ARRAY(angles), sizeof(angles));
        startTime = currentTime;
    }
    
    return 0;
}