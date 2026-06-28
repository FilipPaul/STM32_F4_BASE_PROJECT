#ifndef MULTIPLEXERS_H
#define MULTIPLEXERS_H

#include "main.h"
#include "stdint.h"
#include "globals.h"

typedef struct Multiplexers{
  uint16_t A0_pin;
  uint16_t A1_pin;
  uint16_t A2_pin;
  uint16_t data_out_pins[10];
  uint8_t mux_length;
  uint8_t number_of_paralel_muxes;
  uint16_t noe_pin;
  GPIO_TypeDef* A0_port;
  GPIO_TypeDef* A1_port;
  GPIO_TypeDef* A2_port ;
  GPIO_TypeDef* data_out_ports[10];
  GPIO_TypeDef* noe_port;
  uint64_t current_satus;
  uint8_t endianity;
} Multiplexer;

GPIO_PinState MUXreadAdress(uint8_t address, Multiplexer* mux);
uint64_t MUXreadAll(Multiplexer* mux);
#endif