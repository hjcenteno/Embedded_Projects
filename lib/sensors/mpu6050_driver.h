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
#include <math.h> //at the moment using the std library, but if there are drifts I will switch to the cordic coprocessor

#define MPU6050_WHO_AM_I_REG (0x75u) //the register for the who am i
#define mpu6050_saddr_0 (0x68u) //default slave address of the mpu6050
#define mpu6050_saddr_1 (0x69u) //the slave address of the mpu6050 if the ado pin is set high
#define DEGREES_TO_RADIAN(x) (x * (float)(M_PI / 180)) //avoid the mcu from doing the calculations
#define RADIAN_TO_DEGREES(x) (x * (float)(180 / M_PI))
#define MPU6050_ALPHA (0.98f)
#define MPU6050_ALPHA_MINUS_ONE (1 - MPU6050_ALPHA)

/*
    data was retrieved by the following command:
    stdbuf -oL ~/.../mpu6050_testListener/bin/mpu6050_testListener \
        | awk -W interactive '{print} /x:/{if (++n == 1000) exit}' \
        | tee z_down.txt
    
        With the kalman filter, no need to measure the gyro scale
*/
//constants from testings, unique to this board
#define MPU6050_ACCX_OFFSET (0.0501f)
#define MPU6050_ACCX_SCALE (1.0063f)
#define MPU6050_ACCY_OFFSET (-0.0248f)
#define MPU6050_ACCY_SCALE (1.0050f)
#define MPU6050_ACCZ_OFFSET (0.0723f)
#define MPU6050_ACCZ_SCALE (1.0668f)

#define MPU6050_GYROXX_OFFSET (-2.9596f)
#define MPU6050_GYROXY_OFFSET (-2.9591f)
#define MPU6050_GYROXZ_OFFSET (-0.6509f)

//register and bit defines
/*
    The sel bits determine the magnitude of senstivity of the sensor for the accelerometer and gyrometer
    fsel value | range (gyrometer) | sensitivity (lsb/ deg / s)
        0      | +-250 deg/sec | 131 
        1      | +-500 deg/sec | 65.5
        2      | +-1000 deg/sec | 32.8
        3      | +-2000 deg/sec | 16.4

    fsel value | range (accelerometer) lsb/g
        0      | +-2 g | 16384 
        1      | +-4 g | 8192
        2      | +-8 g | 4096
        3      | +-16 g| 2048
*/
#define MPU6050_GYRO_CONFIG_REG (0x1Bu)
#define MPU6050_XG_ST_BIT (1u << 7)
#define MPU6050_YG_ST_BIT (1u << 6)
#define MPU6050_ZG_ST_BIT (1u << 5)
#define MPU6050_FS_SEL_BIT (3u << 3)
#define MPU6050_FS_SEL_BIT_0 (1u << 3)
#define MPU6050_FS_SEL_BIT_1 (1u << 4)
#define MPU6050_GYRO_SENSITIVITY_0 (131)
#define MPU6050_GYRO_SENSITIVITY_1 (65.5f)
#define MPU6050_GYRO_SENSITIVITY_2 (32.8f)
#define MPU6050_GYRO_SENSITIVITY_3 (16.4f)
#define MPU6050_ACCEl_CONFIG_REG 0x1Cu
#define MPU6050_XA_ST_BIT (1u << 7)
#define MPU6050_YA_ST_BIT (1u << 6)
#define MPU6050_ZA_ST_BIT (1u << 5)
#define MPU6050_FS_ASEL_BIT (3u << 3)
#define MPU6050_FS_ASEL_BIT_0 (1u << 3)
#define MPU6050_FS_ASEL_BIT_1 (1u << 4)
#define MPU6050_ACC_SENSITIVITY_0 (16384.0f) //constants left unsigned since the values are unsigned
#define MPU6050_ACC_SENSITIVITY_1 (8192.0f)
#define MPU6050_ACC_SENSITIVITY_2 (4096.0f)
#define MPU6050_ACC_SENSITIVITY_3 (2048u)
#define MPU6050_MOT_THR_REG (0x1Fu) //motion threshold reg

//H: bits[15:8], L: bits[0:7]
#define MPU6050_ACCX_OUT_REG_H (0x3Bu)
#define MPU6050_ACCX_OUT_REG_L (0x3Cu)
#define MPU6050_ACCY_OUT_REG_H (0x3Du)
#define MPU6050_ACCY_OUT_REG_L (0x3Eu)
#define MPU6050_ACCZ_OUT_REG_H (0x3Fu)
#define MPU6050_ACCZ_OUT_REG_L (0x40u)
#define MPU6050_TEMP_OUT_H (0x41u)
#define MPU6050_TEMP_OUT_L (0x42u)
#define MPU6050_GYROX_OUT_REG_H (0x43u)
#define MPU6050_GYROX_OUT_REG_L (0x44u)
#define MPU6050_GYROY_OUT_REG_H (0x45u)
#define MPU6050_GYROY_OUT_REG_L (0x46u)
#define MPU6050_GYROZ_OUT_REG_H (0x47u)
#define MPU6050_GYROZ_OUT_REG_L (0x48u)
#define MPU6050_DATA_START_ADDR (MPU6050_ACCX_OUT_REG_H)
#define MPU6050_DATA_END_ADDR (MPU6050_GYROZ_OUT_REG_L)

//power management registers and bits
#define MPU6050_PWR_MGMT_1 (0x6Bu)
#define MPU6050_DEVICE_RST_BIT (1u << 7)
#define MPU6050_SLEEP_BIT (1u << 6)
#define MPU6050_CYCLE_BIT (1u << 5)
#define MPU6050_TMP_DIS_BIT (1u << 3)
#define MPU6050_CLKSEL_BIT (7u << 0)
#define MPU6050_CLKSEL_BIT_1 (1u << 0)
#define MPU6050_CLKSEL_BIT_2 (1u << 1)
#define MPU6050_CLKSEL_BIT_3 (1u << 2)

/*
    The lp_wake_crtl bits can be used to configure the wake frequency of the mpu6050 from sleep and power-on
    
    lp_wake_crtl value | freq
        0   | 1.25 Hz
        1   | 5 Hz
        2   | 20 Hz
        3   | 40 Hz

*/
#define MPU6050_PWR_MGMT_2 (0x6Cu)
#define MPU6050_LP_WAKE_CTRL_BIT (3 << 6)
#define MPU6050_LP_WAKE_CTRL_BIT_0 (1 << 6)
#define MPU6050_LP_WAKE_CTRL_BIT_1 (1 << 7)
#define MPU6050_STBY_XA_BIT (1 << 5)
#define MPU6050_STBY_YA_BIT (1 << 4)
#define MPU6050_STBY_ZA_BIT (1 << 3)
#define MPU6050_STBY_XG_BIT (1 << 2)
#define MPU6050_STBY_YG_BIT (1 << 1)
#define MPU6050_STBY_ZG_BIT (1 << 0)

//send this as the data for the i2c to store the raw bytes
typedef struct raw_mpu6050_t{
    //matches how where each value is in stored registers
    int16_t accX;
    int16_t accY;
    int16_t accZ;
    int16_t temp;
    int16_t gyroX;
    int16_t gyroY;
    int16_t gyroZ;
}raw_mpu6050_t;

typedef struct cal_mpu6050_t{
    //holds the data to do math from the raw data
    float accX;
    float accY;
    float accZ;
    float temp;
    float gyroX;
    float gyroY;
    float gyroZ;
}cal_mpu6050_t;

typedef struct mpu6050_ptr{
    //to pass both around
    raw_mpu6050_t *raw_ptr;
    cal_mpu6050_t *cal_ptr;
}mpu6050_ptr; 


//float struct of the normalized accelerometer data
typedef struct accelf_t{
    float x;
    float y;
    float z;
}accelf_t;

//float struct of the normalized accelerometer data
typedef struct gyrof_t{
    float x;
    float y;
    float z;
}gyrof_t;

//functions to do stuff to the mpu6050 sensor
int purify_read_lsb(raw_mpu6050_t *mpu6050); //call to push the data read to make it into least significant byte format
int zero_out_mpu6050(mpu6050_ptr *mpu6050);

//calibrations
int applyAccCalibration(accelf_t *axises); //performs only for the accelerometer data
int applyGyroCalibration(gyrof_t *axises); //performs only for the gyrometer data
int applyTempCalibration(cal_mpu6050_t *mpu6050);
int applyFullCalibrations(cal_mpu6050_t *mpu6050); //performs all the calibrations to the full cal_mpu6050_t struct

//calculate the data
float kalman_filter(cal_mpu6050_t *mpu6050);
float complementary_filter(const float theta, const float gyroRate, const float accAxis, const float dt);
float calculate_roll(const cal_mpu6050_t *mpu6050, const float prevRoll, const float dt);
float calculate_pitch(const cal_mpu6050_t *mpu6050, const float prevPitch, const float dt);
float calculate_yaw(const cal_mpu6050_t *mpu6050, const float prevYaw, const float dt); 

#endif