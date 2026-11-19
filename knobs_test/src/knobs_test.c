/*
    Author: Henry Centeno
    Description: This is to test the knobs header implementation

    Components | Pin | peripheral
        s1 | D3 | pb3
        s2 | D5 | pb4
        key | D4 | pb5
*/

#include "devices/knobs.h"
#include "uart_driver/lpuart_driver.h"
#include "system_time/timx_utils.h"

#define MIN_DELTA (-1.0f)
#define MAX_DELTA (1.0f)
#define DELTA_STEP (0.1f)

static struct KnobTransmission{
    float rotationAmt;
    float delta;
} kt;

void init_gpioPins(void){
    RCC->AHB2ENR |= RCC_AHB2ENR_GPIOBEN;

    //set their modes to gp input with pull up for logic low 
    GPIOB->MODER &= ~(GPIO_MODER_MODE3 | GPIO_MODER_MODE4 | GPIO_MODER_MODE5);
    GPIOB->PUPDR &= ~(GPIO_PUPDR_PUPD3 | GPIO_PUPDR_PUPD4 | GPIO_PUPDR_PUPD5);
    GPIOB->PUPDR &= (GPIO_PUPDR_PUPD3_0 | GPIO_PUPDR_PUPD4_0 | GPIO_PUPDR_PUPD5_0);
}

int main(void){
    //initializations
    //tim2 again for a basic timer since it is 32 bit
    RCC->APB1ENR1 |= RCC_APB1ENR1_TIM2EN;
    init_tim(TIM2);
    init_lpuart();
    init_gpioPins();

    KnobMachine km1 = { //on gpiob
        .pinMasks = 0,
        .prevS1 = 0,
        .q = idle
    };
    
    km1.pinMasks |= (
        SET_PIN(3u, S1_POS) |
        SET_PIN(4u, S2_POS) |
        SET_PIN(5u, KEY_POS)
    );

    //going to use qemu to simulate the behaviors and using gdp set var to change b_idr
    while(1){
        const uint32_t b_idr = ~GPIOB->IDR;
        KnobStates loopState = km1.q;
        switch (loopState){
        case idle:
            loopState = transitionFrom_idle(&km1, b_idr);
            break;
        case invalid:
            loopState = transitionFrom_invalid(&km1, b_idr);
            break;
        case pressed:
            loopState = transitionFrom_pressed(&km1, b_idr);
            break;
        case cw:
            loopState = transitionFrom_cw(&km1, b_idr);
            break;
        case ccw:
            loopState = transitionFrom_ccw(&km1, b_idr);
            break;
        default:
            break;
        }

        km1.q = loopState;
        km1.prevS1 = b_idr & GPIO_IDR_ID3; //pb3 is set to s1, just going to use it directly since the get bit is more expensive

        //handle the state behavior
        switch(loopState){
            case idle:
            case invalid:
                continue; //do nothing
            case pressed:
                //clamp the delta between a min and max
                kt.delta += DELTA_STEP;
                kt.delta = CLAMP(MIN_DELTA, kt.delta, MAX_DELTA); //kinda like the arr in tim, but hopefully goes to min_delta and increments from there
                
                if((kt.delta == MIN_DELTA) || (kt.delta == MAX_DELTA)){
                    kt.delta *= -1;
                }

                break;
            case cw:
                //increment on cw
                kt.rotationAmt += kt.delta; 
                break;
            case ccw:
                //decrememt on ccw
                kt.rotationAmt -= kt.delta;
                break;
            default:
                break;
        }

        // prevState = nextState;
        client_transmit(TO_BYTE_ARRAY(kt), sizeof(kt));
        delay_ms(TIM2, 2);
    }

    return 0;
}