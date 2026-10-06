/*
    Author: Henry Centeno
    Description:
        This is to test the hcsr04 driver using the original ultrasound_sensing.c as a benchmark for:
            -text size
            -performance
            -and if it works
    
    Components | Pins | Peripherals
        HC-SR04 VCC | 5V 
        HC-SR04 GND | GND
        HC-SR04 Trig | D2 | PA10
        HC-SR04 Echo | D3 | PB3
        LED1 | D12 | PA6
    LED2 | D11 | PA7
    LED3 | D10 | PB6
*/

#include "sensors/hcsr04_driver.h"
#include "uart_driver/lpuart_driver.h"
#include "system_time/mcu_time.h"

//sensors initializing
hcsr04_t mainSensor;

void TIM2_IRQHandler(void){
    hcsr04_IRQHandler(&mainSensor);
}

void init_leds(void){
    //initiates the necessary gpio pins to gp output
    //set the pins to moder to '01' 
    RCC->AHB2ENR &= ~(RCC_AHB2ENR_GPIOAEN | RCC_AHB2ENR_GPIOBEN);
    RCC->AHB2ENR |= (RCC_AHB2ENR_GPIOAEN | RCC_AHB2ENR_GPIOBEN);

    GPIOA->MODER &= ~(GPIO_MODER_MODER6 | GPIO_MODER_MODER7); //pa6 and pa7
    GPIOA->MODER |= (GPIO_MODER_MODER6_0 | GPIO_MODER_MODER7_0);
    GPIOB->MODER &= ~GPIO_MODER_MODE6; //pb6
    GPIOB->MODER |= GPIO_MODER_MODER6_0;
}

int main(void){
    init_tim2();
    init_lpuart();
    init_leds();
    init_HCSR04(&mainSensor, 1);

    //maximum distance (inches) to activate the LED
    float MAXIMUM_LED1_THRESHOLD = 3;
    float MAXIMUM_LED2_THRESHOLD = 6;
    float MAXIMUM_LED3_THRESHOLD = 9;

    while(1){
        trigger_HCSR04(); //trigger the trig line high

        //capture the duration of echo signal
        float distance = getDistance_HCSR04(&mainSensor);

        GPIOB->BSRR = GPIO_BSRR_BR6;
        GPIOA->BSRR = GPIO_BSRR_BR7;
        GPIOA->BSRR = GPIO_BSRR_BR6;

        //turn on the depending on the distance
        if((distance > MAXIMUM_LED2_THRESHOLD) && (distance  <= MAXIMUM_LED3_THRESHOLD)){
            //turn on led 3, leave the others off
            GPIOB->BSRR = GPIO_BSRR_BS6; //led 3
            client_transmit((uint8_t *)&distance, sizeof(distance));
        }

        else if((distance > MAXIMUM_LED1_THRESHOLD) && (distance  <= MAXIMUM_LED2_THRESHOLD)){
            //turn on led 2
            GPIOA->BSRR = GPIO_BSRR_BS7; //led 2
            client_transmit((uint8_t *)&distance, sizeof(distance));
        }

        else if((distance > 0) && (distance <= MAXIMUM_LED1_THRESHOLD)){
            //turn on led 1
            GPIOA->BSRR = GPIO_BSRR_BS6; //led 1
            client_transmit((uint8_t *)&distance, sizeof(distance));
        }
    }

    return 0;
}