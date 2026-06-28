#ifndef RELAYCARDS_H
#define RELAYCARDS_H

#include "main.h"
#include "stdint.h"
#include "globals.h"
#include "shiftRegisters.h"

typedef struct ExternalPortMapping{
    uint16_t EXT_PIN_1_pin; //PIN 1 AT EXT CONNECTOR
    uint16_t EXT_PIN_2_pin; //PIN 2 AT EXT CONNECTOR
    uint16_t EXT_PIN_3_pin; //PIN 3 AT EXT CONNECTOR
    uint16_t EXT_PIN_4_pin; //PIN 4 AT EXT CONNECTOR
    uint16_t EXT_PIN_5_pin; //PIN 5 AT EXT CONNECTOR
    uint16_t EXT_PIN_6_pin; //PIN 6 AT EXT CONNECTOR
    uint16_t EXT_PIN_7_pin; //PIN 7 AT EXT CONNECTOR
    uint16_t EXT_PIN_8_pin; //PIN 8 AT EXT CONNECTOR
    uint16_t EXT_OE_pin; //OE PIN AT EXT CONNECTOR


    GPIO_TypeDef* EXT_PIN_1_port; //PORT 1 AT EXT CONNECTOR
    GPIO_TypeDef* EXT_PIN_2_port; //PORT 2 AT EXT CONNECTOR
    GPIO_TypeDef* EXT_PIN_3_port; //PORT 3 AT EXT CONNECTOR
    GPIO_TypeDef* EXT_PIN_4_port; //PORT 4 AT EXT CONNECTOR
    GPIO_TypeDef* EXT_PIN_5_port; //PORT 5 AT EXT CONNECTOR
    GPIO_TypeDef* EXT_PIN_6_port; //PORT 6 AT EXT CONNECTOR
    GPIO_TypeDef* EXT_PIN_7_port; //PORT 7 AT EXT CONNECTOR
    GPIO_TypeDef* EXT_PIN_8_port; //PORT 8 AT EXT CONNECTOR
    GPIO_TypeDef* EXT_OE_port; //OE PORT AT EXT CONNECTOR
} ExternalPortMapping;


typedef struct ExternalRelayCard{
    uint16_t pin_E3;
    uint16_t pin_A;
    uint16_t pin_B;
    uint16_t pin_C;
    uint16_t pin_D1;
    uint16_t pin_D2;
    uint16_t pin_D3;
    uint16_t pin_D4;


    GPIO_TypeDef* pin_E3_port; //PORT 4 AT EXT CONNECTOR
    GPIO_TypeDef* pin_A_port; //PORT 1 AT EXT CONNECTOR
    GPIO_TypeDef* pin_B_port; //PORT 2 AT EXT CONNECTOR
    GPIO_TypeDef* pin_C_port; //PORT 3 AT EXT CONNECTOR
    GPIO_TypeDef* pin_D1_port; //PORT 5 AT EXT CONNECTOR
    GPIO_TypeDef* pin_D2_port; //PORT 6 AT EXT CONNECTOR
    GPIO_TypeDef* pin_D3_port; //PORT 7 AT EXT CONNECTOR
    GPIO_TypeDef* pin_D4_port; //PORT 8 AT EXT CONNECTOR

    uint32_t current_satus;
    uint8_t number_of_relays;
} ExternalRelayCard;

uint8_t relayCardSetSingleRelay(uint8_t relay_number, uint8_t relay_state, ExternalRelayCard* RelayCard); //returns new relay state
uint8_t relayCardSetAllRelays(uint8_t state, ExternalRelayCard* RelayCard); //returns new port state
uint8_t relayCardSetPort(uint32_t value, ExternalRelayCard* RelayCard);
uint8_t _relaySetSingleRelayFor32Cards(uint8_t relay_number, uint8_t relay_state, ExternalRelayCard* RelayCard);
uint8_t _setSingleRelayFor8Cards(uint8_t relay_number, uint8_t relay_state, ExternalRelayCard* RelayCard);
#endif