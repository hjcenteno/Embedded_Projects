/*
    Author: Henry Centeno
    Description:
        Defines the common includes throughout the project to avoid having to type them all out again and again.
*/

#ifndef COMMON_INCLUDES_H
#define COMMON_INCLUDES_H

#include "stm32g474xx.h"
#include <inttypes.h>
#include <stdbool.h>
#include <stdlib.h>

#define TO_BYTE_ARRAY(X) ((uint8_t *)&X) //convert x into an array of bytes
#define PTR_TO_BYTES(X) ((uint8_t *)X) //read what x points to as an array of bytes

#endif