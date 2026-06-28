#ifndef I2C_PCA9548APWR_EXPANDER_H
#define I2C_PCA9548APWR_EXPANDER_H
#include "main.h"
#include "stdint.h"
#include "globals.h"
//include for bool
#include "stdbool.h"

#define PCA9548APWR_NO_CHANNEL_SELECTED ((uint32_t)-1)

typedef struct PCA9548APWRHandle{
    uint16_t nrst; //nrst
    uint16_t sda; //SDA
    uint16_t scl; //SCL

    GPIO_TypeDef* nrst_port;
    GPIO_TypeDef* sda_port;
    GPIO_TypeDef* scl_port;

    I2C_HandleTypeDef* i2c_handle;
    //I2C address
    uint8_t address;

    //Currently selected external I2C channel, or PCA9548APWR_NO_CHANNEL_SELECTED
    uint32_t selected_channel;

    //speed
    uint32_t clock_speed;

    //Last HAL I2C error associated with this expander
    uint32_t last_i2c_error;

} PCA9548APWRHandle;


int32_t PCA9548APWR_readControlRegister(PCA9548APWRHandle* expander);
bool PCA9548APWR_writeControlRegister(PCA9548APWRHandle* expander, uint8_t register_value);
void PCA9548APWR_reset(PCA9548APWRHandle* expander, bool perform_reset);
bool PCA9548APWR_init(PCA9548APWRHandle* expander);
bool PCA9548APWR_selectChannel(PCA9548APWRHandle* expander, uint8_t channel_number);
uint32_t PCA9548APWR_getSelectedChannel(PCA9548APWRHandle* expander);
#endif