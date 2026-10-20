/*
    Author: Henry Centeno
    Description:
        This will expand on the mcu_time.h functions to be more agnostic to the timX/timX_ccrX configurations.
        This will let the caller decide which tim and channels to configure without having to write individually 
        to each register for each tim and etcera. Plus this will be a good practice for writing code in such a manner
        to be more flexible.
*/

#ifndef TIMX_UTILS_H
#define TIMX_UTILS_H

#include "common_includes/common_includes.h"

#define PRESCALAR_VALUE_16MHZ (15) //The counter clock frequency tim_cnt_ck is equal to ftim_psc_ck / (PSC[15:0] + 1)
#define PRESCALAR_VALUE_24MHZ (23) //The counter clock frequency tim_cnt_ck is equal to ftim_psc_ck / (PSC[15:0] + 1)
#define MAX_TIME (0xFFFFFFFF)
#define MILLI_TO_MICRO (1000u) //to convert from milli (10^-3) to micro (10^-6)
#define MILLI_TO_BASE (1 / 1000f) //to convert from milli (10^-3) to base (10^0)
#define MICRO_TO_BASE (1.0f / 1000000.0f) //to convert from micro (10^-6) to base (10^0)
#define BASE_TO_MICRO (1000000u) //to convert from base (10^0) to micro (10^-6)
#define BASE_TO_MILLI (1000u) //to convert from base (10^0) to milli (10^-3)

/*Peripheral bus check for the tim based on cmsisi_device_g4/include/stm32g4747xx.h */
#define IS_TIM_BUS_APB1(INSTANCE) (((INSTANCE) == TIM2) || \
                                   ((INSTANCE) == TIM3) || \
                                   ((INSTANCE) == TIM4) || \
                                   ((INSTANCE) == TIM5) || \
                                   ((INSTANCE) == TIM6) || \
                                   ((INSTANCE) == TIM7))

#define IS_TIM_BUS_APB2(INSTANCE) (((INSTANCE) == TIM1) || \
                                   ((INSTANCE) == TIM8) || \
                                   ((INSTANCE) == TIM15) || \
                                   ((INSTANCE) == TIM16) || \
                                   ((INSTANCE) == TIM17) || \
                                   ((INSTANCE) == TIM20))

//Check if the tim provided is advanced-control, general purpose, or basic
#define IS_TIM_AC(INSTANCE) (((INSTANCE) == TIM1) || \
                             ((INSTANCE) == TIM8) || \
                             ((INSTANCE) == TIM20))

#define IS_TIM_GP(INSTANCE) (((INSTANCE) == TIM2) || \
                             ((INSTANCE) == TIM3) || \
                             ((INSTANCE) == TIM4) || \
                             ((INSTANCE) == TIM5) || \
                             ((INSTANCE) == TIM15) || \
                             ((INSTANCE) == TIM16) || \
                             ((INSTANCE) == TIM17))
                             
#define IS_TIM_BASIC(INSTANCE) (((INSTANCE) == TIM6) || \
                                ((INSTANCE) == TIM7))


//activates the 24 MHz HSE as the system wide clock
int activate_SWS_RCC_HSE(void);

/* sets up timX to act as a basic clock
 * TIM_TypeDef *timX: the timX
 */
void init_tim(TIM_TypeDef *timX);
void init_basic_tim(TIM_TypeDef *timX); //sets up tim6/7 as a basic timer
void init_AC_tim(TIM_TypeDef *timX); //sets up the advance control timer: tim 1/8/20
void init_GP_tim(TIM_TypeDef *timX); //sets up the general purpose timer: tim 2/3/4/5/15/16/17
void init_tim_ccmr1(TIM_TypeDef *timX, const uint32_t channelMask, bool interruptEn); //initiates channels 1/2 for the tim, caller defines what they want with the mask
void init_tim_ccmr2(TIM_TypeDef *timX, const uint32_t channelMask, bool interruptEn); //initiates channels 3/4 for the tim, caller defines what they want with the mask
static uint32_t getTime(TIM_TypeDef *timX){ //gets the value of the cnt register of timX, 0 on error
    return (IS_TIM_INSTANCE(timX)) ? timX->CNT : 0;
}

//delay is done in microseconds, so when adding new delay functions, convert the time to microseconds)
void delay_us(TIM_TypeDef timX, uint32_t microseconds); //amount of time to delay in microseconds
void delay_ms(TIM_TypeDef timX, uint32_t milliseconds); //amount of time to delay in milliseconds
void delay_s(TIM_TypeDef timX, uint32_t seconds); //amount of time to delay in seconds


#endif