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

#define DC_STEP (1.0f)

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

int main(void){
    init_pwm_pins();

    activate_SWS_RCC_HSE();
    
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
    cfgr_tim_ccmr1(TIM3, pwm_mask, ccer_mask);

    //configure channel 2 for tim3
    pwm_mask = ((0b110 << TIM_CCMR1_OC2M_Pos) | TIM_CCMR1_OC2PE);
    ccer_mask = TIM_CCER_CC2E;
    cfgr_tim_ccmr1(TIM3, pwm_mask, ccer_mask);

    //set up the initial duty cycle and frequency
    uint32_t freq = 1000; //let's do 1khz initially

    float target_percent = MAX_DC_PERCENT;
    float dc_step = -DC_STEP;
    setup_pwm_ch1(freq, target_percent); //does each tim to keep the code less bulky in main
    while (1){
        target_percent += dc_step;

        //odulate between the min and max duty cycle percents
        if(target_percent < MIN_DC_PERCENT){
            dc_step = DC_STEP;
            target_percent = MIN_DC_PERCENT;
        }else if(target_percent >= MAX_DC_PERCENT){
            dc_step = -DC_STEP;
            target_percent = MAX_DC_PERCENT;
        }

        //update the pwm
        TIM2->CCR1 = update_pwm_dutyCycle(TIM2, target_percent);
        TIM4->CCR1 = update_pwm_dutyCycle(TIM4, target_percent);
        TIM3->CCR1 = update_pwm_dutyCycle(TIM3, target_percent);
        TIM3->CCR2 = update_pwm_dutyCycle(TIM3, target_percent);
        delay_ms(TIM6, 10);
    }
}