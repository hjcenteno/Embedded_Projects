/*
    Author: Henry Centeno
    Description:
        Implementation of the mpu6050_driver header file
*/

#include "mpu6050_driver.h"

int zero_out_mpu6050(mpu6050_ptr *mpu6050){
    //this functions zeros out each value of the mpu6050, and it can then write it through i2c
    if(mpu6050 == NULL){
        return 1;
    }


    //set each value of the sensor to 0
    mpu6050->raw_ptr->accX = 0;
    mpu6050->raw_ptr->accY = 0;
    mpu6050->raw_ptr->accZ = 0;
    mpu6050->raw_ptr->gyroX = 0;
    mpu6050->raw_ptr->gyroY = 0;
    mpu6050->raw_ptr->gyroZ = 0;
    mpu6050->raw_ptr->temp = 0;

    mpu6050->cal_ptr->accX = 0;
    mpu6050->cal_ptr->accY = 0;
    mpu6050->cal_ptr->accZ = 0;
    mpu6050->cal_ptr->gyroX = 0;
    mpu6050->cal_ptr->gyroY = 0;
    mpu6050->cal_ptr->gyroZ = 0;
    mpu6050->cal_ptr->temp = 0;

    return 0;
}

int purify_read_lsb(raw_mpu6050_t *mpu6050){
    //Data is read in as MSB, so to make it LSB we follow this algorithm: X = (X >> 8) | (X << 8)
    if(mpu6050 == NULL){
        return 1;
    }

    mpu6050->accX = (int16_t)(((uint16_t)mpu6050->accX << 8) | ((uint16_t)mpu6050->accX >> 8));
    mpu6050->accY = (int16_t)(((uint16_t)mpu6050->accY << 8) | ((uint16_t)mpu6050->accY >> 8));
    mpu6050->accZ = (int16_t)(((uint16_t)mpu6050->accZ << 8) | ((uint16_t)mpu6050->accZ >> 8));
    mpu6050->gyroX = (int16_t)(((uint16_t)mpu6050->gyroX << 8) | ((uint16_t)mpu6050->gyroX >> 8));
    mpu6050->gyroY = (int16_t)(((uint16_t)mpu6050->gyroY << 8) | ((uint16_t)mpu6050->gyroY >> 8));
    mpu6050->gyroZ = (int16_t)(((uint16_t)mpu6050->gyroZ << 8) | ((uint16_t)mpu6050->gyroZ >> 8));
    mpu6050->temp = (int16_t)(((uint16_t)mpu6050->temp << 8) | ((uint16_t)mpu6050->temp >> 8));

    return 0;
}

int applyAccCalibration(accelf_t *axises){
    //applies the correction from the raw int readings to be stored as floats
    if(axises == NULL){
        return 1;
    }

    axises->x = (axises->x - MPU6050_ACCX_OFFSET) / MPU6050_ACCX_SCALE;
    axises->y = (axises->y - MPU6050_ACCY_OFFSET) / MPU6050_ACCY_SCALE;
    axises->z = (axises->z - MPU6050_ACCZ_OFFSET) / MPU6050_ACCZ_SCALE;

    return 0;
}

int applyGyroCalibration(gyrof_t *axises){
    //applies the correction from the raw int readings to be stored as floats
    if(axises == NULL){
        return 1;
    }

    axises->x = axises->x - MPU6050_GYROXX_OFFSET;
    axises->y = axises->y - MPU6050_GYROXY_OFFSET;
    axises->z = axises->z - MPU6050_GYROXZ_OFFSET;

    return 0;
}

int applyTempCalibration(cal_mpu6050_t *mpu6050){
    //applies the correction from the raw int readings to be stored as floats
    if(mpu6050 == NULL){
        return 1;
    }

    //I am thinking of perhaps using the temp for sensor correction
    mpu6050->temp *= 1; //haven't calibrate or test for temp yet

    return 0;
}

int applyFullCalibrations(cal_mpu6050_t *mpu6050){
    //apply the calibrations to both the accelerometer and gyrometer readings
    if(mpu6050 == NULL){
        return 1;
    }

    accelf_t accelReadings;
    gyrof_t gyroReadings;
    
    accelReadings.x = mpu6050->accX;
    accelReadings.y = mpu6050->accY;
    accelReadings.z = mpu6050->accZ;

    gyroReadings.x = mpu6050->gyroX;
    gyroReadings.y = mpu6050->gyroY;
    gyroReadings.z = mpu6050->gyroZ;

    //calibrate the readings
    applyAccCalibration(&accelReadings);
    //store the calibrations
    mpu6050->accX = accelReadings.x;
    mpu6050->accY = accelReadings.y;
    mpu6050->accZ = accelReadings.z;
    
    //calibrate the readings
    applyGyroCalibration(&gyroReadings);
    //store the calibrations
    mpu6050->gyroX = gyroReadings.x;
    mpu6050->gyroY = gyroReadings.y;
    mpu6050->gyroZ = gyroReadings.z;
    

    applyTempCalibration(mpu6050);

    return 0;
}

/*  Following the method desribed in hibit for calculating:
        *roll
        *pitch
        *implementing the complimentary filter

    sources
        1. hibit: https://www.hibit.dev/posts/92/complementary-filter-and-relative-orientation-with-mpu6050
*/
float complementary_filter(const float theta, const float gyroRate, const float accAxis, const uint32_t dt){
    float predictedTheta = 0;

    //angle = (1 - alpha) * (angle + gyroscope * dt) + alpha * accelerometer
    predictedTheta =  MPU6050_ALPHA* (theta + (gyroRate * dt)) + (accAxis * MPU6050_ALPHA_MINUS_ONE);

    return predictedTheta;
}

float calculate_roll(const cal_mpu6050_t *mpu6050, const uint32_t dt){
    //calculate the roll angle through the complimentary angle
    float angle;

    //from hibit, using angle formulas to calculate the roll angle
    //roll = atan2(accelerometer_y, sqrt(accelerometer_x^2 + accelerometer_z^2))
    float x_squared = mpu6050->accX * mpu6050->accX;
    float y_squared = mpu6050->accY * mpu6050->accY;
    float xz_sqrt = sqrt((x_squared) + (y_squared));
    angle = atan2(mpu6050->accY, xz_sqrt);

    //pass the angle through the complimentary filter
    //roll = alpha * (roll + gyroscope_y * dt) + (1 - alpha) * accelerometer_y
    angle = complementary_filter(angle, mpu6050->gyroY, mpu6050->accY, dt);

    return angle;
}

float calculate_pitch(const cal_mpu6050_t *mpu6050, const uint32_t dt){
    //calculate the pitch angle through the complimentary angle
    float angle;

    //from hibit, using angle formulas to calculate the pitch angle
    //pitch = atan2(accelerometer_x, sqrt(accelerometer_y^2 + accelerometer_z^2))
    float z_squared = mpu6050->accZ * mpu6050->accZ;
    float y_squared = mpu6050->accY * mpu6050->accY;
    float yz_sqrt = sqrt((z_squared) + (y_squared));
    angle = atan2(mpu6050->accX, yz_sqrt);

    //pass the angle through the complimentary filter
    //pitch = 0.98 * (pitch + gyroscope_x * dt) + 0.02* accelerometer_x
    angle = complementary_filter(angle, mpu6050->gyroX, mpu6050->accX, dt);

    return angle;
}

