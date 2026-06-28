#ifndef USB_MESSAGE_HANDLER_H
#define USB_MESSAGE_HANDLER_H

#include "string.h"
#include "main.h"
#include "commandParser.h"
#include "cmsis_os.h"
#include "stdio.h"
#include "globals.h"
#define CMD_USB_BUFFER_LENGTH 256


extern volatile uint8_t _USB_message_was_received_flag;
extern volatile uint8_t USB_message_buffer[CMD_USB_BUFFER_LENGTH];
extern volatile uint32_t USB_last_char_pointer;
extern volatile uint32_t USB_message_length;
void parse_usb_message(void);


#endif