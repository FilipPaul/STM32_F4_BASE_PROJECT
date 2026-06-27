#ifndef SHIFT_REGISTERS_H
#define SHIFT_REGISTERS_H

#include "main.h"
#include "stdint.h"
#include "globals.h"

#define REGISTER_ENDIANITY_1_PIN_IS_MSB 0
#define REGISTER_ENDIANITY_1_PIN_IS_LSB 1

typedef struct ShiftRegisters{
  uint16_t ser_pin;
  uint16_t srclk_pin;
  uint16_t rclk_pin;
  uint16_t srclr_pin;
  uint16_t noe_pin;
  GPIO_TypeDef* ser_port;
  GPIO_TypeDef* srclk_port;
  GPIO_TypeDef* rclk_port ;
  GPIO_TypeDef* srclr_port;
  GPIO_TypeDef* noe_port;
  uint8_t shift_reg_length;
  uint64_t current_satus;
  uint8_t endianity;
} ShifRegister;

uint8_t loadDataToShiftReg(uint64_t data, ShifRegister* shiftRegister);
uint8_t shiftRegSetSinglePin(uint8_t pin_number, uint8_t value, ShifRegister* shiftRegister);
uint8_t shiftRegNtimesShift(uint8_t number_of_shifts, ShifRegister* shiftRegister);
uint8_t clearRegister(ShifRegister* shiftRegister);

#endif