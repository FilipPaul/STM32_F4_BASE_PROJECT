
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
#include "RAFION_bitbang_I2C.h"

static uint16_t PCA9548APWR_getDeviceAddress(const PCA9548APWRHandle* expander){
    return (uint16_t)(expander->address << 1);
}

uint32_t PCA9548APWR_setSpeed(PCA9548APWRHandle* expander, uint32_t clock_speed){
    expander->clock_speed = clock_speed;
    return clock_speed;
}

static void PCA9548APWR_fillBitbangHandle(const PCA9548APWRHandle* expander, RafionBitbangHandle* bitbang, bool pins_swapped)
{
    if (pins_swapped){
        bitbang->sda_pin = expander->scl;
        bitbang->scl_pin = expander->sda;
        bitbang->sda_port = expander->scl_port;
        bitbang->scl_port = expander->sda_port;
    } else {
        bitbang->sda_pin = expander->sda;
        bitbang->scl_pin = expander->scl;
        bitbang->sda_port = expander->sda_port;
        bitbang->scl_port = expander->scl_port;
    }
    bitbang->clock_speed = expander->clock_speed;
    bitbang->clock_stretch_timeout_ms = 10U;
    bitbang->last_error = HAL_I2C_ERROR_NONE;
}

static bool PCA9548APWR_probeMapping(PCA9548APWRHandle* expander, bool pins_swapped)
{
    RafionBitbangHandle bitbang = {0};
    uint8_t control_register_value = 0U;

    PCA9548APWR_fillBitbangHandle(expander, &bitbang, pins_swapped);
    if (RafionBitbangI2C_Init(&bitbang) != HAL_OK){
        expander->last_i2c_error = HAL_I2C_ERROR_BERR;
        return false;
    }

    if (RafionBitbangI2C_MASTER_Receive(&bitbang, PCA9548APWR_getDeviceAddress(expander), &control_register_value, 1U, HAL_MAX_DELAY) != HAL_OK){
        expander->last_i2c_error = bitbang.last_error;
        return false;
    }

    expander->pins_swapped = pins_swapped;
    expander->last_i2c_error = HAL_I2C_ERROR_NONE;
    expander->selected_channel = (control_register_value == 0U) ? PCA9548APWR_NO_CHANNEL_SELECTED : expander->selected_channel;
    return true;
}

int32_t PCA9548APWR_readControlRegister(PCA9548APWRHandle* expander){
    RafionBitbangHandle bitbang = {0};
    uint8_t control_register_value;

    PCA9548APWR_fillBitbangHandle(expander, &bitbang, expander->pins_swapped);
    if (RafionBitbangI2C_MASTER_Receive(&bitbang, PCA9548APWR_getDeviceAddress(expander), &control_register_value, 1, HAL_MAX_DELAY) != HAL_OK){
        //Error occurred while reading the control register
        expander->last_i2c_error = bitbang.last_error;
        return -1;
    }

    expander->last_i2c_error = HAL_I2C_ERROR_NONE;
    return control_register_value;
}

bool PCA9548APWR_writeControlRegister(PCA9548APWRHandle* expander, uint8_t register_value){
    RafionBitbangHandle bitbang = {0};

    PCA9548APWR_fillBitbangHandle(expander, &bitbang, expander->pins_swapped);
    if (RafionBitbangI2C_MASTER_Transmit(&bitbang, PCA9548APWR_getDeviceAddress(expander), &register_value, 1, 15000) != HAL_OK){
        //Error occurred while writing to the control register
        expander->last_i2c_error = bitbang.last_error;
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
    if (expander->i2c_handle != NULL){
        HAL_I2C_DeInit(expander->i2c_handle);
    }

    //Reset the expander
    PCA9548APWR_reset(expander, true);
    //WAIT 10 ms
    HAL_Delay(50);

    //PUT EXPANEDER OUT OF RESET
    PCA9548APWR_reset(expander, false);
    HAL_Delay(10); //Delay for 10ms

    if (!PCA9548APWR_probeMapping(expander, expander->pins_swapped) && !PCA9548APWR_probeMapping(expander, !expander->pins_swapped)){
        return false;
    }

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