#include "Analog_resistor_frontend.h"
#include "main.h"
#include "commandParser.h"

extern SPI_HandleTypeDef hspi2;
extern char SPI2_BUFFER[24];
//ANALOG RESISTOR FRONTEND

struct AD7124_8_DRIVER AD7124_8_driver = {
  .MISO_pin = SPI2_MISO_Pin,
  .MOSI_pin = SPI2_MOSI_Pin,
  .SCK_pin = SPI2_SCK_Pin,
  .CS_pin = CTRL_21_Pin,
  .nSYNC_pin = CTRL_17_Pin,
  .MISO_port = SPI2_MISO_GPIO_Port,
  .MOSI_port = SPI2_MOSI_GPIO_Port,
  .SCK_port = SPI2_SCK_GPIO_Port,
  .CS_port = CTRL_21_GPIO_Port,
  .nSYNC_port = CTRL_17_GPIO_Port,
};
struct ShiftRegisters SHIFT_REG_RESISTOR_MEASUREMNTS  = {
  .ser_pin = CTRL_19_Pin,
  .srclk_pin = CTRL_24_Pin,
  .rclk_pin = CTRL_22_Pin,
  .srclr_pin = CTRL_23_Pin,
  .noe_pin = CTRL_20_Pin,
  .ser_port = CTRL_19_GPIO_Port,
  .srclk_port = CTRL_24_GPIO_Port,
  .rclk_port = CTRL_22_GPIO_Port,
  .srclr_port = CTRL_23_GPIO_Port,
  .noe_port = CTRL_20_GPIO_Port,
  .shift_reg_length = 16,
  .current_satus = 0,
  .endianity = REGISTER_ENDIANITY_1_PIN_IS_MSB,
};

struct ResistorMeasurementConfig RESISTOR_MEASUREMENT_CONFIG = {
  .RELAY_pins = {
    CTRL_10_Pin,CTRL_11_Pin,CTRL_12_Pin,CTRL_13_Pin,
    CTRL_16_Pin,CTRL_14_Pin,CTRL_15_Pin,CTRL_18_Pin
    },
  .RELAY_ports = {
    CTRL_10_GPIO_Port,CTRL_11_GPIO_Port,CTRL_12_GPIO_Port,CTRL_13_GPIO_Port,
    CTRL_16_GPIO_Port,CTRL_14_GPIO_Port,CTRL_15_GPIO_Port,CTRL_18_GPIO_Port
    },
  .AD7124_8_driver = &AD7124_8_driver,
  .shiftRegister = &SHIFT_REG_RESISTOR_MEASUREMNTS,
  
};


uint8_t RESISTOR_DRIVER_init(ResistorMeasurementConfig* config) {
  //SET ALL RELAY PINS TO PULL DOWN OUTPUTS
  for (int i = 0; i < 8; i++){
    //CREATE CONFIGURATION FOR RELAY PINS
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = config->RELAY_pins[i];
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_PULLDOWN;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_DeInit(config->RELAY_ports[i], config->RELAY_pins[i]);
    HAL_GPIO_Init(config->RELAY_ports[i], &GPIO_InitStruct);

    //PUT RELAY PINS TO RESET
    HAL_GPIO_WritePin(config->RELAY_ports[i], config->RELAY_pins[i], GPIO_PIN_RESET);

    //ADC DRIVER INITIALIZATION:
    //SET nCS to LOW
    HAL_GPIO_WritePin(config->AD7124_8_driver->CS_port, config->AD7124_8_driver->CS_pin, GPIO_PIN_RESET);
    _2500_ns_delay(5);
  }

  //NOW DO THE SAME FOR ALL SHIFT REGISTER PINS
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  GPIO_InitStruct.Pin = SHIFT_REG_RESISTOR_MEASUREMNTS.ser_pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_DeInit(SHIFT_REG_RESISTOR_MEASUREMNTS.ser_port, SHIFT_REG_RESISTOR_MEASUREMNTS.ser_pin);
  HAL_GPIO_Init(SHIFT_REG_RESISTOR_MEASUREMNTS.ser_port, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = SHIFT_REG_RESISTOR_MEASUREMNTS.srclk_pin;
  HAL_GPIO_DeInit(SHIFT_REG_RESISTOR_MEASUREMNTS.srclk_port, SHIFT_REG_RESISTOR_MEASUREMNTS.srclk_pin);
  HAL_GPIO_Init(SHIFT_REG_RESISTOR_MEASUREMNTS.srclk_port, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = SHIFT_REG_RESISTOR_MEASUREMNTS.rclk_pin;
  HAL_GPIO_DeInit(SHIFT_REG_RESISTOR_MEASUREMNTS.rclk_port, SHIFT_REG_RESISTOR_MEASUREMNTS.rclk_pin);
  HAL_GPIO_Init(SHIFT_REG_RESISTOR_MEASUREMNTS.rclk_port, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = SHIFT_REG_RESISTOR_MEASUREMNTS.srclr_pin;
  HAL_GPIO_DeInit(SHIFT_REG_RESISTOR_MEASUREMNTS.srclr_port, SHIFT_REG_RESISTOR_MEASUREMNTS.srclr_pin);
  HAL_GPIO_Init(SHIFT_REG_RESISTOR_MEASUREMNTS.srclr_port, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = SHIFT_REG_RESISTOR_MEASUREMNTS.noe_pin;
  HAL_GPIO_DeInit(SHIFT_REG_RESISTOR_MEASUREMNTS.noe_port, SHIFT_REG_RESISTOR_MEASUREMNTS.noe_pin);
  HAL_GPIO_Init(SHIFT_REG_RESISTOR_MEASUREMNTS.noe_port, &GPIO_InitStruct);

  //PUT ALL PINS TO RESET
  HAL_GPIO_WritePin(SHIFT_REG_RESISTOR_MEASUREMNTS.ser_port, SHIFT_REG_RESISTOR_MEASUREMNTS.ser_pin, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(SHIFT_REG_RESISTOR_MEASUREMNTS.srclk_port, SHIFT_REG_RESISTOR_MEASUREMNTS.srclk_pin, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(SHIFT_REG_RESISTOR_MEASUREMNTS.rclk_port, SHIFT_REG_RESISTOR_MEASUREMNTS.rclk_pin, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(SHIFT_REG_RESISTOR_MEASUREMNTS.srclr_port, SHIFT_REG_RESISTOR_MEASUREMNTS.srclr_pin, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(SHIFT_REG_RESISTOR_MEASUREMNTS.noe_port, SHIFT_REG_RESISTOR_MEASUREMNTS.noe_pin, GPIO_PIN_RESET);

  loadDataToShiftReg(0, &SHIFT_REG_RESISTOR_MEASUREMNTS);  
  return 0;
  }

uint8_t RESISTOR_DRIVER_setOutputRelay(uint8_t relay_number, uint8_t relay_state, ResistorMeasurementConfig* config){
  HAL_GPIO_WritePin(config->RELAY_ports[relay_number], config->RELAY_pins[relay_number],relay_state);
  return 0;
  } 

int8_t RESISTOR_DRIVER_ADFS7124readFromRegister(uint32_t register_to_read, uint8_t expected_data_length,ResistorMeasurementConfig* config,uint32_t *readout_data){

    //TO READ FROM REGISTER SEND FOLLOWING COMMAND TO THE ADFS7124:
    uint8_t payload_for_register_selection = 0b01000000 | register_to_read;
    HAL_StatusTypeDef status;
    status = HAL_SPI_Transmit(&hspi2, &payload_for_register_selection, 1, 1);
    _2500_ns_delay(40);
    if (status != HAL_OK){
        //sprintf(current_command->response,"ERROR;ADFS7124_READ_REGISTER FAILED TO SELECT REGISTER:%d with status:%d\n",register_to_read,status);
        return -1;//ERROR;ADFS7124_READ_REGISTER FAILED TO SELECT REGISTER
    }
    _2500_ns_delay(40);
    //NOW READ FROM REGISTER
    status = HAL_SPI_Receive(&hspi2, (uint8_t *)SPI2_BUFFER, expected_data_length, 3);
      _2500_ns_delay(40);
    if (status != HAL_OK){
        //sprintf(current_command->response,"ERROR;ADFS7124_READ_REGISTER FAILED TO READ DATA FROM REGISTER:%d with status:%d\n",register_to_read,status);
        
        return -2;
    }
    //CONVERT bytes of buffer into single uint32_t
    uint32_t data = 0;
    for (uint32_t i = 0; i < expected_data_length; i++){
        //printf("SPI2_BUFFER[%d]:%d\n",i,SPI2_BUFFER[i]);
        data = data | ((uint8_t)SPI2_BUFFER[expected_data_length-i-1] << (8*i));
    }
    *readout_data = data;
    //sprintf(current_command->response,"OK;ADFS7124_READ_REGISTER REGISTER:%d, VALUE:%lu\n",register_to_read,data);
    return 1;
  }

int8_t RESISTOR_DRIVER_ADFS7124writeToRegister(uint32_t register_to_write, uint8_t register_lenght,uint32_t data, ResistorMeasurementConfig* config){
    //ACCORDING TO THE DATA LENGHT CREATE PAYLOAD OF BYTE CHUNCS
    //FIRST BYTE IS MSB -> LAST BYTE IS LSB
    uint8_t payload[4];
    for (uint32_t i = 0; i < register_lenght; i++){
        payload[i] = (data >> (8*(register_lenght-i-1))) & 0xFF;
    }

    //TO WRITE INTO REGISTER FIRST SEND FOLLOWING COMMAND TO THE ADFS7124:
    uint8_t payload_for_register_selection = 0b00000000 | register_to_write;
    HAL_StatusTypeDef status;
    status = HAL_SPI_Transmit(&hspi2, &payload_for_register_selection, 1, 1);
    _2500_ns_delay(40);

    if (status != HAL_OK){
        //sprintf(current_command->response,"ERROR;ADFS7124_WRITE_TO_REGISTER FAILED TO SELECT REGISTER:%d with status:%d\n",register_to_write,status);
        return -1;
    }

    //NOW WRITE DATA TO THE REGISTER
    status = HAL_SPI_Transmit(&hspi2, payload, register_lenght, 3);
    _2500_ns_delay(40);

    if (status != HAL_OK){
        //sprintf(current_command->response,"ERROR;ADFS7124_WRITE_TO_REGISTER FAILED TO WRITE DATA TO REGISTER:%d with status:%d\n",register_to_write,status);
        return -2;
    }

    //sprintf(current_command->response,"OK;ADFS7124_WRITE_TO_REGISTER REGISTER:%d DATA:%lu\n",register_to_write,data);  
    return 1;

}

int8_t RESISTOR_DRIVER_setChannelEnableBit(uint8_t channel_number, uint8_t enable_bit, ResistorMeasurementConfig* config){
  //RAD CHANNEL CONFIGURATION FROM REGISTER, channels are indexed from 0 to 15
  uint32_t current_channel_settings;
  int8_t status = RESISTOR_DRIVER_ADFS7124readFromRegister(0x09+channel_number, 2, config, &current_channel_settings);
  if (status < 0){
    //printf("ERROR;RESISTOR_DRIVER_ENABLE_CHANNEL FAILED TO READ CHANNEL SETTINGS\n");
    return status;
  }

  //SET CHANNEL ENABLE BIT, this bit is the bit 15 of the register
  if (enable_bit == 0){
    current_channel_settings = current_channel_settings & ~(1UL << 15);
  }
  else{
    current_channel_settings = current_channel_settings | (1UL << 15);
  }
  //WRITE BACK TO THE REGISTER
  status = RESISTOR_DRIVER_ADFS7124writeToRegister(0x09+channel_number, 2, current_channel_settings, config);
  if (status < 0){
    //printf("ERROR;RESISTOR_DRIVER_ENABLE_CHANNEL FAILED TO WRITE CHANNEL SETTINGS\n");
    return status;
  }

  return 1;
}

int8_t RESISTOR_DRIVER_selectChannel(uint8_t channel_number, ResistorMeasurementConfig* config){
  //DISABLE ALL CHANNELS, ENABLE ONLY SELECTED
  for (uint8_t i = 0; i < 16; i++)
  {
    if (i == channel_number){
        if(RESISTOR_DRIVER_setChannelEnableBit(i, 1, config) < 0){
          return -1;
        }
    }
    else{
      
      if (RESISTOR_DRIVER_setChannelEnableBit(i, 0, config) < 0){
        return -2;
        }
    }
  }
  return 1;
  }

int8_t RESISTOR_DRIVER_measureChannelADCValue(uint8_t channel_nr, ResistorMeasurementConfig* config, uint32_t* readout_data){
  //read status register until nRDY_bit7 == 0 (READY) and also ACTIVE_CHANNEL_bits_3_to_0 == channel_nr
  int8_t response;
  uint32_t readout_status;
  int32_t timeout = _us_tick + 2000000; //5s timeout 2e6*2.5ms = 5s
  uint32_t n_of_bad_reads = 0;
  while(1){
    if (timeout < _us_tick){
      return -1;
    }
    response = RESISTOR_DRIVER_ADFS7124readFromRegister(0x00, 1, config, &readout_status);
    //printf("response:%d, readout_status:%lu\n",response,readout_status);
    if (response < 0){
      n_of_bad_reads++;
      if (n_of_bad_reads > 10){
        return -2;
      }
      continue;
    }


    if ((readout_status & (1UL << 7)) != 0){ //nRDY_bit7 must be 0
      continue;
    }

    if ((readout_status & 0b1111) != channel_nr){ //ACTIVE_CHANNEL_bits_3_to_0 must be channel_nr
      continue;
    }

    //DATA SHOULD BE READY
    //READ DATA REGISTER 0x2 with length of 3 bytes
    uint32_t adc_readout;
    response = RESISTOR_DRIVER_ADFS7124readFromRegister(0x02, 3, config, &adc_readout);
    if (response < 0){
      return -2; //ERROR;RESISTOR_DRIVER_MEASURE_CHANNEL_ADC_VALUE FAILED TO READ DATA REGISTER
    }

    *readout_data = adc_readout;
    return 1;//OK;RESISTOR_DRIVER_MEASURE_CHANNEL_ADC_VALUE
  }
  
  return 0;}

uint8_t RESISTOR_DRIVER_setCurrentLevel(uint8_t current_level, ResistorMeasurementConfig* config){return 0;}
uint8_t RESISTOR_DRIVER_setCurrentSourceToPin(uint8_t pin_number, ResistorMeasurementConfig* config){return 0;}
uint8_t RESISTOR_DRIVER_setReferenceSourceToExternal(ResistorMeasurementConfig* config){return 0;}
uint8_t RESISTOR_DRIVER_setReferenceSourceToInternal(ResistorMeasurementConfig* config){return 0;}
uint16_t RESISTOR_DRIVER_setPinGrounding(uint8_t pin_number,uint8_t grounding_option, ResistorMeasurementConfig* config){return 0;}