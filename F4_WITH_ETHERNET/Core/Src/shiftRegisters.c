#include "shiftRegisters.h"
#include "stdio.h"



uint8_t loadDataToShiftReg(uint64_t data, ShifRegister* shiftRegister)
{
  //printf("LOADING DATA: %d TO SHIFT REG\n", data);
  /**/
  //printf("SER_PIN: %d\n", shiftRegister->ser_pin);

  //RESET SHIFT REGISTER
  HAL_GPIO_WritePin(shiftRegister->srclr_port, shiftRegister->srclr_pin,GPIO_PIN_RESET);//RESET SHIFT REGISTER
  _2500_ns_delay(DELAY_MULTIPLES_OF_2500_NS);
  HAL_GPIO_WritePin(shiftRegister->srclr_port, shiftRegister->srclr_pin,GPIO_PIN_SET);//RESET SHIFT REGISTER
  HAL_GPIO_WritePin(shiftRegister->noe_port, shiftRegister->noe_pin, GPIO_PIN_RESET); //ENABLE OUTPUT
  _2500_ns_delay(DELAY_MULTIPLES_OF_2500_NS);

  for (int i = 0; i < shiftRegister->shift_reg_length; i++){
    
    if (shiftRegister->endianity == REGISTER_ENDIANITY_1_PIN_IS_MSB){
      HAL_GPIO_WritePin(shiftRegister->ser_port, shiftRegister->ser_pin, ((data >> (shiftRegister->shift_reg_length - 1 - i)) & ((uint64_t)(0x01))) ); //LOAD DATA
    }
    else{
      HAL_GPIO_WritePin(shiftRegister->ser_port, shiftRegister->ser_pin, ((data >> i) & ((uint64_t)(0x01))) ); //LOAD DATA
    }
    //HAL_GPIO_WritePin(shiftRegister->ser_port, shiftRegister->ser_pin, ((data >> i) & ((uint64_t)(0x01))) ); //LOAD DATA
    _2500_ns_delay(DELAY_MULTIPLES_OF_2500_NS);
    DO_CLK_PULSE(shiftRegister->srclk_port, shiftRegister->srclk_pin); //CLK PULSE
    _2500_ns_delay(DELAY_MULTIPLES_OF_2500_NS);
    
  }


  DO_CLK_PULSE(shiftRegister->rclk_port, shiftRegister->rclk_pin); //OUTPUT DATA
  shiftRegister->current_satus = data;
  
  //printf("TASK ENDED: %lu%lu\n", (uint32_t)(data / 4294967296UL), (uint32_t)(data % 4294967296UL));
  return 1;

}

uint8_t clearRegister(ShifRegister* shiftRegister){
  DO_CLK_PULSE(shiftRegister->srclr_port, shiftRegister->srclr_pin); //CLK PULSE
  DO_CLK_PULSE(shiftRegister->rclk_port, shiftRegister->rclk_pin); //OUTPUT DATA
  return 1;
}

uint8_t shiftRegNtimesShift(uint8_t number_of_shifts, ShifRegister* shiftRegister){
    HAL_GPIO_WritePin(shiftRegister->ser_port, shiftRegister->ser_pin,GPIO_PIN_RESET);
    for( uint8_t i = 0; i < number_of_shifts; i++){
      DO_CLK_PULSE(shiftRegister->srclk_port, shiftRegister->srclk_pin); //CLK PULSE
    }

    DO_CLK_PULSE(shiftRegister->rclk_port, shiftRegister->rclk_pin); //OUTPUT DATA
    shiftRegister->current_satus = shiftRegister->current_satus >> number_of_shifts;
    return 1;
}

uint8_t shiftRegSetSinglePin(uint8_t pin_number, uint8_t value, ShifRegister* shiftRegister){
  static uint64_t data;
  if (value == 0){
    data = shiftRegister->current_satus & (~(1ULL << pin_number));
  }
  else{
    data = shiftRegister->current_satus | (1ULL << pin_number);
  }
  loadDataToShiftReg(data, shiftRegister);
  return 1;
}