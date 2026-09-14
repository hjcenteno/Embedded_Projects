/*
    Author: Henry Centeno
    Description:
        Utilizes the HC-SR04 ultrasound sensor to detect the distance of an object
        For the purposes of testing, 3 LEDs will be used to simulate visual closeness of the object,
        and I will add a buzzer (hopefully) whose frequency is relative to the distance detected, 
        then I will transmit the distance through the LPUART.
    
    Components | Pins | Peripherals
    HC-SR04 VCC | 5V 
    HC-SR04 GND | GND
    HC-SR04 Trig | D2 | PA10
    HC-SR04 Echo | D3 | PB3
    LED1 | D12 | PA6
    LED2 | D11 | PA7
    LED3 | D10 | PB6
*/

#include "common_includes/common_includes.h"
#include "system_time/mcu_time.h"
#include "uart_driver/lpuart_driver.h"

volatile uint32_t echoRisingEdge = 0;
volatile uint32_t echoFallingEdge = 0;
volatile bool echoRecorded = false;

void init_leds(void){
    //initiates the necessary gpio pins to gp output
    //set the pins to moder to '01' 
    GPIOA->MODER &= ~(GPIO_MODER_MODER6 | GPIO_MODER_MODER7); //pa6 and pa7
    GPIOA->MODER |= (GPIO_MODER_MODER6_0 | GPIO_MODER_MODER7_0);
    GPIOB->MODER &= ~GPIO_MODER_MODE6; //pb6
    GPIOB->MODER |= GPIO_MODER_MODER6_0;
}

void init_HCSR04(void){
    //set up the trig and echo pins
    //trig configuration (pa10) set to gp output to drive it high then low to trigger
    GPIOA->MODER &= ~GPIO_MODER_MODE10;
    GPIOA->MODER |= GPIO_MODER_MODE10_0; //set to gp output
    
    //echo location (pb3) to tim_ch2 for input capture mode
    /*
    RM04 page 1315
    In input capture mode, the capture/compare registers (TIMx_CCRx) are used to latch the
    value of the counter after a transition detected by the corresponding ICx signal.
    */
    GPIOB->MODER &= ~GPIO_MODER_MODE3; //set pb3 to AF1
    GPIOB->MODER |= GPIO_MODER_MODE3_1; //'10' for AF
    GPIOB->AFR[0] &= ~GPIO_AFRL_AFRL3; //clearing the pin 3 bits in AFRL
    GPIOB->AFR[0] |= GPIO_AFRL_AFSEL3_0; //write 1 to use AF1
    
    //configure channel 2 for the tim2
    //init_tim2(clock, channels); channels masked in a uint8_t where first 4 bits are inputs amd second 4 is output.
    init_tim2_ch2_input(1); //set it to a high priority since for now it is the only sensor used
}

void init_components(void){
    //general gpio init function for port A and B since they are the ones to be used
    //activate their peripheral clocks
    RCC->AHB2ENR &= ~(RCC_AHB2ENR_GPIOAEN | RCC_AHB2ENR_GPIOBEN);
    RCC->AHB2ENR |= (RCC_AHB2ENR_GPIOAEN | RCC_AHB2ENR_GPIOBEN);

    init_leds();
    init_HCSR04();
}

void trigger_HCSR04(void){
    //the trig pin in the hcsr04 must pulse high for at least 10 us
    GPIOA->BSRR = GPIO_BSRR_BS10; //set it to high
    delay_us(10); //delay for 10 us per the manual for the HCSR04
    GPIOA->BSRR = GPIO_BSRR_BR10; //set it low
}

float getDistance(void){
    /*
        gets the distance in inches following this formula from the hcsr04 user manual
        distance = pulse width(uS) / 148 for inches
    */

    float distance = 0;
    
    if(echoRecorded == true){
        distance = (float)(echoFallingEdge - echoRisingEdge) / 148;
    }

    return distance;
}

void TIM2_IRQHandler(void){
    //tim2 read a rising/falling edge
    if(TIM2->SR & TIM_SR_CC2IF){ //check for CC2IF
        //check for a rising edge at the pin
        if(GPIOB->IDR & GPIO_IDR_ID3){
            echoRisingEdge = TIM2->CCR2; //get the count on the rising edge
        }else{
            echoRecorded = true;
            echoFallingEdge = TIM2->CCR2; //get the count on the falling edge
        }
    }

    //clear the bit at the end
    TIM2->SR &= ~TIM_SR_CC2IF;
}

int main(void){
    // init_lpuart();
    init_tim2();
    init_components();

    //maximum distance (inches) to activate the LED
    float MAXIMUM_LED1_THRESHOLD = 3;
    float MAXIMUM_LED2_THRESHOLD = 6;
    float MAXIMUM_LED3_THRESHOLD = 9;

    while(1){
        trigger_HCSR04(); //trigger the trig line high

        //capture the duration of echo signal
        float distance = getDistance(); 

        GPIOB->BSRR = GPIO_BSRR_BR6;
        GPIOA->BSRR = GPIO_BSRR_BR7;
        GPIOA->BSRR = GPIO_BSRR_BR6;

        //turn on the depending on the distance
        if((distance > MAXIMUM_LED2_THRESHOLD) && (distance  <= MAXIMUM_LED3_THRESHOLD)){
            //turn on led 3
            GPIOB->BSRR = GPIO_BSRR_BS6;
        }

        else if((distance > MAXIMUM_LED1_THRESHOLD) && (distance  <= MAXIMUM_LED2_THRESHOLD)){
            //turn on led 2
            GPIOA->BSRR = GPIO_BSRR_BS7;
        }

        else if((distance > 0) && (distance <= MAXIMUM_LED1_THRESHOLD)){
            //turn on led 1
            GPIOA->BSRR = GPIO_BSRR_BS6;
        }
    }

    return 0;
}
