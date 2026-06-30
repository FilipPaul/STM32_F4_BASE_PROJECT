#include "UsbMessageHanler.h"


volatile uint8_t _USB_message_was_received_flag = 0;
volatile uint8_t USB_message_buffer[CMD_USB_BUFFER_LENGTH];
volatile uint32_t USB_message_length = 0;
volatile uint32_t USB_last_char_pointer;
static commandTemplate usb_current_command;

static void usb_parse_command(char * command){
	//char buffer[1024];
	//sprintf(buffer,"%s",command); // coppy data to the buffer
	//Take commandTemplate mutex
	xSemaphoreTake(commandTemplateMutex, portMAX_DELAY);
	if (updateCommandStruct(command, &usb_current_command, USB_HANDLER) == 0)
	{
		xSemaphoreGive(commandTemplateMutex);
		printf("ERROR;Command parse failed or payload too long\n");
		return;
	}
	usb_current_command.TCP_client_netconn = NULL; //not used for USB commands, only for TCP commands
	//Put commandTemplate into queue to be processed by main loop
	xQueueSend(commandQueueHandle, &usb_current_command, 0);
	xSemaphoreGive(commandTemplateMutex);
	//runCommand(&current_command);
	//printf("%s",current_command.response);
}

static void usb_byte_avaiable(uint8_t byte)
{
	static uint16_t bfr_pointer;
	static char data[CMD_USB_BUFFER_LENGTH];

	if(bfr_pointer < CMD_USB_BUFFER_LENGTH && byte >= 32 && byte <=127) data[bfr_pointer++] = byte; //
	if ( (byte == '\n' || byte == '\r') && bfr_pointer > 0){
		data[bfr_pointer] = '\0'; //add null terminator -> make string from bytearray
		//printf("Received: %s\n", data);
		usb_parse_command(data);
		bfr_pointer = 0; //reset buffer pointer
	}
}

void parse_usb_message(void){
	uint32_t bytes_to_process;
	uint8_t process_buffer[CMD_USB_BUFFER_LENGTH];

    if(_USB_message_was_received_flag){
		__disable_irq();
		bytes_to_process = USB_message_length;
		if (bytes_to_process > CMD_USB_BUFFER_LENGTH)
        {
			bytes_to_process = CMD_USB_BUFFER_LENGTH;
        }
		memcpy(process_buffer, (const void *)USB_message_buffer, bytes_to_process);
		USB_message_length = 0;
		_USB_message_was_received_flag = 0;
		__enable_irq();

		USB_last_char_pointer = 0;
		while (USB_last_char_pointer < bytes_to_process)
		{
			usb_byte_avaiable(process_buffer[USB_last_char_pointer++]);
		}
    }
}
