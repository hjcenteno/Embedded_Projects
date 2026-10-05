/*
    Author: Henry Centeno
    Description:
        This is will be obvious fault code to see if the ci will fail at this file
*/

#include "common_includes/common_includes.h"

int main(void){
    uint8_t *badPtr = NULL;
    *badPtr = 5; //null dereferencing, big no no

    int buffer[4];
    buffer[4] = 0; //out-of-bounds write, also big no no
}