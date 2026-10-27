/*
    Author: Henry Centeno
    Description:
        The purpose of this program is to manage balancing the thrust of of the propellers to maintain equillibrium.
        I will use the mpu6050 and 4 motors, the mcu will then drive the motors to run in a way to maintain the
        roll and pitch angles close towards 0 degrees with some tolerance

    components | pin | peripheral
        scl | d15 | pb8
        sda | d14 | pb9
        led1 | d13 | pa5
        led2 | d12 | pa6
        led3 | d11 | pa7
        led4 | d10 | pb6

    Version 0.1:
        At the moment, I do not have propellers so I will use leds as stand-in.
        The initial duty cylce of each led will be set to 50%, and then each led's duty cylce
        will then change depending on the roll and pitch angle. 
*/

#include "system_time/timx_utils.h"
#include "sensors/mpu6050_driver.h"
#include "i2c_driver/i2c_driver.h"

#define DC_STEP (1.0f)

typedef enum mainState_t{
    readSensorEn = 0, //read the sensor first, transition only on success
    readData = 1 //read the data
}mainState_t;

void init_pwm_pins(void){
    //initiates the pins for pwm
    //d13/pa5 => tim2_ch1 af1
    //d12/pa6 => tim3_ch1 af2
    //d11/pa7 => tim3_ch2 af2
    //d10/pb6 => tim4_ch1 af2
    RCC->AHB2ENR |= (RCC_AHB2ENR_GPIOAEN | RCC_AHB2ENR_GPIOBEN); //turn on the peripheral clock
    
    //clear the pins for af mode
    GPIOA->MODER &= ~(GPIO_MODER_MODE5 | GPIO_MODER_MODE6 | GPIO_MODER_MODE7);
    GPIOB->MODER &= ~GPIO_MODER_MODE6;
    GPIOA->AFR[0] &= ~(GPIO_AFRL_AFSEL5 | GPIO_AFRL_AFSEL6 | GPIO_AFRL_AFSEL7);
    GPIOB->AFR[0] &= ~GPIO_AFRL_AFSEL6;

    //set the pins af
    GPIOA->MODER |= (GPIO_MODER_MODE5_1 | GPIO_MODER_MODE6_1 | GPIO_MODER_MODE7_1);
    GPIOB->MODER |= GPIO_MODER_MODE6_1;
    GPIOA->AFR[0] |= (
        (1u << GPIO_AFRL_AFSEL5_Pos) | 
        (2u << GPIO_AFRL_AFSEL6_Pos) | 
        (2u << GPIO_AFRL_AFSEL7_Pos)
    );
    GPIOB->AFR[0] |= (2u << GPIO_AFRL_AFSEL6_Pos);
}

void setup_pwm_ch1(uint32_t targetHz, float initialDC){
    //initial setup of the pwm channels at a 50% duty cycle
    //rewrite the arr
    TIM2->ARR = targetHz;
    TIM4->ARR = targetHz;
    TIM3->ARR = targetHz;

    //buffer it
    TIM2->CR1 |= TIM_CR1_ARPE;
    TIM4->CR1 |= TIM_CR1_ARPE;
    TIM3->CR1 |= TIM_CR1_ARPE;

    //generate the update
    TIM2->EGR |= TIM_EGR_UG;
    TIM4->EGR |= TIM_EGR_UG;
    TIM3->EGR |= TIM_EGR_UG;

    //set the duty cycles at 50%
    TIM2->CCR1 = update_pwm_dutyCycle(TIM2, initialDC);
    TIM4->CCR1 = update_pwm_dutyCycle(TIM4, initialDC);
    TIM3->CCR1 = update_pwm_dutyCycle(TIM3, initialDC);
    TIM3->CCR2 = update_pwm_dutyCycle(TIM3, initialDC);
}

//returns true if and only if the whoAmI register gave 0x68 and the sensor woke up
bool init_mpu6050(void){
    //check if we can read the whoAmI register
    uint8_t whoAmi = 0;
    if(i2c_master_read(mpu6050_saddr_0, MPU6050_WHO_AM_I_REG, &whoAmi, sizeof(whoAmi)) == 0){
        if(whoAmi != 0x68){
            return false;
        }
    }

    //check if we can wake up the sensor
    uint8_t pwr_mgmt_mask = 0 & (~MPU6050_SLEEP_BIT); //sensor is initially in sleep mode (sleep bit = 1)
    if(i2c_master_write(mpu6050_saddr_0, MPU6050_PWR_MGMT_1, &pwr_mgmt_mask, sizeof(pwr_mgmt_mask)) != 0){
        return false; //failed write
    }

    return true;
}

int main(void){
    //initial setups
    activate_SWS_RCC_HSE();
    init_pwm_pins();
    init_i2c(fast);
    
    //mpu6050 setup
    bool readSensor = init_mpu6050();
    raw_mpu6050_t rawReadings;
    cal_mpu6050_t calReadings;
    mpu6050_ptr mainSensor = {
        .raw_ptr = &rawReadings,
        .cal_ptr = &calReadings
    };
    zero_out_mpu6050(&mainSensor);

    orienta
    

    //activate the tims needed, tim6 will act as a basic timer
    RCC->APB1ENR1 |= (
        RCC_APB1ENR1_TIM2EN | RCC_APB1ENR1_TIM4EN | 
        RCC_APB1ENR1_TIM3EN | RCC_APB1ENR1_TIM6EN
    );
    init_tim(TIM2);
    init_tim(TIM3);
    init_tim(TIM4);
    init_tim(TIM6);

    //each tim channel defined will be set up in pwm mode
    uint32_t pwm_mask = ((0b110 << TIM_CCMR1_OC1M_Pos) | TIM_CCMR1_OC1PE); //do tim2_ch1 first
    uint32_t ccer_mask = TIM_CCER_CC1E;
    //all but pa7 uses channel 1 for their tim
    cfgr_tim_ccmr1(TIM2, pwm_mask, ccer_mask);
    cfgr_tim_ccmr1(TIM4, pwm_mask, ccer_mask);

    //configure channel 2 for tim3
    pwm_mask = ((0b110 << TIM_CCMR1_OC1M_Pos) | TIM_CCMR1_OC1PE) | ((0b110 << TIM_CCMR1_OC2M_Pos) | TIM_CCMR1_OC2PE);
    ccer_mask = TIM_CCER_CC1E | TIM_CCER_CC2E;
    cfgr_tim_ccmr1(TIM3, pwm_mask, ccer_mask);

    //set up the initial duty cycle and frequency
    uint32_t freq = 1000; //let's do 1khz initially

    float target_percent = 50;
    float dc_step = -DC_STEP;
    setup_pwm_ch1(freq, target_percent); //does each tim to keep the code less bulky in main
    
    uint32_t startTime = 0; //track time between data reads of the sensor
    while(1){
        //see if we can read the sensor
        if(!readSensor){
            //recover the sensor if not
            readSensor = init_mpu6050();
            delay_ms(TIM6, 500);
            startTime = getTime(TIM6);
            continue;
        }

        //read the accelerometer and gyrometer data
        if(i2c_master_read(mpu6050_saddr_0, MPU6050_DATA_START_ADDR, PTR_TO_BYTES(mainSensor.raw_ptr), sizeof(*mainSensor.raw_ptr)) != 0){ //assume it failed
            readSensor = false;
            recover_i2c();
            continue;
        }
        purify_read_lsb(mainSensor.raw_ptr); //the data is transmitted as MSB
        
        //calibrate the data
        mainSensor.cal_ptr->accX / MPU6050_ACC_SENSITIVITY_0; //debating whether I should include these lines in the function call or not
        mainSensor.cal_ptr->accY / MPU6050_ACC_SENSITIVITY_0;
        mainSensor.cal_ptr->accZ / MPU6050_ACC_SENSITIVITY_0;
        mainSensor.cal_ptr->gyroX / MPU6050_GYRO_SENSITIVITY_0;
        mainSensor.cal_ptr->gyroY / MPU6050_GYRO_SENSITIVITY_0;
        mainSensor.cal_ptr->gyroZ / MPU6050_GYRO_SENSITIVITY_0;
        applyFullCalibrations(mainSensor.cal_ptr);
        uint32_t currentTime = getTime(TIM6); //time it took to read the data plus any setbacks

        //get the roll and pitch angles
        float dt = currentTime - startTime;


        startTime = currentTime;
    }

    return 0;
}