/*
    Author: Henry Centeno
    Description:
       Refer to the readme
                    
*/

#include "stm32g474xx.h"
#include <inttypes.h>
#include "stdio.h"

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
int server_init_lpuart();
int client_init_lpuart();

//tx, rx for the client and server
void client_transmit(); //transmits to the server's rx
void server_transmit(); //transmits to the client's rx
void client_read(); //reads the server's rx
void server_read(); //reads the client's rx