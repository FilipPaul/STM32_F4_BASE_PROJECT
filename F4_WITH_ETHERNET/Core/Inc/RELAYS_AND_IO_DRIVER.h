#ifndef RELAY_AND_IO_DRIVER_H
#define RELAY_AND_IO_DRIVER_H
#include "main.h"
#include "shiftRegisters.h"
#include "multiplexers.h"


typedef struct RELAY_IO_DRIVER_STRUCT {
  struct ShiftRegisters* RELAY_SHIFT_REGISTER;
  struct ShiftRegisters* VENTIL_SHIFT_REGISTER;
  struct Multiplexers* INPUT_MUX;
} RELAY_IO_DRIVER_STRUCT;

uint8_t RELAY_DRIVER_init(struct RELAY_IO_DRIVER_STRUCT* RELAY_IO_DRIVER);
uint8_t RELAY_DRIVER_setRelay(uint8_t relay_number, uint8_t state, struct RELAY_IO_DRIVER_STRUCT* RELAY_IO_DRIVER);
uint8_t RELAY_DRIVER_setRelayPort(uint32_t value, struct RELAY_IO_DRIVER_STRUCT* RELAY_IO_DRIVER);
uint8_t RELAY_DRIVER_setVentil(uint8_t ventil_number, uint8_t state, struct RELAY_IO_DRIVER_STRUCT* RELAY_IO_DRIVER);
uint8_t RELAY_DRIVER_setVentilPort(uint32_t value, struct RELAY_IO_DRIVER_STRUCT* RELAY_IO_DRIVER);
uint8_t RELAY_DRIVER_readInput(uint8_t input_number, struct RELAY_IO_DRIVER_STRUCT* RELAY_IO_DRIVER);
uint64_t RELAY_DRIVER_readInputsPort(struct RELAY_IO_DRIVER_STRUCT* RELAY_IO_DRIVER);

#endif