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

#include "uart_driver/lpuart_driver.h"
#include "system_time/timx_utils.h"
#include "devices/knobs.h"

int main(void){
    //initializations
    //planning to use tim6 as a basic timer
    return 0;
}