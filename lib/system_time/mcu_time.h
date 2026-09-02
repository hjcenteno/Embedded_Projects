/*
    author: Henry Centeno
    Description: Sets up a systems clock with the RTC to get a more accurate read on time
*/

#ifndef MCU_TIME_H
#define MCU_TIME_H

#include "stm32g474xx.h"
#include <inttypes.h>
#include <stdbool.h>

void init_mcu_time(void); //sets up the system clock for the mcu

#endif MCU_TIME_H