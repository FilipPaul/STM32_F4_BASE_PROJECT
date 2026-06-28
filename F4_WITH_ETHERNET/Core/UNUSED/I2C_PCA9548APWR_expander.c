
/*
 ******************************************************************************
 * @file           : I2C_PCA9548APWR_expander.c
 * @brief          : This file provides code for the configuration of the PCA9548A I2C expander.
 ******************************************************************************
 * @Author: Filip Paul
 * @date: 2024
 * 
 *  @attention
 *
 * Use of that code without the express written permission of RAFIONTECH s.r.o. is prohibited.
 * Copyright (c) 2024 RAFIONTECH s.r.o.
 */
#include "I2C_PCA9548APWR_expander.h"

static uint16_t PCA9548APWR_getDeviceAddress(const PCA9548APWRHandle* expander){
    return (uint16_t)(expander->address << 1);
}

int32_t PCA9548APWR_readControlRegister(PCA9548APWRHandle* expander){
    uint8_t control_register_value;
    if (HAL_I2C_Master_Receive(expander->i2c_handle, PCA9548APWR_getDeviceAddress(expander), &control_register_value, 1, HAL_MAX_DELAY) != HAL_OK){
        //Error occurred while reading the control register
        expander->last_i2c_error = HAL_I2C_GetError(expander->i2c_handle);
        return -1;
    }

    expander->last_i2c_error = HAL_I2C_ERROR_NONE;
    return control_register_value;
}

bool PCA9548APWR_writeControlRegister(PCA9548APWRHandle* expander, uint8_t register_value){
    if (HAL_I2C_Master_Transmit(expander->i2c_handle, expander->address, &register_value, 1, 15000) != HAL_OK){
        //Error occurred while writing to the control register
        expander->last_i2c_error = HAL_I2C_GetError(expander->i2c_handle);
        return false;
    }
    expander->last_i2c_error = HAL_I2C_ERROR_NONE;
    return true;
}


void PCA9548APWR_reset(PCA9548APWRHandle* expander, bool perform_reset){
    //reset is active low -> if perform_reset is true, we set the nrst pin low, otherwise we set it high
    if (perform_reset){
        HAL_GPIO_WritePin(expander->nrst_port, expander->nrst, GPIO_PIN_RESET);
    } else {
        HAL_GPIO_WritePin(expander->nrst_port, expander->nrst, GPIO_PIN_SET);
    }
}


bool PCA9548APWR_init(PCA9548APWRHandle* expander){

    //Reset the expander
    PCA9548APWR_reset(expander, true);
    //WAIT 10 ms
    HAL_Delay(10);

    //INIT THE I2C peripheral
    //The I2C peripheral should be already initialized in the main.c, so we just need to check if it's ready
    if (HAL_I2C_GetState(expander->i2c_handle) != HAL_I2C_STATE_READY){
        //I2C peripheral is not ready -> instantiate it
        //This should not happen, because the I2C peripheral should be initialized in the main.c, but we can try to initialize it here just in case
        HAL_I2C_DeInit(expander->i2c_handle);
        expander->i2c_handle->Init.ClockSpeed = expander->clock_speed;
        //Try to init 
        if (HAL_I2C_Init(expander->i2c_handle) != HAL_OK){
            //Initialization failed
            return false;
        }
    }
    //PUT EXPANEDER OUT OF RESET
    PCA9548APWR_reset(expander, false);
    HAL_Delay(10); //Delay for 10ms

    //After reset, all channels are disabled.
    expander->selected_channel = PCA9548APWR_NO_CHANNEL_SELECTED;
    expander->last_i2c_error = HAL_I2C_ERROR_NONE;
    return true;
}

bool PCA9548APWR_selectChannel(PCA9548APWRHandle* expander, uint8_t channel_number){
    if (channel_number > 7){
        //Invalid channel number
        return false;
    }
    uint8_t register_value = 1 << channel_number; //Set the bit corresponding to the channel number
    if (!PCA9548APWR_writeControlRegister(expander, register_value)){
        //Error occurred while writing to the control register
        return false;
    }
    expander->selected_channel = channel_number;
    return true;
}

uint32_t PCA9548APWR_getSelectedChannel(PCA9548APWRHandle* expander){
    int32_t control_register_value = PCA9548APWR_readControlRegister(expander);
    if (control_register_value < 0){
        //Error occurred while reading the control register
        return PCA9548APWR_NO_CHANNEL_SELECTED;
    }

    if (control_register_value == 0){
        expander->selected_channel = PCA9548APWR_NO_CHANNEL_SELECTED;
        return PCA9548APWR_NO_CHANNEL_SELECTED;
    }

    //Single-channel mode expects exactly one enabled bit in the control register.
    if ((control_register_value & (control_register_value - 1)) != 0){
        expander->selected_channel = PCA9548APWR_NO_CHANNEL_SELECTED;
        return PCA9548APWR_NO_CHANNEL_SELECTED;
    }

    //The selected channel is the position of the only bit that is set in the control register.
    for (uint8_t i = 0; i < 8; i++){
        if (control_register_value & (1 << i)){
            expander->selected_channel = i;
            return i;
        }
    }
    expander->selected_channel = PCA9548APWR_NO_CHANNEL_SELECTED;
    return PCA9548APWR_NO_CHANNEL_SELECTED;
    
}