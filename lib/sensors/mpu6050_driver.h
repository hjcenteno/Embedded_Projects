/*
    Author: Henry Centeno
    Description:
        Handles the implementation of the code responsible for reading from the mpu6050 and writing to it.
        This will not initiate the i2c_driver, but will assume it has been initiated. The reading and writing will
        not actually read/write, but will set up the data to be read/written into.
*/

#ifndef MPU6050_DRIVER_H
#define MPU6050_DRIVER_H

#include "common_includes/common_includes.h"
#include <math.h>

#define WHO_AM_I_REG 0x75u //the register for the who am i
#define mpu6050_saddr_0 0x68u //default slave address of the mpu6050
#define mpu6050_saddr_1 0x69u //the slave address of the mpu6050 if the ado pin is set high
#define DEGREES_TO_RADIAN(x) (x * M_PI) / 180 //avoid the mcu from doing the calculations
#define RADIAN_TO_DEGREES(x) (x * 180) / M_PI
#define CALIBRATION_CONST 0 //calibration const to normalize the data

//send this as the data for the i2c
typedef struct mpu6050_t{
        uint16_t accX;
        uint16_t accY;
        uint16_t accZ;
        uint16_t gyroX;
        uint16_t gyroY;
        uint16_t gyroZ;
        uint16_t temp;
}mpu6050_t;

//functions to do stuff to the mpu6050 sensor

int purify_read_lsb(mpu6050_t *mpu6050); //call to push the data read to make it into least significant byte format
int zero_out_mpu6050(mpu6050_t *mpu6050);

float calculate_roll(mpu6050_t *mpu6050);
float calculate_pitch(mpu6050_t *mpu6050);
float calculate_yaw(mpu6050_t *mpu6050);

#endif