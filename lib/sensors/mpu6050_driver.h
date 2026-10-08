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

#define MPU6050_WHO_AM_I_REG (0x75u) //the register for the who am i
#define mpu6050_saddr_0 (0x68u) //default slave address of the mpu6050
#define mpu6050_saddr_1 (0x69u) //the slave address of the mpu6050 if the ado pin is set high
#define DEGREES_TO_RADIAN(x) ((x * M_PI) / 180) //avoid the mcu from doing the calculations
#define RADIAN_TO_DEGREES(x) ((x * 180) / M_PI)

//constants from testings, unique to this board
#define MPU6050_ACCX_OFFSET ((1.053 - 0.954) / 2) //in testing, the upright the y axis averaged to 1.053, downright was 0.954
#define MPU6050_ACCX_SCALE ((1.053 + 0.954) / 2)
#define MPU6050_ACCY_OFFSET ((0.978 - 1.028) / 2) //in testing, the upright the y axis averaged to .978, downright was 1.028
#define MPU6050_ACCY_SCALE ((0.978 + 1.028) / 2)
#define MPU6050_ACCZ_OFFSET ((1.139 - 0.911) / 2) //in testing, the upright the z axis averaged to 1.139, downright was 0.911
#define MPU6050_ACCZ_SCALE ((1.139 + 0.911) / 2)

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
typedef struct mpu6050_t{
    //matches how where each value is in stored registers
    int16_t accX;
    int16_t accY;
    int16_t accZ;
    int16_t temp;
    int16_t gyroX;
    int16_t gyroY;
    int16_t gyroZ;
}mpu6050_t;

//float struct of the normalized accelerometer data
typedef struct accelf_t{
    float x;
    float y;
    float z;
}accelf_t;

//float struct of the normalized accelerometer data
typedef struct gyrof_t{
    float roll;
    float pitch;
    float yaw;
}gyrof_t;

//functions to do stuff to the mpu6050 sensor
void purify_read_lsb(mpu6050_t *mpu6050); //call to push the data read to make it into least significant byte format
void zero_out_mpu6050(mpu6050_t *mpu6050);

//calculate the data
void applyAccCalibration(accelf_t *axises);
float kalman_filter(mpu6050_t *mpu6050);
float calculate_roll(mpu6050_t *mpu6050);
float calculate_pitch(mpu6050_t *mpu6050);
float calculate_yaw(mpu6050_t *mpu6050);

#endif