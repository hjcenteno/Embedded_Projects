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
    //Data is read in as MSB, so to make it LSB we follow this algorithm: X = (X >> 8) | (X << 8)
    mpu6050->accX = (int16_t)((uint16_t)mpu6050->accX << 8) | ((uint16_t)mpu6050->accX >> 8);
    mpu6050->accY = (int16_t)((uint16_t)mpu6050->accY << 8) | ((uint16_t)mpu6050->accY >> 8);
    mpu6050->accZ = (int16_t)((uint16_t)mpu6050->accZ << 8) | ((uint16_t)mpu6050->accZ >> 8);
    mpu6050->gyroX = (int16_t)((uint16_t)mpu6050->gyroX << 8) | (uint16_t)(mpu6050->gyroX >> 8);
    mpu6050->gyroY = (int16_t)((uint16_t)mpu6050->gyroY << 8) | (uint16_t)(mpu6050->gyroY >> 8);
    mpu6050->gyroZ = (int16_t)((uint16_t)mpu6050->gyroZ << 8) | (uint16_t)(mpu6050->gyroZ >> 8);
    mpu6050->temp = (int16_t)((uint16_t)mpu6050->temp << 8) | ((uint16_t)mpu6050->temp >> 8);
}