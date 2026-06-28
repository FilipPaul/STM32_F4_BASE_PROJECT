#ifndef EEPROM_SETTINGS_H
#define EEPROM_SETTINGS_H
#include "main.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "main.h"
#include "globals.h"
#include "commandParser.h"
extern I2C_HandleTypeDef hi2c2;

#define offset 0

#define EEPROM_I2C_ADDR_FOR_MAC 0b10110001
#define EEPROM_I2C_STARTADDR_OF_MAC 0x98 //binary 0b10011010
#define EEPROM_I2C_MAC_ADDR_LEN 8

#define EEPROM_I2C_ADDR_FOR_WRITING_CUSTOM_VALUES 0b10100000
#define EEPROM_I2C_ADDR_FOR_READING_CUSTOM_VALUES 0b10100001

#define EEPROM_FLAG_ADDR_IP 0x20 + offset
#define EEPROM_FLAG_ADDR_SUBNET 0x21 + offset
#define EEPROM_FLAG_ADDR_GATEWAY 0x22 + offset
#define EEPROM_FLAG_ADDR_DNS 0x23 + offset
#define EEPROM_FLAG_ADDR_DHCP 0x24 + offset
#define EEPROM_FLAG_ADDR_MAC 0x25 + offset

#define CUSTOM_FLAG_IS_SET_VALUE 0x38 //just a random value, that indicates that the flag is set

#define EEPROM_CUSTOM_ADDR_OF_IP 0x00 + offset
#define EEPROM_CUSTOM_ADDR_OF_SUBNET 0x04 + offset
#define EEPROM_CUSTOM_ADDR_OF_GATEWAY 0x08 + offset
#define EEPROM_CUSTOM_ADDR_OF_DNS 0x0C + offset
#define EEPROM_CUSTOM_ADDR_OF_DHCP 0x10 + offset
#define EEPROM_CUSTOM_ADDR_OF_MAC 0x14 + offset

#define EEPROM_TEMPERATURE_OFFSET 0x40 + offset
#define EEPROM_TEMPERATURE_OFFSET_FLAG 0x41 + offset

int8_t EEPROMsetFlag(uint16_t flagAddress, uint8_t flagValue);
int8_t EEPROMCheckFlag(uint16_t flagAddress);

int8_t EEPROMgetDefaultMacAddress(uint8_t macAddress[6]);
int8_t EEPROMgetCustomIPAddress(uint8_t ipAddress[4]);
int8_t EEPROMgetCustomSubnet(uint8_t subnet[4]);
int8_t EEPROMgetCustomGateway(uint8_t gateway[4]);
int8_t EEPROMgetCustomMac(uint8_t mac[6]);

int8_t EEPROMWriteCustomIP(uint8_t ipAddress[4]);
int8_t EEPROMWriteCustomSubnet(uint8_t subnet[4]);
int8_t EEPROMWriteCustomGateway(uint8_t gateway[4]);
int8_t EEPROMWriteCustomMac(uint8_t mac[6]);

void EEPROMInputsParser(commandTemplate* current_command);

int8_t EEPROMWriteCustomTemperatureOffset(int8_t temperature_offset[1]);
int8_t EEPROMgetCustomTemperatureOffset(uint8_t temperature_offset[1]);
int8_t EEPROMEraseAll();





#endif // 80PIN_MODULE_CARD_H

