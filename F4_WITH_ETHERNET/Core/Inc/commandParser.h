#ifndef COMMAND_PARSER_H
#define COMMAND_PARSER_H

#include "stdio.h"
#include "stdint.h"
#define MAXIMUM_SUBCOMMAND_LENGTH 45
#define MAXIMUM_COMMAND_LENGTH 64
#define MAXIMUM_NUMBER_OF_SUBCOMMANDS 10
#define MAXIMUM_RESPONSE_LENGTH 310
#define DELIMETER " "

enum CommandParser_source_of_commands{
	TCP_THREAD,
	USB_HANDLER,
	RS232_HANDLER
};

typedef struct
{
	char command[MAXIMUM_COMMAND_LENGTH];
	uint8_t number_of_subcommands;
	char subcommands[MAXIMUM_NUMBER_OF_SUBCOMMANDS][MAXIMUM_SUBCOMMAND_LENGTH];
	char* delimeter;
    char response[MAXIMUM_RESPONSE_LENGTH];
	enum CommandParser_source_of_commands source;
	struct netconn *TCP_client_netconn; //used to send response back to the client in case command was received via TCP

}commandTemplate;

uint8_t updateCommandStruct(char* command_str, commandTemplate* current_command, enum CommandParser_source_of_commands source);
uint8_t runCommand(commandTemplate* current_command);
#endif