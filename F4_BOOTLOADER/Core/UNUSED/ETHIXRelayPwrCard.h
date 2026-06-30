#ifndef ETH_IX_RELAY_PWR_CARD_H
#define ETH_IX_RELAY_PWR_CARD_H

#include "main.h"
#include "stdio.h"
#include "globals.h"


// CONTROL SHIFT REGISTER
#define PWR_CARD_CTR_MUX_A2 0 //QH at register
#define PWR_CARD_CTR_MUX_A1 1
#define PWR_CARD_CTR_MUX_A0 2
#define PWR_CARD_CTR_SHFT_SER 3
#define PWR_CARD_CTR_SHFT_nSRCLR 4
#define PWR_CARD_CTR_SHFT_SRCLK 5
#define PWR_CARD_CTR_SHFT_nOE 6
#define PWR_CARD_CTR_SHFT_RCLK 7 //QA at register


// PWR CARD SHIFT REGISTER
#define PWR_CARD_CH1_POLARIY 23
#define PWR_CARD_CH2_INST_COM 22
#define PWR_CARD_CH2_AMETER 21
#define PWR_CARD_CH2_VMETER 20
#define PWR_CARD_CH1_AMETER 19
#define PWR_CARD_CH1_CONNECT 18
#define PWR_CARD_CH1_VMETER 17
#define PWR_CARD_CH1_INST_COM 16

#define PWR_CARD_CH2_POLARITY 15
#define PWR_CARD_CH4_AMETER 14
#define PWR_CARD_CH3_POLARITY 13
#define PWR_CARD_CH3_CONNECT 12
#define PWR_CARD_CH3_INST_COM 11
#define PWR_CARD_CH3_AMETER 10
#define PWR_CARD_CH2_CONNECT 9
#define PWR_CARD_CH3_VMETER 8

//NOT CONNECTED PINS 4 5 6 7
#define PWR_CARD_CH4_CONNECT 3
#define PWR_CARD_CH4_INST_COM 2
#define PWR_CARD_CH4_VMETER 1
#define PWR_CARD_CH4_POLARITY 0



typedef struct
{
  // pins of the registers .. corresponds to the pins at ETHISX CONNECTOR
  uint16_t SHFT_SER_pin;
  uint16_t SHFT_SRCLK_pin;
  uint16_t SHFT_RCLK_pin;
  uint16_t SHFT_nSRCLR_pin;
  uint16_t SHFT_nOE_pin;
  GPIO_TypeDef* SHFT_SER_port;
  GPIO_TypeDef* SHFT_SRCLK_port;
  GPIO_TypeDef* SHFT_RCLK_port ;
  GPIO_TypeDef* SHFT_nSRCLR_port;
  GPIO_TypeDef* SHFT_nOE_port;
  
  uint16_t MUX_DATA_1_pin;
  uint16_t MUX_DATA_2_pin;
  uint16_t MUX_DATA_3_pin;
  GPIO_TypeDef* MUX_DATA_1_port;
  GPIO_TypeDef* MUX_DATA_2_port;
  GPIO_TypeDef* MUX_DATA_3_port ;
  uint8_t EXT_PORT_NUMBER;
  
  
  uint32_t last_Card_register_value;
  uint8_t last_control_register_value;
  uint32_t last_MUXes_readout_value;

  //RowPinStatus pins[NUMBER_OF_PINS_IN_ROW];
}ETHIXRelayPwrCard;

uint8_t setControlRegisterValue(ETHIXRelayPwrCard* card, uint8_t value);

//pin = 0-7 PWR_CARD_CTR_MUX_A2|PWR_CARD_CTR_MUX_A1|PWR_CARD_CTR_MUX_A0|
//PWR_CARD_CTR_SHFT_SER|PWR_CARD_CTR_SHFT_nSRCLR|PWR_CARD_CTR_SHFT_SRCLK|
//PWR_CARD_CTR_SHFT_nOE|PWR_CARD_CTR_SHFT_RCLK

// value = 0 or 1
uint8_t setPowerCardRegisterValue(ETHIXRelayPwrCard* card, uint32_t value);
uint8_t initPowerCard(ETHIXRelayPwrCard* card);
uint8_t readPowrCardMuxValue(ETHIXRelayPwrCard* card);


#endif