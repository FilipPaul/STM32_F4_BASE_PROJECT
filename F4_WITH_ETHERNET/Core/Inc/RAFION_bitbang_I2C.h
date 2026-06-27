#ifndef RAFION_BITBANG_I2C_H
#define RAFION_BITBANG_I2C_H
#include "main.h"
#include "stdint.h"
#include "stdbool.h"

typedef struct RafionBitbangHandle{
    uint16_t sda_pin; //SDA
    uint16_t scl_pin; //SCL

    GPIO_TypeDef* sda_port;
    GPIO_TypeDef* scl_port;
    //speed
    uint32_t clock_speed;
    uint32_t clock_stretch_timeout_ms;
    uint32_t last_error;

} RafionBitbangHandle;


HAL_StatusTypeDef RafionBitbangI2C_Init(RafionBitbangHandle* handle);
HAL_StatusTypeDef RafionBitbangI2C_MASTER_Transmit(RafionBitbangHandle* handle, uint16_t dev_address, const uint8_t* data, uint16_t size, uint32_t timeout);
HAL_StatusTypeDef RafionBitbangI2C_MASTER_Receive(RafionBitbangHandle* handle, uint16_t dev_address, uint8_t* data, uint16_t size, uint32_t timeout);
HAL_StatusTypeDef RafionBitbangI2C_MASTER_IsDeviceReady(RafionBitbangHandle* handle, uint16_t dev_address, uint32_t trials, uint32_t timeout);

int32_t RafionBitbangDoStartCondition(RafionBitbangHandle* handle);
int32_t RafionBitbangDoStopCondition(RafionBitbangHandle* handle);
int32_t RafionBitbangDoClockPulse(RafionBitbangHandle* handle);
int32_t RafionBitbangWriteBit(RafionBitbangHandle* handle, bool bit_value);
int32_t RafionBitbangReadBit(RafionBitbangHandle* handle, bool*bit_value);
#endif