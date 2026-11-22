/*
    Author: Henry Centeno
    Description:
        Configure three rotary encoders and be able to read their inputs.

    knob 1:
    Components | pin | peripheral
        s1     | CN10.1 | pc9
        s2     | CN10.2 | pc8
        key    | CN10.4 | pc6

    knob 2:
    Components | pin | peripheral
        s1     | CN10.12 | pa12
        s2     | CN10.14 | pa11
        key    | CN10.21 | pa9

    knob 3:
    Components | pin | peripheral
        s1     | CN7.2 | pc11
        s2     | CN7.1 | pc10
        key    | CN7.3 | pc12
*/

#include "common_includes/common_includes.h"
#include "stm32g474xx.h"
#include "uart_driver/lpuart_driver.h"
#include "system_time/timx_utils.h"
#include "devices/knobs.h"

//between 1 hz and 10 Khz
#define FREQ_STEP (0.1f) //how mcuh each turn of the knob adds to the freq
#define MAX_FREQ_STEP (100.0f)
#define MIN_FREQ_STEP (-100.0f)
#define MIN_FREQ (1.0f)
#define MAX_FREQ (10000.0f)

#define DC_STEP (0.1f) //how much each turn of the knob adds to the dc
#define MAX_DC_STEP (1.0f)
#define MIN_DC_STEP (-1.0f)

//between 250 us and 5000 us
#define DT_STEP (1)
#define MAX_DT_STEP (10)
#define MIN_DT_STEP (1)
#define MAX_DT_US (5000u)
#define MIN_DT_US (250u)

typedef struct pwm_t{
    uint32_t deltaTime;
    float freq;
    float dc;
}PWM_T;

void init_GPIOPins(void){
    //activate the peripheral in the rcc
    RCC->AHB2ENR |= (RCC_AHB2ENR_GPIOAEN | RCC_AHB2ENR_GPIOCEN);

    //I am using the ky040 ec11 module, and the module is built with pull-up resistors
    //I will initially set the gpio pins to PU to avoid noise and uneccessary reads
    //first clear the moder registers and leave it as 00 for input
    GPIOA->MODER &= ~(GPIO_MODER_MODE9 | GPIO_MODER_MODE11 | GPIO_MODER_MODE12);
    GPIOC->MODER &= ~(
        GPIO_MODER_MODE6 | GPIO_MODER_MODE8 | GPIO_MODER_MODE9 |
        GPIO_MODER_MODE10 | GPIO_MODER_MODE11 | GPIO_MODER_MODE12 
    );
    //clear the pupdr
    GPIOA->PUPDR &= ~(GPIO_PUPDR_PUPD9 | GPIO_PUPDR_PUPD11 | GPIO_PUPDR_PUPD12);
    GPIOC->PUPDR &= ~(
        GPIO_PUPDR_PUPD6 | GPIO_PUPDR_PUPD8 | GPIO_PUPDR_PUPD9 |
        GPIO_PUPDR_PUPD10 | GPIO_PUPDR_PUPD11 | GPIO_PUPDR_PUPD12 
    );

    //set the pupdr to pu
    GPIOA->PUPDR |= (GPIO_PUPDR_PUPD9_0 | GPIO_PUPDR_PUPD11_0 | GPIO_PUPDR_PUPD12_0);
    GPIOC->PUPDR |= (
        GPIO_PUPDR_PUPD6_0 | GPIO_PUPDR_PUPD8_0 | GPIO_PUPDR_PUPD9_0 |
        GPIO_PUPDR_PUPD10_0 | GPIO_PUPDR_PUPD11_0 | GPIO_PUPDR_PUPD12_0 
    );
}
//this function will get the state of the knob and store the previous state of the s1 pin
void update_km_state(KnobMachine *km, const uint32_t idr){
    KnobStates currState = km->q; //get the 
    switch (currState){
        case idle:
            currState = transitionFrom_idle(km, idr);
            break;
        case invalid:
            currState = transitionFrom_invalid(km, idr);
            break;
        case pressed:
            currState = transitionFrom_pressed(km, idr);
            break;
        case cw:
            currState = transitionFrom_cw(km, idr);
            break;
        case ccw:
            currState = transitionFrom_ccw(km, idr);
            break;
        default:
            break;
    }

    km->q = currState;
    km->prevS1 = KNOB_S1(idr, km->pinMasks);
}

/**On idle or invalid, skip handling input.
 * On pressed, the step is incremented then decrement in an oscilating pattern
 * On cw, increment the field by its step
 * On ccw, decrement the field by its step
 */
void update_frequency(const KnobStates q, float *step, float *freq){
    float newFreq = *freq; //avoid directly updating the parameter
    float newStep = *step;
    switch(q){
        case idle:
        case invalid:
            break;
        case pressed:
            newStep += FREQ_STEP;
            *step = CLAMP(MIN_FREQ_STEP, newStep, MAX_FREQ_STEP);
            
            //reverse the direction when either extreme is met
            if((*step == MIN_FREQ_STEP) || (*step == MAX_FREQ_STEP)){
                *step *= -1;
            }

            break;
        case cw:
            newFreq += *step;
            *freq = CLAMP(MIN_FREQ, newFreq, MAX_FREQ);
            break;
        case ccw:
            newFreq -= *step;
            *freq = CLAMP(MIN_FREQ, newFreq, MAX_FREQ);
            break;
        default:
            break;
    }
}

void update_dutyCycle(const KnobStates q, float *step, float *dc){
    float newDc = *dc;
    float newStep = *step;

    switch(q){
        case idle:
        case invalid:
            break;
        case pressed:
            newStep += DC_STEP;
            if((newStep == MIN_DC_STEP) || (newStep == MAX_DC_STEP)){
                newStep *= -1;
            }

            *step = newStep;
            break;
        case cw:
            newDc += *step;
            *dc = CLAMP(MIN_DC_PERCENT, newDc, MAX_DC_PERCENT);
            break;
        case ccw:
            newDc -= *step;
            *dc = CLAMP(MIN_DC_PERCENT, newDc, MAX_DC_PERCENT);
            break;
        default:
            break;
    }
}

void update_deltaTime(const KnobStates q, int32_t *step, uint32_t *dt){
    //dt is left as a uint32 because it will act as the argument for the delay
    int32_t newStep = *step;
    int32_t newDT = (int32_t)*dt; //to allow for the newStep to subtract the newDT

    switch(q){
        case idle:
        case invalid:
            break;
        case pressed:
            newStep += DT_STEP;
            newStep = CLAMP(MIN_DT_STEP, newStep, MAX_DT_STEP);

            if((newStep == MIN_DT_STEP) || (newStep == MAX_DT_STEP)){
                newStep *= -1;
            }

            *step = newStep;
            break;
        case cw:
            newDT += newStep;
            *dt = (uint32_t)CLAMP(MIN_DT_US, (uint32_t)newDT, MAX_DT_US);
            break;
        case ccw:
            newDT -= newStep;
            *dt = (uint32_t)CLAMP(MIN_DT_US, (uint32_t)newDT, MAX_DT_US);
            break;
        default:
            break;
    }
}


int main(void){
    //initializations
    RCC->APB1ENR1 |= RCC_APB1ENR1_TIM6EN; //planning to use tim6 as a basic timer
    init_tim(TIM6);

    init_lpuart();
    init_GPIOPins();

    PWM_T pwm = {
        .freq = 1000,
        .dc = 50,
        .deltaTime = MIN_DT_US,
    };

    //set up the knob machines
    KnobMachine freqKnob = {0}; //this knob determines the frequency of the PWM
    KnobMachine dcKnob = {0}; //this knob determines the target duty cycle of the pwm
    KnobMachine dtKnob = {0}; //this determines the delay between each loop

    //km1 sits on gpioc
    freqKnob.pinMasks |= (
        SET_PIN(9, S1_POS) | 
        SET_PIN(8, S2_POS) | 
        SET_PIN(6, KEY_POS) 
    );

    //km2 sits on gpioa
    dcKnob.pinMasks |= (
        SET_PIN(12, S1_POS) | 
        SET_PIN(11, S2_POS) | 
        SET_PIN(9, KEY_POS) 
    );

    //km3 sits on gpioc
    dtKnob.pinMasks |= (
        SET_PIN(11, S1_POS) | 
        SET_PIN(10, S2_POS) | 
        SET_PIN(12, KEY_POS) 
    );

    float freqStep = FREQ_STEP;
    float dcStep = DC_STEP;
    //the dtStep is a uint32 since the argument for the delay functions only take in a uint32
    int32_t dtStep = DT_STEP;

    while(1){
        const uint32_t a_idr = ~GPIOA->IDR;
        const uint32_t c_idr = ~GPIOC->IDR;

        //get the state of each knob
        update_km_state(&freqKnob, c_idr);
        update_km_state(&dcKnob, a_idr);
        update_km_state(&dtKnob, c_idr);

        //continue if all the knobs are in an idle/invalid state
        if(
            ((freqKnob.q == idle) || (freqKnob.q == invalid)) &&
            ((dcKnob.q == idle) || (dcKnob.q == invalid)) &&
            ((dtKnob.q == idle) || (dtKnob.q == invalid)) 
        ){
            continue;
        }

        //handle each state
        update_frequency(freqKnob.q, &freqStep, &pwm.freq);
        update_dutyCycle(dcKnob.q, &dcStep, &pwm.dc);
        update_deltaTime(dtKnob.q, &dtStep, &pwm.deltaTime);

        //transmit the information of the pwm
        client_transmit(TO_BYTE_ARRAY(pwm), sizeof(pwm));

        delay_us(TIM6, 2000);
    }
    return 0;
}