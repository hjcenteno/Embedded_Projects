/*
    Author: Henry Centeno
    Description:
       Refer to the readme
                    
*/

#ifndef LPUART_DRIVER_H
#define LPUART_DRIVER_H

#include "stm32g474xx.h"
#include <inttypes.h>
#include <stdbool.h>
#include <stdlib.h>

#define MAX_RB_SIZE 255
#define MAX_RB_LENGTH 80
#define FRAME_LENGTH 10
#define lpuart_baud 9600
#define lse_hz 32768
#define start_byte 0xAA

typedef struct ringBuffer{
    uint8_t buffer[MAX_RB_SIZE]; //represents the raw bytes to be read
    uint8_t size; //go back to the beginning once size > MAX_RB_SIZE
} ringBuffer;

/*
initiates the communication between the MCU (client) to the laptop (server).
return 0 on success
*/
int init_lpuart(void);

//tx, rx for the client and server
void client_transmit(uint8_t *data, const uint8_t length); //transmits to the server's rx
void client_byte_transmit(uint8_t data);
void server_transmit(uint8_t *data, const uint8_t length); //transmits to the client's rx
void server_byte_transmit(uint8_t data);

//returns the number of bytes read
uint32_t client_read(void); //reads the server's rx
uint32_t server_read(void); //reads the client's rx

#endif