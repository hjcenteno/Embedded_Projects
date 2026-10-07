/*
    Author: Henry Centeno
    Description:
        Implementation of the mpu6050_driver header file
*/

#include "mpu6050_driver.h"

void zero_out_mpu6050(mpu6050_t *mpu6050){
    //this functions zeros out each value of the mpu6050, and it can then write it through i2c

    //set each value of the sensor to 0
    mpu6050->accX = 0;
    mpu6050->accY = 0;
    mpu6050->accZ = 0;
    mpu6050->gyroX = 0;
    mpu6050->gyroY = 0;
    mpu6050->gyroZ = 0;
    mpu6050->temp = 0;
}

void purify_read_lsb(mpu6050_t *mpu6050){
    //Data is read in as MSB, so to make it LSB we follow this algorithm: X = (X >> 4) | (X << 4)
    mpu6050->accX = (mpu6050->accX << 4) | (mpu6050->accX >> 4);
    mpu6050->accY = (mpu6050->accY << 4) | (mpu6050->accY >> 4);
    mpu6050->accZ = (mpu6050->accZ << 4) | (mpu6050->accZ >> 4);
    mpu6050->gyroX = (mpu6050->gyroX << 4) | (mpu6050->gyroX >> 4);
    mpu6050->gyroY = (mpu6050->gyroY << 4) | (mpu6050->gyroY >> 4);
    mpu6050->gyroZ = (mpu6050->gyroZ << 4) | (mpu6050->gyroZ >> 4);
    mpu6050->temp = (mpu6050->temp << 4) | (mpu6050->temp >> 4);
}