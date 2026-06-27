#ifndef GLOBALS_H
#define GLOBALS_H
#include "stdio.h"
#include "stdint.h"
#include "commandParser.h"
//include for xQueueHandle_t
#include "cmsis_os.h"
#include "main.h"

#define DELAY_MULTIPLES_OF_2500_NS 5 //5us
#define DO_CLK_PULSE(pinPort, pin) do {\
            HAL_GPIO_WritePin(pinPort,pin,GPIO_PIN_SET);\
            _2500_ns_delay(DELAY_MULTIPLES_OF_2500_NS);\
            HAL_GPIO_WritePin(pinPort,pin,GPIO_PIN_RESET);\
            _2500_ns_delay(DELAY_MULTIPLES_OF_2500_NS);\
           } while(0)



void _2500_ns_delay(uint32_t us); //delay in multiples of approx 2.5us -> not accurate!!
void init_command_queue(void);
int32_t GlobalRecoverStalledI2CBus(I2C_HandleTypeDef* hi2c, GPIO_TypeDef* scl_port, uint16_t scl_pin, GPIO_TypeDef* sda_port, uint16_t sda_pin);
//MUTEX for filling commandTemplate
extern SemaphoreHandle_t commandTemplateMutex;
extern volatile uint32_t ethernet_cable_plugged;
//QEUEUE OF COMMANDS TO BE PROCESSED BY MAIN LOOP
//QUEUE type is commandTemplate
extern QueueHandle_t commandQueueHandle;
extern volatile uint32_t _us_tick;
extern volatile uint32_t TIM_7_tick_1_us;
#endif