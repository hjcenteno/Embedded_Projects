/*
    Author: Henry Centeno
    Description:
        The user presses a button and the corresponding led is lit up.
        Then through the lpuart, the computer is told which led is lit up by which button.

    Components: [Component | Pin | Peripheral]:
    *button 1 | d0  | p  
    *button 2 | d1  | p  
    *button 3 | d2  | p  
    *LED 1    | d4  | p 
    *LED 2    | d5  | p  
    *LED 3    | d6  | p  
*/

#include "lib/uart_driver/lpuart_driver.h"
