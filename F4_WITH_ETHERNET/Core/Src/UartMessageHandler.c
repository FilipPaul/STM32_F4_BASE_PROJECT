#include "UartMessageHandler.h"

static void uart_handle_message(char* message, uartHandlerStruct* uartHandlerStruct){
	if (uartHandlerStruct->connected_as_bridge){
		//forward message to the USB AS IN RAW FORMAT without parsing it as command
		printf("%s\n", message);
	}

	else
	{
		//Add prefix to USB:
		printf("%s: %s\n", uartHandlerStruct->uart_name, message);
	}
}

static void uart_byte_avaiable(uint8_t byte, uartHandlerStruct* uartHandlerStruct)
{
	if ((byte >= 32U) && (byte <= 127U) && (uartHandlerStruct->line_buffer_length < (CMD_UART_BUFFER_LENGTH - 1U))) {
		uartHandlerStruct->line_buffer[uartHandlerStruct->line_buffer_length++] = (char)byte;
	}

	if (((byte == '\n') || (byte == '\r')) && (uartHandlerStruct->line_buffer_length > 0U)){
		uartHandlerStruct->line_buffer[uartHandlerStruct->line_buffer_length] = '\0';
		uart_handle_message(uartHandlerStruct->line_buffer, uartHandlerStruct);
		uartHandlerStruct->line_buffer_length = 0U;
	}
}


void parse_uart_message(uartHandlerStruct* uartHandlerStruct){
	uint16_t bytes_to_process;
	uint16_t read_position;

	if(!uartHandlerStruct->message_was_received_flag){
		return;
	}

	__disable_irq();
	bytes_to_process = uartHandlerStruct->dma_pending_bytes;
	uartHandlerStruct->dma_pending_bytes = 0U;
	uartHandlerStruct->message_length = bytes_to_process;
	uartHandlerStruct->message_was_received_flag = 0U;
	__enable_irq();

	read_position = (uint16_t)uartHandlerStruct->last_char_pointer;
	while (bytes_to_process > 0U)
	{
		uart_byte_avaiable(uartHandlerStruct->DMA_RX_buffer[read_position], uartHandlerStruct);
		read_position++;
		bytes_to_process--;
		if (read_position >= uartHandlerStruct->DMA_BUFFER_SIZE)
		{
			read_position = 0U;
		}
	}

	uartHandlerStruct->last_char_pointer = read_position;
}