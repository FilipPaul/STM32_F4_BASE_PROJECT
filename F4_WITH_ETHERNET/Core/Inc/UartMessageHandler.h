#ifndef UART_MESSAGE_HANDLER_H
#define UART_MESSAGE_HANDLER_H

#include "string.h"
#include "main.h"
#include "commandParser.h"
#include "stdio.h"
#include "globals.h"
#define CMD_UART_BUFFER_LENGTH 2048


typedef struct uartHandlerStruct{
    volatile uint8_t message_was_received_flag;
    uint8_t DMA_RX_buffer[CMD_UART_BUFFER_LENGTH];
    volatile uint32_t last_char_pointer;
    volatile uint32_t message_length;
    volatile uint16_t dma_write_position;
    volatile uint16_t dma_pending_bytes;
    char line_buffer[CMD_UART_BUFFER_LENGTH];
    uint16_t line_buffer_length;
    uint32_t DMA_BUFFER_SIZE;
    //UART HANDLE
    UART_HandleTypeDef* huart;
    char* uart_name; //for debug purposes only, not used for any logic
    uint8_t connected_as_bridge; //if this UART is used as bridge between two other UARTs, this flag is set to true, otherwise false. If true, the message received on this UART will be forwarded to the other UART instead of being parsed as command. This is used for example for RS232-1 and RS232-2, which are connected to each other via a transceiver and are used as bridge between the PC and the devices connected to the RS232 ports
} uartHandlerStruct;


void parse_uart_message(uartHandlerStruct* huart);

#endif