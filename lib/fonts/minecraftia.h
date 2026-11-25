/*
    Author: Henry Centeno
    Description:
        Courtesy of Andrew Tyler for creating the font. This file defines
        the bitmap for the display. For getting the values,  I did use claude to 
        convert the ttf file into a bitmap

    Source:
        *https://www.dafont.com/minecraftia.font
*/

#ifndef MINECRAFTIA_H
#define MINECRAFTIA_H

#include <stdint.h>

#define MC_NUM_CHARACTERS (95u)
#define MC_CHAR_HEIGHT (9u)

//this defines the pixel width of each character
extern const uint8_t minecraftia_width[MC_NUM_CHARACTERS];

//defines the bitmap of each character
extern const uint8_t minecraftia_font[MC_NUM_CHARACTERS][MC_CHAR_HEIGHT];

#endif