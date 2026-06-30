#ifndef ANALOG_RESISTOR_FRONTEND_H
#define ANALOG_RESISTOR_FRONTEND_H

#include "main.h"
#include "stdint.h"
#include "globals.h"
#include "shiftRegisters.h"

typedef struct AD7124_8_DRIVER{
  uint16_t MISO_pin;
  uint16_t MOSI_pin;
  uint16_t SCK_pin;
  uint16_t CS_pin;
  uint16_t nSYNC_pin;
  GPIO_TypeDef* MISO_port;
  GPIO_TypeDef* MOSI_port;
  GPIO_TypeDef* SCK_port;
  GPIO_TypeDef* CS_port;
  GPIO_TypeDef* nSYNC_port;
} AD7124_8_DRIVER;


typedef struct ResistorMeasurementConfig{
  uint16_t RELAY_pins[8];
  GPIO_TypeDef* RELAY_ports[8];
  ShifRegister* shiftRegister;
  AD7124_8_DRIVER* AD7124_8_driver;

} ResistorMeasurementConfig;

uint8_t RESISTOR_DRIVER_init(ResistorMeasurementConfig* config);
int8_t RESISTOR_DRIVER_ADFS7124readFromRegister(uint32_t register_to_read, uint8_t expected_data_length,ResistorMeasurementConfig* config,uint32_t *readout_data);
int8_t RESISTOR_DRIVER_ADFS7124writeToRegister(uint32_t register_to_write, uint8_t register_lenght,uint32_t data, ResistorMeasurementConfig* config);
int8_t RESISTOR_DRIVER_setChannelEnableBit(uint8_t channel_number, uint8_t enable_bit, ResistorMeasurementConfig* config);
int8_t RESISTOR_DRIVER_selectChannel(uint8_t channel_number, ResistorMeasurementConfig* config);
int8_t RESISTOR_DRIVER_measureChannelADCValue(uint8_t channel_nr, ResistorMeasurementConfig* config, uint32_t* readout_data);

uint8_t RESISTOR_DRIVER_setOutputRelay(uint8_t relay_number, uint8_t relay_state, ResistorMeasurementConfig* config);
uint8_t RESISTOR_DRIVER_setCurrentLevel(uint8_t current_level, ResistorMeasurementConfig* config);
uint8_t RESISTOR_DRIVER_setCurrentSourceToPin(uint8_t pin_number, ResistorMeasurementConfig* config);
uint8_t RESISTOR_DRIVER_setReferenceSourceToExternal(ResistorMeasurementConfig* config);
uint8_t RESISTOR_DRIVER_setReferenceSourceToInternal(ResistorMeasurementConfig* config);
uint16_t RESISTOR_DRIVER_setPinGrounding(uint8_t pin_number,uint8_t grounding_option, ResistorMeasurementConfig* config);
#endif