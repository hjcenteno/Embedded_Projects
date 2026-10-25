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

void init_pwm_pins(void){
    //initiates the pins for pwm
    //d13/pa5 => tim2_ch1 af1
    //d12/pa6 => tim16_ch1 af1
    //d11/pa7 => tim17_ch1 af1
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
        (1u << GPIO_AFRL_AFSEL6_Pos) | 
        (1u << GPIO_AFRL_AFSEL7_Pos)
    );
    GPIOB->AFR[0] |= (2u << GPIO_AFRL_AFSEL6_Pos);
}

void setup_pwm_ch1(uint32_t targetHz, float initialDC){
    //initial setup of the pwm channels at a 50% duty cycle
    //rewrite the arr
    TIM2->ARR = targetHz;
    TIM4->ARR = targetHz;
    TIM16->ARR = targetHz;
    TIM17->ARR = targetHz;

    //buffer it
    TIM2->CR1 |= TIM_CR1_ARPE;
    TIM4->CR1 |= TIM_CR1_ARPE;
    TIM16->CR1 |= TIM_CR1_ARPE;
    TIM17->CR1 |= TIM_CR1_ARPE;

    //generate the update
    TIM2->EGR |= TIM_EGR_UG;
    TIM4->EGR |= TIM_EGR_UG;
    TIM16->EGR |= TIM_EGR_UG;
    TIM17->EGR |= TIM_EGR_UG;

    //set the duty cycles at 50%
    TIM2->CCR1 = update_pwm_dutyCycle(TIM2, initialDC);
    TIM4->CCR1 = update_pwm_dutyCycle(TIM4, initialDC);
    TIM16->CCR1 = update_pwm_dutyCycle(TIM16, initialDC);
    TIM17->CCR1 = update_pwm_dutyCycle(TIM17, initialDC);
}

int main(void){
    init_pwm_pins();

    activate_SWS_RCC_HSE();
    
    //activate the tims needed, tim6 will act as a basic timer
    RCC->APB1ENR1 |= (RCC_APB1ENR1_TIM2EN | RCC_APB1ENR1_TIM4EN | RCC_APB1ENR1_TIM6EN);
    RCC->APB2ENR |= (RCC_APB2ENR_TIM16EN | RCC_APB2ENR_TIM17EN);
    init_tim(TIM2);
    init_tim(TIM4);
    init_tim(TIM16);
    init_tim(TIM17);
    init_tim(TIM6);

    //each tim channel defined will be set up in pwm mode
    uint32_t pwm_mask = ((0b110 << TIM_CCMR1_OC1M_Pos) | TIM_CCMR1_OC1PE); //do tim2_ch1 first
    uint32_t ccer_mask = TIM_CCER_CC1E;
    //good thing about each tim using ch1 is that the mask can be reused
    cfgr_tim_ccmr1(TIM2, pwm_mask, ccer_mask);
    cfgr_tim_ccmr1(TIM4, pwm_mask, ccer_mask);
    cfgr_tim_ccmr1(TIM16, pwm_mask, ccer_mask);
    cfgr_tim_ccmr1(TIM17, pwm_mask, ccer_mask);

    //set up the initial duty cycle and frequency
    uint32_t freq = 1000; //let's do 1khz initially

    setup_pwm_ch1(freq, 50.0f); //does each tim to keep the code less bulky in main

    while (1){
        /* code */
    }
    
}