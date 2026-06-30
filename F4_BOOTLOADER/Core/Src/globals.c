
#include "globals.h"
#include "main.h"

static StaticQueue_t commandQueueControlBlock;
static uint8_t commandQueueStorage[2 * sizeof(commandTemplate)];

//MUTEX for filling commandTemplate
SemaphoreHandle_t commandTemplateMutex;

//QUEUE OF COMMANDS TO BE PROCESSED BY MAIN LOOP
QueueHandle_t commandQueueHandle;

volatile uint32_t ethernet_cable_plugged = 0;


void init_command_queue(void)
{
    commandQueueHandle = xQueueCreateStatic(2,
                                            sizeof(commandTemplate),
                                            commandQueueStorage,
                                            &commandQueueControlBlock);
}

int32_t GlobalRecoverStalledI2CBus(I2C_HandleTypeDef* hi2c, GPIO_TypeDef* scl_port, uint16_t scl_pin, GPIO_TypeDef* sda_port, uint16_t sda_pin)
{
    //CHECK if hi2c is not NULL
    if (hi2c != NULL)
    {
        //DEINIT I2C
        if (HAL_I2C_DeInit(hi2c) != HAL_OK)
        {
            return -1; //Failed to deinit I2C
        }

        osDelay(10); //Short delay to ensure deinitialization is complete
    }

    //CONFIGURE SCL and SDA as GPIO outputs to manually toggle them
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = scl_pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;

    HAL_GPIO_Init(scl_port, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = sda_pin;
    HAL_GPIO_Init(sda_port, &GPIO_InitStruct);

    //MANUALLY TOGGLE SCL to free the bus,
    //put 1ms delay between toggles to ensure any connected slave devices have time to react

    //Some sensors require 9 clock pulses to release the bus, so we will do 9 toggles just to be safe
    //toggle 18times for sure
    for (int i = 0; i < 18; i++)
    {
        HAL_GPIO_WritePin(scl_port, scl_pin, GPIO_PIN_SET);
        osDelay(1);
        HAL_GPIO_WritePin(scl_port, scl_pin, GPIO_PIN_RESET);
        osDelay(1);
    }

    //SDA SHOULD BE RELEASED.. check it
    if (HAL_GPIO_ReadPin(sda_port, sda_pin) == GPIO_PIN_RESET)
    {
        return -2; //Failed to recover I2C bus, SDA line is still held low
    }

    //If we had a valid hi2c handle, try to reinitialize it
    if (hi2c != NULL)
    {
        if (HAL_I2C_Init(hi2c) != HAL_OK)
        {
            return -3; //Failed to reinitialize I2C
        }
    }

    return 0; //Successfully recovered I2C bus

}


volatile uint32_t _us_tick = 0;
volatile uint32_t TIM_7_tick_1_us = 0;
void _2500_ns_delay(uint32_t us){
    uint32_t start = _us_tick;
    //printf("start: %lu, _us_tick: %lu, us: %lu\n", start, _us_tick, us);
    //HAL_GPIO_WritePin(TIMER_check_GPIO_Port, TIMER_check_Pin, GPIO_PIN_SET);
    while((_us_tick - start) < us){
        __NOP();
    };
    //HAL_GPIO_WritePin(TIMER_check_GPIO_Port, TIMER_check_Pin, GPIO_PIN_RESET);
}