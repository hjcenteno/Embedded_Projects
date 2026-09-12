/*
    Author: Henry Centeno
    Description: Sets up a general purpose timer
    *uses tim2_ch2 as pb3 is FT meaning it is 5V tolerant
*/

#ifndef MCU_TIME_H
#define MCU_TIME_H

#include "common_includes/common_includes.h"

#define PRESCALAR_VALUE_16MHZ 15 //The counter clock frequency tim_cnt_ck is equal to ftim_psc_ck / (PSC[15:0] + 1)
#define MAX_TIME 0xFFFFFFFF
#define MILLI_TO_MICRO 1000 //to convert between milli (10^-3) and micro (10^-6)
#define MILLI_TO_BASE 1000 //to convert between milli (10^-3) and base (10^0)
#define MICRO_TO_BASE 1000000 //to convert between base (10^0) and micro (10^-6)

void init_tim2(void); //sets up tim2 to act as the system clock for the mcu
uint32_t getTime(void); //gets the value of the cnt register of tim2

//delay is done in microseconds, so when adding new delay functions, convert the time to microseconds)
void delay_us(uint32_t microseconds); //amount of time to delay in microseconds
void delay_ms(uint32_t milliseconds); //amount of time to delay in milliseconds
void delay_s(uint32_t seconds); //amount of time to delay in seconds

#endif