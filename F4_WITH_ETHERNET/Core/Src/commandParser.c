#include "commandParser.h"
#include "string.h"
#include "stdio.h"
#include "stdlib.h"
#include "multiplexers.h"
#include "globals.h"
#include "main.h"
#include "lwip/opt.h"
#include "lwip/api.h"
#include "lwip/sys.h"
#include "lwip.h"
#include "IPsettings.h"
#include "EEPROM_24AA02E48T_I_IOT_settings.h"
#include "shiftRegisters.h"
#include "RELAYS_AND_IO_DRIVER.h"
#include "relayCards.h"
#include "tcpserver.h"
#include "I2C_PCA9548APWR_expander.h"
#include "VL53L1X_RAFIONTECH.h"
#include "UartMessageHandler.h"

extern SPI_HandleTypeDef hspi2;
extern I2C_HandleTypeDef hi2c1;
extern I2C_HandleTypeDef hi2c2;
extern I2C_HandleTypeDef hi2c3;
extern UART_HandleTypeDef huart4;
extern UART_HandleTypeDef huart5;
extern struct netif gnetif;
extern ADC_HandleTypeDef hadc1;
extern ADC_HandleTypeDef hadc2;
extern ExternalRelayCard EXTERNAL_32_RELAY_CARD;
extern ExternalPortMapping EXTERNAL_PORT_MAPPING;
char SPI2_BUFFER[24];
extern struct RELAY_IO_DRIVER_STRUCT RELAY_IO_DRIVER;
extern PCA9548APWRHandle pca9548apwr_expander;
extern VL53L1X* tof_sensors[8];
extern uartHandlerStruct uart4_handler_struct;
extern uartHandlerStruct uart5_handler_struct;
extern osMutexId take_measurement_mutex;

#define BOOT_ADDR	0x1FFF0000U	// STM32F407 system memory (ROM bootloader) base address
#define	MCU_IRQS	70u	// no. of NVIC IRQ inputs

struct boot_vectable_ {
    uint32_t Initial_SP;
    void (*Reset_Handler)(void);
};

#define BOOTVTAB	((struct boot_vectable_ *)BOOT_ADDR)

static VL53L1X* CommandParser_GetTofSensor(uint8_t sensor_index)
{
    if (sensor_index >= 8U)
    {
        return NULL;
    }
    return tof_sensors[sensor_index];
}

static I2C_HandleTypeDef* CommandParser_GetI2CHandle(uint8_t bus_index)
{
    switch (bus_index)
    {
    case 1:
        return &hi2c1;
    case 2:
        return &hi2c2;
    case 3:
        return &hi2c3;
    default:
        return NULL;
    }
}

static uartHandlerStruct* CommandParser_GetUartHandle(uint8_t uart_index)
{
    switch (uart_index)
    {
    case 4:
        return &uart4_handler_struct;
    case 5:
        return &uart5_handler_struct;
    default:
        return NULL;
    }
}

static uint8_t CommandParser_ParseTofMode(const char* mode_str, uint16_t* mode)
{
    if ((mode_str == NULL) || (mode == NULL))
    {
        return 0;
    }

    //TOF [0-7] SET_MODE SHORT
    if ((strcmp(mode_str, "SHORT") == 0) || (strcmp(mode_str, "1") == 0))
    {
        *mode = 1U;
        return 1;
    }

    //TOF [0-7] SET_MODE LONG
    if ((strcmp(mode_str, "LONG") == 0) || (strcmp(mode_str, "2") == 0))
    {
        *mode = 2U;
        return 1;
    }

    return 0;
}

void JumpToBootloader(void)
{
    uint32_t boot_stack_ptr = BOOTVTAB->Initial_SP;
    void (*boot_reset_handler)(void) = BOOTVTAB->Reset_Handler;

    /* Disable all interrupts while deinitializing the running application context. */
    __disable_irq();

    /* Stop SysTick. */
    SysTick->CTRL = 0;
    SysTick->LOAD = 0;
    SysTick->VAL = 0;

    /* Reset HAL state/peripherals and clocks. */
    HAL_DeInit();
    HAL_RCC_DeInit();

    /* Disable and clear all NVIC interrupts. */
    for (uint8_t i = 0; i < (MCU_IRQS + 31u) / 32; i++)
    {
        NVIC->ICER[i] = 0xFFFFFFFF;
        NVIC->ICPR[i] = 0xFFFFFFFF;
    }

    /* Map system memory (ROM) at address 0x00000000 for bootloader startup. */
    __HAL_RCC_SYSCFG_CLK_ENABLE();
    __HAL_SYSCFG_REMAPMEMORY_SYSTEMFLASH();

    /* Ensure remap takes effect before stack/vector handover. */
    __DSB();
    __ISB();

    /* Set MSP then jump to ROM bootloader reset handler. */
    __set_MSP(boot_stack_ptr);
    __enable_irq();
    boot_reset_handler();

    while (1)
    {
    }
}


uint8_t runCommand(commandTemplate* current_command){
    //"""Implementation of commands"""
    if (current_command->command[0] == '\0'){
        strcpy(current_command->response,"ERROR;No command received\n");
        return 0;
    };

        //HELP
        if (strcmp(current_command->command,"HELP") == 0){
		snprintf(current_command->response, sizeof(current_command->response), "OK;Help command received\n");
    }


        //EEPROM <handled by EEPROMInputsParser>
    else if (strcmp(current_command->command,"EEPROM") == 0){
        HAL_GPIO_WritePin(EEPROM_WP_GPIO_Port, EEPROM_WP_Pin, GPIO_PIN_RESET);
        EEPROMInputsParser(current_command);
        HAL_GPIO_WritePin(EEPROM_WP_GPIO_Port, EEPROM_WP_Pin, GPIO_PIN_SET);
    }

        //IPCONFIG <handled by EEPROMInputsParser>
    else if (strcmp(current_command->command,"IPCONFIG") == 0){
        
        //SAME CALLBACS AS EEPROM with different command name...
        //IPCONFIG WRITE IP 192 168 1 2
        //IPCONFIG WRITE SUBNET 255 255 255 0
        //IPCONFIG WRITE GATEWAY 192 168 1 0
        //IPCONFIG WRITE CUSTOM_MAC 3D 00 00 15 3E D0

        //IPCONFIG READ IP
        //IPCONFIG READ SUBNET 255
        //IPCONFIG READ GATEWAY
        //IPCONFIG READ CUSTOM_MAC

        //IPCONFIG ERASE_CUSTOM IP
        //IPCONFIG ERASE_CUSTOM SUBNET 255
        //IPCONFIG ERASE_CUSTOM GATEWAY
        //IPCONFIG ERASE_CUSTOM CUSTOM_MAC
        HAL_GPIO_WritePin(EEPROM_WP_GPIO_Port, EEPROM_WP_Pin, GPIO_PIN_RESET);
        EEPROMInputsParser(current_command);
        HAL_GPIO_WritePin(EEPROM_WP_GPIO_Port, EEPROM_WP_Pin, GPIO_PIN_SET);
        //ALL THESE SETTIGS ARE SAVED IN EEPROM -> after restart the board will read the values from EEPROM
    }

    //*IDN?
    else if (strcmp(current_command->command,"*IDN?") == 0){
        snprintf(current_command->response, sizeof(current_command->response), "OK;RAFIONTECH s.r.o. - E0000010(FUT CTRL BOARD) V1.0\n");
    }

    //*ENTER_BOOTLOADER
    else if (strcmp(current_command->command,"*ENTER_BOOTLOADER") == 0){
        printf("OK;GOING TO BOOTLOADER\n");
        //WAIT UNTIL PRINTF FINISHES
        HAL_Delay(100);
        JumpToBootloader();
    }

    //*RST
    else if (strcmp(current_command->command,"*RST") == 0){
        NVIC_SystemReset();
    }

    //I2C_EXPANDER <ACTION> [...]
    else if (strcmp(current_command->command,"I2C_EXPANDER") == 0){
        osMutexWait(take_measurement_mutex, portMAX_DELAY); //Take mutex to ensure no measurements are being taken while we are controlling the I2C expander
        //I2C_EXPANDER SELECT_CHANNEL [CHANNEL NUMBER 0-7]
        if (strcmp(current_command->subcommands[0],"SELECT_CHANNEL") == 0){
            uint8_t channel_number = atoi(current_command->subcommands[1]);
            bool result = PCA9548APWR_selectChannel(&pca9548apwr_expander, channel_number);
            if (result) {
                snprintf(current_command->response, sizeof(current_command->response), "OK;I2C_EXPANDER SELECT_CHANNEL:%d\n", channel_number);
            } else {
                snprintf(current_command->response, sizeof(current_command->response), "ERROR;I2C_EXPANDER SELECT_CHANNEL FAILED HAL_ERROR:%u\n", pca9548apwr_expander.last_i2c_error);
            }
        }

        //I2C_EXPANDER GET_SELECTED_CHANNEL
        else if (strcmp(current_command->subcommands[0],"GET_SELECTED_CHANNEL") == 0){
            uint32_t selected_channel = PCA9548APWR_getSelectedChannel(&pca9548apwr_expander);
            if (selected_channel != PCA9548APWR_NO_CHANNEL_SELECTED) {
                snprintf(current_command->response, sizeof(current_command->response), "OK;I2C_EXPANDER SELECTED_CHANNEL:%u\n", selected_channel);
            } else if (pca9548apwr_expander.last_i2c_error == HAL_I2C_ERROR_NONE) {
                snprintf(current_command->response, sizeof(current_command->response), "OK;I2C_EXPANDER SELECTED_CHANNEL:NONE\n");
            } else {
                snprintf(current_command->response, sizeof(current_command->response), "ERROR;I2C_EXPANDER GET_SELECTED_CHANNEL FAILED HAL_ERROR:%u\n", pca9548apwr_expander.last_i2c_error);
            }
        }

        //I2C_EXPANDER RESET [0/1]
        else if (strcmp(current_command->subcommands[0],"RESET") == 0){
            bool perform_reset = atoi(current_command->subcommands[1]);
            PCA9548APWR_reset(&pca9548apwr_expander, perform_reset);
            snprintf(current_command->response, sizeof(current_command->response), "OK;I2C_EXPANDER RESET PERFORMED:%d\n", perform_reset);
        }
        
        //I2C_EXPANDER INIT
        else if (strcmp(current_command->subcommands[0],"INIT") == 0){
            bool result = PCA9548APWR_init(&pca9548apwr_expander);
            if (result) {
                snprintf(current_command->response, sizeof(current_command->response), "OK;I2C_EXPANDER INITIALIZED SUCCESSFULLY\n");
            } else {
                snprintf(current_command->response, sizeof(current_command->response), "ERROR;I2C_EXPANDER INITIALIZATION FAILED HAL_ERROR:%u\n", pca9548apwr_expander.last_i2c_error);
            }
        }

        //I2C_EXPANDER WRITE_CONTROL_REGISTER [VALUE]
        else if (strcmp(current_command->subcommands[0],"WRITE_CONTROL_REGISTER") == 0){
            uint8_t register_value = atoi(current_command->subcommands[1]);
            bool result = PCA9548APWR_writeControlRegister(&pca9548apwr_expander, register_value);
            if (result) {
                snprintf(current_command->response, sizeof(current_command->response), "OK;I2C_EXPANDER WRITE_CONTROL_REGISTER VALUE:%d\n", register_value);
            } else {
                snprintf(current_command->response, sizeof(current_command->response), "ERROR;I2C_EXPANDER WRITE_CONTROL_REGISTER FAILED HAL_ERROR:%u\n", pca9548apwr_expander.last_i2c_error);
            }
        }

         //I2C_EXPANDER READ_CONTROL_REGISTER
        else if (strcmp(current_command->subcommands[0],"READ_CONTROL_REGISTER") == 0){
            int32_t register_value = PCA9548APWR_readControlRegister(&pca9548apwr_expander);
            if (register_value >= 0) {
                snprintf(current_command->response, sizeof(current_command->response), "OK;I2C_EXPANDER CONTROL_REGISTER_VALUE:%ld\n", (long)register_value);
            } else {
                snprintf(current_command->response, sizeof(current_command->response), "ERROR;I2C_EXPANDER READ_CONTROL_REGISTER FAILED HAL_ERROR:%u\n", pca9548apwr_expander.last_i2c_error);
            }
        }

        //I2C_EXPANDER SET_SPEED [VALUE IN HZ]
        else if (strcmp(current_command->subcommands[0],"SET_SPEED") == 0){
            uint32_t clock_speed = strtoul(current_command->subcommands[1], NULL, 10);
            PCA9548APWR_setSpeed(&pca9548apwr_expander, clock_speed);
            snprintf(current_command->response, sizeof(current_command->response), "OK;I2C_EXPANDER SET_SPEED:%u\n", clock_speed);
        }

        else{
            snprintf(current_command->response, sizeof(current_command->response), "ERROR;No such subcommand\n");
        }
        osMutexRelease(take_measurement_mutex); //Release mutex after we are done controlling the I2C expander
    }

    //I2C [1-3] REINIT [SPEED IN HZ]
    else if (strcmp(current_command->command,"I2C") == 0){
        osMutexWait(take_measurement_mutex, portMAX_DELAY); //Take mutex to ensure no measurements are being taken while we are reinitializing the I2C bus
        if (current_command->number_of_subcommands < 2){
            snprintf(current_command->response, sizeof(current_command->response), "ERROR;I2C requires BUS and ACTION\n");
            osMutexRelease(take_measurement_mutex); //Release mutex before returning
            return 0;
        }

        uint8_t bus_index = (uint8_t)atoi(current_command->subcommands[0]);
        I2C_HandleTypeDef* i2c_handle = CommandParser_GetI2CHandle(bus_index);
        if (i2c_handle == NULL){
            snprintf(current_command->response, sizeof(current_command->response), "ERROR;I2C bus must be 1, 2 or 3\n");
            osMutexRelease(take_measurement_mutex); //Release mutex before returning
            return 0;
        }

        //I2C [1-3] REINIT [SPEED IN HZ]
        if (strcmp(current_command->subcommands[1],"REINIT") == 0){
            uint32_t speed_hz = i2c_handle->Init.ClockSpeed;
            if (current_command->number_of_subcommands >= 3){
                speed_hz = strtoul(current_command->subcommands[2], NULL, 10);
                if (speed_hz == 0U){
                    snprintf(current_command->response, sizeof(current_command->response), "ERROR;I2C REINIT speed must be > 0\n");
                    osMutexRelease(take_measurement_mutex); //Release mutex before returning
                    return 0;
                }
            }

            i2c_handle->Init.ClockSpeed = speed_hz;
            if (HAL_I2C_Init(i2c_handle) != HAL_OK){
                snprintf(current_command->response, sizeof(current_command->response), "ERROR;I2C %u REINIT FAILED HAL_ERROR:%lu\n", bus_index, (unsigned long)HAL_I2C_GetError(i2c_handle));
                osMutexRelease(take_measurement_mutex); //Release mutex before returning
                return 0;
            }

            snprintf(current_command->response, sizeof(current_command->response), "OK;I2C %u REINIT SPEED_HZ:%u\n", bus_index, speed_hz);
            osMutexRelease(take_measurement_mutex); //Release mutex after reinitializing the I2C bus
            return 1;
        }

        //I2C [1-3] SCAN_ADDRESSES
        if (strcmp(current_command->subcommands[1],"SCAN_ADDRESSES") == 0)
        {

            snprintf(current_command->response, sizeof(current_command->response), "OK;I2C %u SCAN_ADDRESSES:", bus_index);
            for (uint16_t address = 1; address < 128; address++){
                if (HAL_I2C_IsDeviceReady(i2c_handle, (address << 1), 4, 500) == HAL_OK){
                    size_t current_length = strlen(current_command->response);
                    snprintf(&current_command->response[current_length], sizeof(current_command->response) - current_length, "%02X,", address);
                }
            }
            snprintf(current_command->response + strlen(current_command->response) - 1, 2, "\n"); // Replace last comma with newline
            osMutexRelease(take_measurement_mutex); //Release mutex after scanning I2C addresses
            return 1;

        }

        snprintf(current_command->response, sizeof(current_command->response), "ERROR;No such I2C action\n");
        osMutexRelease(take_measurement_mutex); //Release mutex before returning
        return 0;

    }

    //RS232 [4-5] [SEND/BRIDGE] [...]
    else if (strcmp(current_command->command,"RS232") == 0){
        char payload[MAXIMUM_RESPONSE_LENGTH] = {0};
        size_t payload_length = 0;

        if (current_command->number_of_subcommands < 2){
            snprintf(current_command->response, sizeof(current_command->response), "ERROR;RS232 requires UART and ACTION\n");
            return 0;
        }

        uint8_t uart_index = (uint8_t)atoi(current_command->subcommands[0]);
        uartHandlerStruct* uart_handle = CommandParser_GetUartHandle(uart_index);
        if (uart_handle == NULL){
            snprintf(current_command->response, sizeof(current_command->response), "ERROR;RS232 UART must be 4 or 5\n");
            return 0;
        }

        //RS232 [4-5] SEND [MESSAGE...]
        if (strcmp(current_command->subcommands[1], "SEND") == 0){
            for (uint8_t i = 2; i < current_command->number_of_subcommands; ++i){
                size_t part_length = strlen(current_command->subcommands[i]);

                if ((payload_length + part_length + 3U) >= sizeof(payload)){
                    snprintf(current_command->response, sizeof(current_command->response), "ERROR;RS232 %u SEND payload too long\n", uart_index);
                    return 0;
                }

                if (payload_length > 0U){
                    payload[payload_length++] = ' ';
                }

                memcpy(&payload[payload_length], current_command->subcommands[i], part_length);
                payload_length += part_length;
                payload[payload_length] = '\0';
            }

            if (payload_length == 0U){
                snprintf(current_command->response, sizeof(current_command->response), "ERROR;RS232 %u SEND requires payload\n", uart_index);
                return 0;
            }

            payload[payload_length++] = '\r';
            payload[payload_length++] = '\n';
            payload[payload_length] = '\0';

            if (HAL_UART_Transmit(uart_handle->huart, (uint8_t *)payload, (uint16_t)payload_length, 1000) != HAL_OK){
                snprintf(current_command->response,
                         sizeof(current_command->response),
                         "ERROR;RS232 %u SEND FAILED HAL_ERROR:0x%08lX\n",
                         uart_index,
                         (unsigned long)HAL_UART_GetError(uart_handle->huart));
                return 0;
            }

            if (uart_handle->connected_as_bridge) {
                //DATA HAS BEEN SENT TO UART ... now do not respond to sender, but forward the message to the other UART
                current_command->response[0] = '\0'; //clear response, so no response will be sent to sender
                return 1;
            }
            snprintf(current_command->response,
                     sizeof(current_command->response),
                     "OK;RS232 %u SENT_BYTES:%u\n",
                     uart_index,
                     (unsigned int)payload_length);
            return 1;
        }

        //RS232 [4-5] BRIDGE [0/1]
        else if (strcmp(current_command->subcommands[1], "BRIDGE") == 0)
        {
            uint8_t bridge_mode = atoi(current_command->subcommands[2]);
            if (bridge_mode > 1U)  
            {

                snprintf(current_command->response, sizeof(current_command->response), "ERROR;RS232 %u BRIDGE mode must be 0 (disable) or 1 (enable)\n", uart_index);
                return 0;
            }

            uart_handle->connected_as_bridge = bridge_mode;
            snprintf(current_command->response, sizeof(current_command->response), "OK;RS232 %u BRIDGE_MODE:%s\n", uart_index, bridge_mode ? "ENABLED" : "DISABLED");
            return 1;
        }

        snprintf(current_command->response, sizeof(current_command->response), "ERROR;No such RS232 action\n");
        return 0;
    }

    //TOF [0-7] <ACTION> [...]
    else if (strcmp(current_command->command,"TOF") == 0){
        if (current_command->number_of_subcommands < 2){
            snprintf(current_command->response, sizeof(current_command->response), "ERROR;TOF requires SENSOR and ACTION\n");
            return 0;
        }

        int32_t sensor_staus_result;
        uint8_t sensor_index = (uint8_t)atoi(current_command->subcommands[0]);
        VL53L1X* sensor = CommandParser_GetTofSensor(sensor_index);
        if(sensor == NULL){
            snprintf(current_command->response, sizeof(current_command->response), "ERROR;TOF sensor index must be 0-7\n");
            return 0;
        }
        const char* ACTION = current_command->subcommands[1];


        /////////////////////////// STATES THAT ONLY CHECK AUTOMEASUREMENT FLAGS AND DO NOT NEED TO COMMUNICATE WITH SENSOR ///////////////////////////

        //TOF [0-7] GET_FLAG [PERSISTENT_OUT_OF_LIMIT| PERSISTENT_IN_LIMIT | IS_INSIDE_LIMITS | AUTOMATED_MEASUREMENT_IS_RUNNING | LAST_ERROR]
        if (strcmp(ACTION, "GET_FLAG") == 0){
            if (current_command->number_of_subcommands < 3){
                snprintf(current_command->response, sizeof(current_command->response), "ERROR;TOF GET_FLAG requires FLAG_NAME\n");
                return 0;
            }
            
            if (strcmp(current_command->subcommands[2], "PERSISTENT_OUT_OF_LIMIT") == 0)
            {
                snprintf(current_command->response, sizeof(current_command->response), "OK;TOF %u FLAG PERSISTENT_OUT_OF_LIMIT:%d\n", sensor_index, sensor->out_of_liminit_detected_single_flag);
            }
            else if (strcmp(current_command->subcommands[2], "PERSISTENT_IN_LIMIT") == 0)
            {
                snprintf(current_command->response, sizeof(current_command->response), "OK;TOF %u FLAG PERSISTENT_IN_LIMIT:%d\n", sensor_index, sensor->inside_limit_detected_single_flag);
            }
            else if (strcmp(current_command->subcommands[2], "IS_INSIDE_LIMITS") == 0)
            {
                //Check if automated messurement is running
                if( TOF_is_automated_measurement_running(sensor) ){
                    uint8_t is_inside_limits = TOF_is_currently_in_limits(sensor);
                    snprintf(current_command->response, sizeof(current_command->response), "OK;TOF %u FLAG IS_INSIDE_LIMITS:%d\n", sensor_index, is_inside_limits);
                }
                else{
                    snprintf(current_command->response, sizeof(current_command->response), "OK;TOF %u WARNING AUTOMATED MEASUREMENT IS NOT RUNNING, FLAG IS_INSIDE_LIMITS:%d\n", sensor_index, sensor->automated_measurement_is_running_flag);
                }
            }
            else if (strcmp(current_command->subcommands[2], "AUTOMATED_MEASUREMENT_IS_RUNNING") == 0)
            {
                uint8_t is_running = TOF_is_automated_measurement_running(sensor);
                snprintf(current_command->response, sizeof(current_command->response), "OK;TOF %u FLAG AUTOMATED_MEASUREMENT_IS_RUNNING:%d\n", sensor_index, is_running);
            }

            else if (strcmp(current_command->subcommands[2], "LAST_ERROR") == 0)
            {
                uint32_t last_error = sensor->detected_error_during_automated_measurement;
                snprintf(current_command->response, sizeof(current_command->response), "OK;TOF %u FLAG LAST_ERROR:%d\n", sensor_index, (uint8_t)last_error);
            }
            else{
                snprintf(current_command->response, sizeof(current_command->response), "ERROR;TOF GET_FLAG unknown flag name: %s\n", current_command->subcommands[2]);
                return 0;
            }
            return 1;
        }

        //TOF [0-7] SET_FLAG [PERSISTENT_OUT_OF_LIMIT| PERSISTENT_IN_LIMIT | AUTOMATED_MEASUREMENT | ALL_PERSISTENT_FLAGS | LASTE_ERROR] [0/1]
        else if(strcmp(ACTION, "SET_FLAG") == 0){
            if (current_command->number_of_subcommands < 4){
                snprintf(current_command->response, sizeof(current_command->response), "ERROR;TOF SET_FLAG requires FLAG_NAME and VALUE\n");
                return 0;
            }

            uint8_t value = atoi(current_command->subcommands[3]);
            if (strcmp(current_command->subcommands[2], "PERSISTENT_OUT_OF_LIMIT") == 0)
            {
                sensor->out_of_liminit_detected_single_flag = value;
                snprintf(current_command->response, sizeof(current_command->response), "OK;TOF %u FLAG PERSISTENT_OUT_OF_LIMIT set to:%d\n", sensor_index, value);
            }
            else if (strcmp(current_command->subcommands[2], "PERSISTENT_IN_LIMIT") == 0)
            {
                sensor->inside_limit_detected_single_flag = value;
                snprintf(current_command->response, sizeof(current_command->response), "OK;TOF %u FLAG PERSISTENT_IN_LIMIT set to:%d\n", sensor_index, value);
            }
            else if (strcmp(current_command->subcommands[2], "AUTOMATED_MEASUREMENT") == 0)
            {
                sensor->automated_measurement_is_running_flag = value;
                snprintf(current_command->response, sizeof(current_command->response), "OK;TOF %u FLAG AUTOMATED_MEASUREMENT set to:%d\n", sensor_index, value);
            }
            else if (strcmp(current_command->subcommands[2], "ALL_PERSISTENT_FLAGS") == 0)
            {
                sensor->out_of_liminit_detected_single_flag = value;
                sensor->inside_limit_detected_single_flag = value;
                snprintf(current_command->response, sizeof(current_command->response), "OK;TOF %u FLAG ALL_PERSISTENT_FLAGS set to:%d\n", sensor_index, value);
            }
            else if (strcmp(current_command->subcommands[2], "LAST_ERROR") == 0)
            {
                sensor->detected_error_during_automated_measurement = value;
                snprintf(current_command->response, sizeof(current_command->response), "OK;TOF %u FLAG LAST_ERROR set to:%d\n", sensor_index, value);
            }
        
            else{
                snprintf(current_command->response, sizeof(current_command->response), "ERROR;TOF SET_FLAG unknown flag name: %s\n", current_command->subcommands[2]);
                return 0;
            }
            return 1;
        }

        //TOF [0-7] [RANGE_MIN_MM] [RANGE_MAX_MM]
        else if (strcmp(ACTION, "SET_LIMITS") == 0){
            if (current_command->number_of_subcommands < 4){
                snprintf(current_command->response, sizeof(current_command->response), "ERROR;TOF SET_LIMITS requires SENSOR_INDEX, RANGE_MIN_MM and RANGE_MAX_MM\n");
                return 0;
            }

            uint16_t range_min_mm = (uint16_t)strtoul(current_command->subcommands[2], NULL, 10);
            uint16_t range_max_mm = (uint16_t)strtoul(current_command->subcommands[3], NULL, 10);
            if (range_min_mm >= range_max_mm){
                snprintf(current_command->response, sizeof(current_command->response), "ERROR;TOF SET_LIMITS RANGE_MIN_MM must be less than RANGE_MAX_MM\n");
                return 0;
            }

            sensor->min_distance_liminit_mm = range_min_mm;
            sensor->max_distance_liminit_mm = range_max_mm;
            snprintf(current_command->response, sizeof(current_command->response), "OK;TOF %u LIMITS set to MIN:%u mm MAX:%u mm\n", sensor_index, range_min_mm, range_max_mm);
            return 1;
        }

        //TOF GET_LIMITS [0-7]
        else if (strcmp(ACTION, "GET_LIMITS") == 0){
            uint16_t range_min_mm = sensor->min_distance_liminit_mm;
            uint16_t range_max_mm = sensor->max_distance_liminit_mm;
            snprintf(current_command->response, sizeof(current_command->response), "OK;TOF %u LIMITS MIN:%u mm MAX:%u mm\n", sensor_index, range_min_mm, range_max_mm);
            return 1;
        }

        //TOF GET_LAST_AUTOMATED_MEASUREMENT [0-7]
        else if (strcmp(ACTION, "GET_LAST_AUTOMATED_MEASUREMENT") == 0){
            uint16_t last_distance_mm = sensor->last_measured_distance_mm;
            snprintf(current_command->response, sizeof(current_command->response), "OK;TOF %u LAST_MEASUREMENT_mm:%u\n", sensor_index, last_distance_mm);
            return 1;
        }


        ////////////////////////// STATES THAT DOES NOT NEED REINITIALIZATION, BUT NEEDS TO BE MUTEXED
        osMutexWait(take_measurement_mutex, portMAX_DELAY); //Take mutex to ensure no measurements are being taken while we are reinitializing the I2C bus
        //TOF [0-7] SET_I2C_SPEED [SPEED IN HZ]
        if (strcmp(ACTION, "SET_I2C_SPEED") == 0){
            uint32_t speed_hz = strtoul(current_command->subcommands[2], NULL, 10);
            if (speed_hz == 0U){
                snprintf(current_command->response, sizeof(current_command->response), "ERROR;TOF SET_I2C_SPEED speed must be > 0\n");
                osMutexRelease(take_measurement_mutex);
                return 0;
            }

            if (speed_hz > 100000U)
            {
                snprintf(current_command->response, sizeof(current_command->response), "ERROR;TOF SET_I2C_SPEED speed must be <= 100000\n");
                osMutexRelease(take_measurement_mutex);
                return 0;
            }
            //UPDATE SENSOR STRUCT
            sensor->i2c_speed_Hz = speed_hz;
            HAL_StatusTypeDef  I2C_response;
            //Deinit current I2C bus, so next TOF action will reinit it with new speed
            I2C_response = HAL_I2C_DeInit(sensor->hi2c);
            if (I2C_response != HAL_OK){
                snprintf(current_command->response, sizeof(current_command->response), "ERROR;TOF %u SET_I2C_SPEED DeInit failed with HAL_ERROR:0x%08lX\n", sensor_index, (unsigned long)I2C_response);
                osMutexRelease(take_measurement_mutex);
                return 0;
            }
            //SET NEW SPEED
            osDelay(20);
            sensor->hi2c->Init.ClockSpeed = speed_hz;
            I2C_response = HAL_I2C_Init(sensor->hi2c);
            if (I2C_response != HAL_OK){
                snprintf(current_command->response, sizeof(current_command->response), "ERROR;TOF %u SET_I2C_SPEED Init failed with HAL_ERROR:0x%08lX\n", sensor_index, (unsigned long)I2C_response);
                osMutexRelease(take_measurement_mutex);
                return 0;
            }
            osDelay(20);
            snprintf(current_command->response, sizeof(current_command->response), "OK;TOF %u SET_I2C_SPEED:%u\n", sensor_index, speed_hz);
            osMutexRelease(take_measurement_mutex);
            return 1;
        }


        //////////////// COMMANDS THAT ACCUALY NEED TO COMMUNICATE WITH SENSOR AND NOT ONLY CHECK AUTOMEASUREMENT FLAGS ///////////////////
        //CHECK IF EXPANDER CHANNEL IS CORRECTLY SET TO THE SENSOR:
        if (PCA9548APWR_getSelectedChannel(&pca9548apwr_expander) != sensor_index){
            //IF CHANNEL IS NOT CORRECT, TRY TO SELECT THE CORRECT CHANNEL
            if(PCA9548APWR_selectChannel(&pca9548apwr_expander, sensor_index) != true)
            {
                snprintf(current_command->response, sizeof(current_command->response), "ERROR;TOF %u I2C expander channel is not set to the TOF sensor and failed to set it correctly HAL_ERROR:%u\n", sensor_index, pca9548apwr_expander.last_i2c_error);
                osMutexRelease(take_measurement_mutex);
                return 0;
            }  
        }

        //CHECK IF I2C BUS FOR SENSOR IS READY
        if (HAL_I2C_GetState(sensor->hi2c) != HAL_I2C_STATE_READY){
            GlobalRecoverStalledI2CBus(sensor->hi2c, sensor->scl_port, sensor->scl_pin, sensor->sda_port, sensor->sda_pin);
        }

        //CHECK IF THERE IS SENSOR AT THAT ADDRESS BEFORE TRYING TO OPERATE WITH IT
        if (HAL_I2C_IsDeviceReady(sensor->hi2c, (sensor->address << 1), 4, 500) != HAL_OK){
            //IF NOT TRY TO REINITLIZE THE I2C BUS
            GlobalRecoverStalledI2CBus(sensor->hi2c, sensor->scl_port, sensor->scl_pin, sensor->sda_port, sensor->sda_pin);
            //osDelay(100);

            //CHECK IF REINITIALIZATION HELPED
            if (HAL_I2C_IsDeviceReady(sensor->hi2c, (sensor->address << 1), 4, 500) != HAL_OK){
                snprintf(current_command->response, sizeof(current_command->response), "ERROR;TOF %u sensor not responding at address 0x%02X after I2C bus recovery attempts\n", sensor_index, sensor->address);
                osMutexRelease(take_measurement_mutex);
                return 0;
            }

        }

        
        //TOF [0-7] UNSTUCK_STALLED_I2C_BUS
        if (strcmp(ACTION, "UNSTUCK_STALLED_I2C_BUS") == 0){
            int32_t recovery_result = GlobalRecoverStalledI2CBus(sensor->hi2c, sensor->scl_port, sensor->scl_pin, sensor->sda_port, sensor->sda_pin);
            if (recovery_result != 0){
                snprintf(current_command->response, sizeof(current_command->response), "ERROR;TOF %u UNSTUCK_STALLED_I2C_BUS failed with error:%i\n", sensor_index, recovery_result);
                osMutexRelease(take_measurement_mutex);
                return 0;
            }
            snprintf(current_command->response, sizeof(current_command->response), "OK;TOF %u UNSTUCK_STALLED_I2C_BUS successful\n", sensor_index);
            osMutexRelease(take_measurement_mutex);
            return 1;
        }

        //TOF [0-7] INIT
        else if (strcmp(ACTION, "INIT") == 0){ 
            sensor_staus_result = TOF_initSensor(sensor);
            if (sensor_staus_result != 0){
                snprintf(current_command->response, sizeof(current_command->response), "ERROR;TOF %u INITSENSOR FAILED with error:%i\n", sensor_index, sensor_staus_result);
                osMutexRelease(take_measurement_mutex);
                return 0;
            }
            snprintf(current_command->response, sizeof(current_command->response), "OK;TOF %u INITSENSOR SUCCESSFUL\n", sensor_index);
            osMutexRelease(take_measurement_mutex);
            return 1;
        }

        

    
        //TOF [0-7] SET_MODE [SHORT/LONG/1/2]
        else if (strcmp(ACTION, "SET_MODE") == 0){
            if (current_command->number_of_subcommands < 3){
                snprintf(current_command->response, sizeof(current_command->response), "ERROR;TOF SET_MODE requires MODE\n");
                osMutexRelease(take_measurement_mutex);
                return 0;
            }

            uint16_t mode;
            if (CommandParser_ParseTofMode(current_command->subcommands[2], &mode) == 0){
                snprintf(current_command->response, sizeof(current_command->response), "ERROR;TOF SET_MODE MODE must be SHORT, LONG, 1 or 2\n");
                osMutexRelease(take_measurement_mutex);
                return 0;
            }

            sensor_staus_result = TOF_SetMode(sensor, mode);
            if (sensor_staus_result != 0){
                snprintf(current_command->response, sizeof(current_command->response), "ERROR;TOF %u SET_MODE FAILED with error:%i\n", sensor_index, sensor_staus_result);
                osMutexRelease(take_measurement_mutex);
                return 0;
            }
            snprintf(current_command->response, sizeof(current_command->response), "OK;TOF %u SET_MODE:%s\n", sensor_index, mode == 1U ? "SHORT" : "LONG");
            osMutexRelease(take_measurement_mutex);
            return 1;
        }

        //TOF [0-7] GET_BOOT_STATE
        else if (strcmp(ACTION, "GET_BOOT_STATE") == 0){
            uint8_t boot_state_result;
            sensor_staus_result = TOF_BootState(sensor, &boot_state_result);
            if (sensor_staus_result != 0){
                snprintf(current_command->response, sizeof(current_command->response), "ERROR;TOF %u GET_BOOT_STATE FAILED with error:%i\n", sensor_index, sensor_staus_result);
                osMutexRelease(take_measurement_mutex);
                return 0;
            }

            snprintf(current_command->response, sizeof(current_command->response), "OK;TOF %u BOOT_STATE:%d\n", sensor_index, boot_state_result);
            osMutexRelease(take_measurement_mutex);
            return 1;
        }

        //TOF [0-7] SET_INTER_MEASUREMENT_PERIOD [PERIOD_MS]
        else if (strcmp(ACTION, "SET_INTER_MEASUREMENT_DELAY") == 0){
            uint32_t period_ms = strtoul(current_command->subcommands[2], NULL, 10);
            if (period_ms == 0U){
                snprintf(current_command->response, sizeof(current_command->response), "ERROR;TOF SET_INTER_MEASUREMENT_DELAY period must be > 0\n");
                osMutexRelease(take_measurement_mutex);
                return 0;
            }
            sensor_staus_result = TOF_SetInterMeasurementInMs(sensor, period_ms);
            if (sensor_staus_result != 0){
                snprintf(current_command->response, sizeof(current_command->response), "ERROR;TOF %u SET_INTER_MEASUREMENT_DELAY FAILED with error:%i\n", sensor_index, sensor_staus_result);
                osMutexRelease(take_measurement_mutex);
                return 0;
            }
            snprintf(current_command->response, sizeof(current_command->response), "OK;TOF %u SET_INTER_MEASUREMENT_DELAY:%u ms\n", sensor_index, period_ms);
            osMutexRelease(take_measurement_mutex);
            return 1;
        }


        //TOF [0-7] SET_TIMING_BUDGET [BUDGET_MS]
        else if (strcmp(ACTION, "SET_TIMING_BUDGET") == 0){
            uint32_t budget_ms = strtoul(current_command->subcommands[2], NULL, 10);
            if (budget_ms == 0U){
                snprintf(current_command->response, sizeof(current_command->response), "ERROR;TOF SET_TIMING_BUDGET budget must be > 0\n");
                osMutexRelease(take_measurement_mutex);
                return 0;
            }
            sensor_staus_result = TOF_SetTimingBudget(sensor, budget_ms);
            if (sensor_staus_result != 0){
                snprintf(current_command->response, sizeof(current_command->response), "ERROR;TOF %u SET_TIMING_BUDGET FAILED with error:%i\n", sensor_index, sensor_staus_result);
                osMutexRelease(take_measurement_mutex);
                return 0;
            }
            snprintf(current_command->response, sizeof(current_command->response), "OK;TOF %u SET_TIMING_BUDGET:%u ms\n", sensor_index, budget_ms);
            osMutexRelease(take_measurement_mutex);
            return 1;
        }

        //TOF [0-7] SET_ROI [X] [Y]
        else if (strcmp(ACTION, "SET_ROI") == 0){
            if (current_command->number_of_subcommands < 4){
                snprintf(current_command->response, sizeof(current_command->response), "ERROR;TOF SET_ROI requires X and Y\n");
                osMutexRelease(take_measurement_mutex);
                return 0;
            }

            uint8_t roi_x = (uint8_t)atoi(current_command->subcommands[2]);
            uint8_t roi_y = (uint8_t)atoi(current_command->subcommands[3]);
            sensor_staus_result = TOF_SetROI(sensor, roi_x, roi_y);
            if (sensor_staus_result != 0){
                snprintf(current_command->response, sizeof(current_command->response), "ERROR;TOF %u SET_ROI FAILED with error:%i\n", sensor_index, sensor_staus_result);
                osMutexRelease(take_measurement_mutex);
                return 0;
            }
            snprintf(current_command->response, sizeof(current_command->response), "OK;TOF %u SET_ROI X:%u Y:%u\n", sensor_index, roi_x, roi_y);
            osMutexRelease(take_measurement_mutex);
            return 1;
        }

        //TOF [0-7] SET_DISTANCE_TRESHOLD [MIN_MM] [MAX_MM] [0:bellow|1:above|2:out|3:in] [IntOnNoTarget]
        else if (strcmp(ACTION, "SET_DISTANCE_TRESHOLD") == 0){
            if (current_command->number_of_subcommands < 6){
                snprintf(current_command->response, sizeof(current_command->response), "ERROR;TOF SET_DISTANCE_TRESHOLD requires MIN_MM, MAX_MM, WINDOW_MM and IntOnNoTarget\n");
                osMutexRelease(take_measurement_mutex);
                return 0;
            }

            uint16_t min_mm = (uint16_t)strtoul(current_command->subcommands[2], NULL, 10);
            uint16_t max_mm = (uint16_t)strtoul(current_command->subcommands[3], NULL, 10);
            uint16_t window_mm = (uint16_t)strtoul(current_command->subcommands[4], NULL, 10);
            uint8_t int_on_no_target = atoi(current_command->subcommands[5]);
            sensor_staus_result = TOF_SetDistanceThreshold(sensor, min_mm, max_mm, window_mm, int_on_no_target);
            if (sensor_staus_result != 0){
                snprintf(current_command->response, sizeof(current_command->response), "ERROR;TOF %u SET_DISTANCE_TRESHOLD FAILED with error:%i\n", sensor_index, sensor_staus_result);
                osMutexRelease(take_measurement_mutex);
                return 0;
            }
            snprintf(current_command->response, sizeof(current_command->response), "OK;TOF %u SET_DISTANCE_TRESHOLD MIN:%u mm MAX:%u mm WINDOW:%u mm IntOnNoTarget:%d\n", sensor_index, min_mm, max_mm, window_mm, int_on_no_target);
            osMutexRelease(take_measurement_mutex);
            return 1;
        }

        else if (strcmp(ACTION, "GET_DISTANCE_TRESHOLD") == 0){
        
            uint16_t min_mm;
            uint16_t max_mm;
            uint16_t window_mm;

            sensor_staus_result = TOF_GetDistanceThresholdLow(sensor, &min_mm);
            sensor_staus_result |= TOF_GetDistanceThresholdHigh(sensor, &max_mm);
            sensor_staus_result |= TOF_GetDistanceThresholdWindow(sensor, &window_mm);

            snprintf(current_command->response, sizeof(current_command->response), "OK;TOF %u GET_DISTANCE_TRESHOLD MIN:%u mm MAX:%u mm WINDOW:%u mm\n", sensor_index, min_mm, max_mm, window_mm);
            osMutexRelease(take_measurement_mutex);
            return 1;
        }


        //TOF [0-7] CLEAR_INNTERUPT
        else if (strcmp(ACTION, "CLEAR_INTERRUPT") == 0){
            sensor_staus_result = TOF_CleanInterrupt(sensor);
            if (sensor_staus_result != 0){
                snprintf(current_command->response, sizeof(current_command->response), "ERROR;TOF %u CLEAR_INTERRUPT FAILED with error:%i\n", sensor_index, sensor_staus_result);
                osMutexRelease(take_measurement_mutex);
                return 0;
            }
            snprintf(current_command->response, sizeof(current_command->response), "OK;TOF %u CLEAR_INTERRUPT successful\n", sensor_index);
            osMutexRelease(take_measurement_mutex);
            return 1;
        }


        //TOF [0-7] MEASURE_DISTANCE [TIMEOUT_MS]
        else if (strcmp(ACTION, "MEASURE_DISTANCE") == 0){
            uint32_t default_timeout_ms = 1000;
            if (current_command->number_of_subcommands >= 3){
                default_timeout_ms = strtoul(current_command->subcommands[2], NULL, 10);
                if (default_timeout_ms == 0U){
                    snprintf(current_command->response, sizeof(current_command->response), "ERROR;TOF MEASURE_DISTANCE timeout must be > 0\n");
                    osMutexRelease(take_measurement_mutex);
                    return 0;
                }
            }

            uint16_t distance_mm;
            sensor_staus_result = TOF_GetDistance(sensor, default_timeout_ms, &distance_mm);
            if (sensor_staus_result != 0){
                snprintf(current_command->response, sizeof(current_command->response), "ERROR;TOF %u MEASURE_DISTANCE FAILED with error:%i\n", sensor_index, sensor_staus_result);
                osMutexRelease(take_measurement_mutex);
                return 0;
            }

            snprintf(current_command->response, sizeof(current_command->response), "OK;TOF %u DISTANCE_mm:%u\n", sensor_index, distance_mm);
            osMutexRelease(take_measurement_mutex);
            return 1;
        }

        //TOF [0-7] DO_LIMIT_CALIBRATION [TIME_OF_CALIBRATION_MS] [FINAL_LIMIT_OFFSET_PERCENT]
        
        else if (strcmp(ACTION, "DO_LIMIT_CALIBRATION") == 0){
            if (current_command->number_of_subcommands < 4){
                snprintf(current_command->response, sizeof(current_command->response), "ERROR;TOF DO_LIMIT_CALIBRATION requires SENSOR_INDEX, TIME_OF_CALIBRATION_MS and FINAL_LIMIT_OFFSET_PERCENT\n");
                osMutexRelease(take_measurement_mutex); //Release mutex before returning
                return 0;
            }

            uint32_t calibration_time_ms = strtoul(current_command->subcommands[2], NULL, 10);
            uint8_t final_limit_offset_percent = (uint8_t)atoi(current_command->subcommands[3]);
            if (final_limit_offset_percent > 100U){
                snprintf(current_command->response, sizeof(current_command->response), "ERROR;TOF DO_LIMIT_CALIBRATION FINAL_LIMIT_OFFSET_PERCENT must be between 0 and 100\n");
                osMutexRelease(take_measurement_mutex); //Release mutex before returning
                return 0;
            }

            int32_t calibration_result = TOF_calibrate_to_distance(sensor, calibration_time_ms, final_limit_offset_percent);
            if (calibration_result != 0){
                snprintf(current_command->response, sizeof(current_command->response), "ERROR;TOF %u DO_LIMIT_CALIBRATION failed with error:%i\n", sensor_index, calibration_result);
                osMutexRelease(take_measurement_mutex); //Release mutex before returning
                return 0;
            }

            snprintf(current_command->response, sizeof(current_command->response), "OK;TOF %u DO_LIMIT_CALIBRATION successful, min_limit:%i, max_limit:%i\n", 
            sensor_index, 
            sensor->min_distance_liminit_mm,
            sensor->max_distance_liminit_mm);
            osMutexRelease(take_measurement_mutex); //Release mutex after we are done with limit calibration
            return 1;
        }
        
        //TOF [0-7] CHECK_IF_MATCHES_WINDOW [DISTANCE_MM]
        else if (strcmp(ACTION, "CHECK_IF_MATCHES_WINDOW") == 0){
            uint8_t windowRegister;
            sensor_staus_result = TOF_check_Window_critera_match(sensor, &windowRegister);
            if (sensor_staus_result != 0){
                snprintf(current_command->response, sizeof(current_command->response), "ERROR;TOF %u CHECK_IF_MATCHES_WINDOW failed with error:%i\n", sensor_index, sensor_staus_result);
                osMutexRelease(take_measurement_mutex);
                return 0;
            }

            snprintf(current_command->response, sizeof(current_command->response), "OK;TOF %u CHECK_IF_MATCHES_WINDOW windowRegister:%u\n", sensor_index, windowRegister);
            osMutexRelease(take_measurement_mutex);
            return 1;
        }

        //TOF [0-7] CHECK_INTERRUPT
        else if (strcmp(ACTION, "CHECK_INTERRUPT") == 0){
            uint8_t is_interrupt_active = 0;
            sensor_staus_result = TOF_check_for_GPIO_interrupt(sensor, &is_interrupt_active);
            if (sensor_staus_result != 0){
                snprintf(current_command->response, sizeof(current_command->response), "ERROR;TOF %u CHECK_INTERRUPT failed with error:%i\n", sensor_index, sensor_staus_result);
                osMutexRelease(take_measurement_mutex);
                return 0;
            }

            snprintf(current_command->response, sizeof(current_command->response), "OK;TOF %u CHECK_INTERRUPT active:%d\n", sensor_index, is_interrupt_active);
            osMutexRelease(take_measurement_mutex);
            return 1;
        }

        //TOF [0-7] GET_INFO
        else if (strcmp(ACTION, "GET_INFO") == 0){
            //RETURNS:
            //BOOT STATE
            //MIN LIMIT
            //MAX LIMIT
            //DISTANCE TRESHOLD FOR INTTERRUPT MIN
            //DISTANCE TRESHOLD FOR INTTERRUPT MAX
            //WINDOW FOR INTTERRUPT
            //TIMING BUDGET
            //INTER MEASUREMENT DELAY
            //OPERATION MODE (LONG/SHORT)

            uint8_t boot_state_result;
            uint16_t min_limit_mm = sensor->min_distance_liminit_mm;
            uint16_t max_limit_mm = sensor->max_distance_liminit_mm;
            uint16_t distance_threshold_min_mm;
            uint16_t distance_threshold_max_mm;
            uint16_t distance_threshold_window_mm;
            uint16_t timing_budget_ms;
            uint32_t inter_measurement_delay_ms;
            uint16_t distance_mode;
            sensor_staus_result = TOF_BootState(sensor, &boot_state_result);
            sensor_staus_result |= TOF_GetDistanceThresholdLow(sensor, &distance_threshold_min_mm);
            sensor_staus_result |= TOF_GetDistanceThresholdHigh(sensor, &distance_threshold_max_mm);
            sensor_staus_result |= TOF_GetDistanceThresholdWindow(sensor, &distance_threshold_window_mm);
            sensor_staus_result |= TOF_GetTimingBudget(sensor, &timing_budget_ms);
            sensor_staus_result |= TOF_GetInterMeasurementInMs(sensor, &inter_measurement_delay_ms);
            sensor_staus_result |= TOF_GetDistanceMode(sensor, &distance_mode);
            if (sensor_staus_result != 0){
                snprintf(current_command->response, sizeof(current_command->response), "ERROR;TOF %u GET_INFO failed with error:%i\n", sensor_index, sensor_staus_result);
                osMutexRelease(take_measurement_mutex);
                return 0;
            }

            snprintf(current_command->response, sizeof(current_command->response), "OK;TOF %u INFO BOOT_STATE:%d MIN_LIMIT:%u mm MAX_LIMIT:%u mm DIST_THRESHOLD_MIN:%u mm DIST_THRESHOLD_MAX:%u mm DIST_THRESHOLD_WINDOW:%u mm TIMING_BUDGET:%u ms INTER_MEASUREMENT_DELAY:%u ms DISTANCE_MODE:%u\n",
            sensor_index, boot_state_result, min_limit_mm, max_limit_mm, distance_threshold_min_mm, distance_threshold_max_mm, distance_threshold_window_mm, timing_budget_ms, inter_measurement_delay_ms, distance_mode);
            osMutexRelease(take_measurement_mutex);
            return 1;
        }

        //TOF [0-7] RAW_I2C_READ_BYTE [REGISTER_ADDRESS]
        else if (strcmp(ACTION, "RAW_I2C_READ_BYTE") == 0){
            uint8_t register_address = (uint8_t)strtoul(current_command->subcommands[2], NULL, 10);
            uint8_t read_value;
            sensor_staus_result = VL53L1_RdByte(sensor->hi2c, sensor->address, register_address, &read_value, 1000);
            if (sensor_staus_result != 0){
                snprintf(current_command->response, sizeof(current_command->response), "ERROR;TOF %u RAW_I2C_READ_BYTE failed with error:%i\n", sensor_index, sensor_staus_result);
                osMutexRelease(take_measurement_mutex);
                return 0;
            }

            snprintf(current_command->response, sizeof(current_command->response), "OK;TOF %u RAW_I2C_READ_BYTE REG:0x%02X VALUE:0x%02X\n", sensor_index, register_address, read_value);
            osMutexRelease(take_measurement_mutex);
            return 1;
        }

        //TOF [0-7] RAW_I2C_READ_WORD [REGISTER_ADDRESS]

        else if (strcmp(ACTION, "RAW_I2C_READ_WORD") == 0){
            uint8_t register_address = (uint8_t)strtoul(current_command->subcommands[2], NULL, 10);
            uint16_t read_value;
            sensor_staus_result = VL53L1_RdWord(sensor->hi2c, sensor->address, register_address, &read_value, 1000);
            if (sensor_staus_result != 0){
                snprintf(current_command->response, sizeof(current_command->response), "ERROR;TOF %u RAW_I2C_READ_WORD failed with error:%i\n", sensor_index, sensor_staus_result);
                osMutexRelease(take_measurement_mutex);
                return 0;
            }

            snprintf(current_command->response, sizeof(current_command->response), "OK;TOF %u RAW_I2C_READ_WORD REG:0x%02X VALUE:0x%04X\n", sensor_index, register_address, read_value);
            osMutexRelease(take_measurement_mutex);
            return 1;
        }

        //TOF [0-7] RAW_I2C_WRITE_BYTE [REGISTER_ADDRESS] [VALUE]
        else if (strcmp(ACTION, "RAW_I2C_WRITE_BYTE") == 0){
            uint8_t register_address = (uint8_t)strtoul(current_command->subcommands[2], NULL, 10);
            uint8_t write_value = (uint8_t)strtoul(current_command->subcommands[3], NULL, 10);
            sensor_staus_result = VL53L1_WrByte(sensor->hi2c, sensor->address, register_address, write_value, 1000);
            if (sensor_staus_result != 0){
                snprintf(current_command->response, sizeof(current_command->response), "ERROR;TOF %u RAW_I2C_WRITE_BYTE failed with error:%i\n", sensor_index, sensor_staus_result);
                osMutexRelease(take_measurement_mutex);
                return 0;
            }

            snprintf(current_command->response, sizeof(current_command->response), "OK;TOF %u RAW_I2C_WRITE_BYTE REG:0x%02X VALUE:0x%02X\n", sensor_index, register_address, write_value);
            osMutexRelease(take_measurement_mutex);
            return 1;
        }


        //TOF [0-7] RAW_I2C_WRITE_WORD [REGISTER_ADDRESS] [VALUE]
        else if (strcmp(ACTION, "RAW_I2C_WRITE_WORD") == 0){
            uint8_t register_address = (uint8_t)strtoul(current_command->subcommands[2], NULL, 10);
            uint16_t write_value = (uint16_t)strtoul(current_command->subcommands[3], NULL, 10);
            sensor_staus_result = VL53L1_WrWord(sensor->hi2c, sensor->address, register_address, write_value, 1000);
            if (sensor_staus_result != 0){
                snprintf(current_command->response, sizeof(current_command->response), "ERROR;TOF %u RAW_I2C_WRITE_WORD failed with error:%i\n", sensor_index, sensor_staus_result);
                osMutexRelease(take_measurement_mutex);
                return 0;
            }

            snprintf(current_command->response, sizeof(current_command->response), "OK;TOF %u RAW_I2C_WRITE_WORD REG:0x%02X VALUE:0x%04X\n", sensor_index, register_address, write_value);
            osMutexRelease(take_measurement_mutex);
            return 1;
        }

        //TOF [0-7] SET_100HZ_MODE
        else if (strcmp(ACTION, "SET_100HZ_MODE") == 0){
            sensor_staus_result = TOF_Set100HzMeasurementMode(sensor);
            if (sensor_staus_result != 0){
                snprintf(current_command->response, sizeof(current_command->response), "ERROR;TOF %u SET_100HZ_MODE failed with error:%i\n", sensor_index, sensor_staus_result);
                osMutexRelease(take_measurement_mutex);
                return 0;
            }
            snprintf(current_command->response, sizeof(current_command->response), "OK;TOF %u SET_100HZ_MODE successful\n", sensor_index);
            osMutexRelease(take_measurement_mutex);
            return 1;
        }


        snprintf(current_command->response, sizeof(current_command->response), "ERROR;No such TOF action\n");
        osMutexRelease(take_measurement_mutex); //Release mutex before returning
        return 0;
    }

    
    
    //SET_EXT_PIN [1-8] [0/1]
    else if (strcmp(current_command->command,"SET_EXT_PIN") == 0){
        uint8_t pin = atoi(current_command->subcommands[0]);
        uint8_t pin_value = atoi(current_command->subcommands[1]);
        if (pin == 1){
            HAL_GPIO_WritePin(EXTERNAL_PORT_MAPPING.EXT_PIN_1_port, EXTERNAL_PORT_MAPPING.EXT_PIN_1_pin, pin_value);
        }
        else if (pin == 2){
            HAL_GPIO_WritePin(EXTERNAL_PORT_MAPPING.EXT_PIN_2_port, EXTERNAL_PORT_MAPPING.EXT_PIN_2_pin, pin_value);
        }
        else if (pin == 3){
            HAL_GPIO_WritePin(EXTERNAL_PORT_MAPPING.EXT_PIN_3_port, EXTERNAL_PORT_MAPPING.EXT_PIN_3_pin, pin_value);
        }
        else if (pin == 4){
            HAL_GPIO_WritePin(EXTERNAL_PORT_MAPPING.EXT_PIN_4_port, EXTERNAL_PORT_MAPPING.EXT_PIN_4_pin, pin_value);
        }
        else if (pin == 5){
            HAL_GPIO_WritePin(EXTERNAL_PORT_MAPPING.EXT_PIN_5_port, EXTERNAL_PORT_MAPPING.EXT_PIN_5_pin, pin_value);
        }
        else if (pin == 6){
            HAL_GPIO_WritePin(EXTERNAL_PORT_MAPPING.EXT_PIN_6_port, EXTERNAL_PORT_MAPPING.EXT_PIN_6_pin, pin_value);
        }
        else if (pin == 7){
            HAL_GPIO_WritePin(EXTERNAL_PORT_MAPPING.EXT_PIN_7_port, EXTERNAL_PORT_MAPPING.EXT_PIN_7_pin, pin_value);
        }
        else if (pin == 8){
            HAL_GPIO_WritePin(EXTERNAL_PORT_MAPPING.EXT_PIN_8_port, EXTERNAL_PORT_MAPPING.EXT_PIN_8_pin, pin_value);
        }
        else{
            snprintf(current_command->response, sizeof(current_command->response), "ERROR;No such pin %d\n", pin);
            return 0;
        }
        
        snprintf(current_command->response, sizeof(current_command->response), "OK;SET_EXT_PIN PIN:%d VALUE:%d\n", pin, pin_value);
    }

    //EXT_SET_EN_PIN [0/1]
    else if (strcmp(current_command->command,"EXT_SET_EN_PIN") == 0){
        uint8_t pin_state = atoi(current_command->subcommands[0]);
        HAL_GPIO_WritePin(EXTERNAL_PORT_MAPPING.EXT_OE_port, EXTERNAL_PORT_MAPPING.EXT_OE_pin, pin_state);
        snprintf(current_command->response, sizeof(current_command->response), "OK;EXT_SET_EN_PIN VALUE:%d\n", pin_state);

    }

    //EXT_32_RELAY_CARD [RELAY|PORT] [...]
    else if (strcmp(current_command->command,"EXT_32_RELAY_CARD") == 0){
        //EXT_32_RELAY_CARD RELAY [1-32] [0/1]
        if (strcmp(current_command->subcommands[0],"RELAY") == 0){
            uint8_t relay_number = atoi(current_command->subcommands[1])-1;//RELAY NUMBERS PROGRAMATICALY INDEXED FROM 0 AT SILKSCREEN INDEXED FROM 1
            uint8_t relay_state = atoi(current_command->subcommands[2]);
            relayCardSetSingleRelay(relay_number,relay_state,&EXTERNAL_32_RELAY_CARD);
            snprintf(current_command->response, sizeof(current_command->response), "OK;EXT_32_RELAY_CARD NEWPORTVALUE:%d, RELAY:%u SET TO VALUE:%d\n", relay_number, EXTERNAL_32_RELAY_CARD.current_satus, relay_state);
            return 0;
        }


        //EXT_32_RELAY_CARD PORT [VALUE]
        if (strcmp(current_command->subcommands[0],"PORT") == 0){
            uint32_t portValue = strtoull(current_command->subcommands[1],NULL,10);
           
            relayCardSetPort(portValue,&EXTERNAL_32_RELAY_CARD);
            snprintf(current_command->response, sizeof(current_command->response), "OK;EXT_32_RELAY_CARD SET TO PORT VALUE:%u\n", EXTERNAL_32_RELAY_CARD.current_satus);
            return 0;
        }
        else{
            snprintf(current_command->response, sizeof(current_command->response), "ERROR;No such subcommand\n");
            return 0;
        }
    }

    //DPDT_RELAYS [SET_PORT|SET_RELAY] [...]
    else if (strcmp(current_command->command,"DPDT_RELAYS") == 0){

        //DPDT_RELAYS SET_PORT [VALUE]
        if (strcmp(current_command->subcommands[0],"SET_PORT") == 0){
            uint32_t port_value;
            port_value = strtoull(current_command->subcommands[1],NULL,10);
            RELAY_DRIVER_setRelayPort(port_value,&RELAY_IO_DRIVER);   
            snprintf(current_command->response, sizeof(current_command->response), "OK;DPDT_RELAYS SET_PORT:%u\n", port_value);    
        }

        //DPDT_RELAYS SET_RELAY [INDEX] [0/1]
        else if (strcmp(current_command->subcommands[0],"SET_RELAY") == 0){
            uint8_t relay_number = atoi(current_command->subcommands[1]);
            uint8_t relay_state = atoi(current_command->subcommands[2]);
            RELAY_DRIVER_setRelay(relay_number,relay_state,&RELAY_IO_DRIVER);
            snprintf(current_command->response, sizeof(current_command->response), "OK;DPDT_RELAYS SET_RELAY:%d TO VALUE:%d\n", relay_number, relay_state);
        }

        else{
            snprintf(current_command->response, sizeof(current_command->response), "ERROR;No such subcommand\n");
            return 0;
        }

    }

    //VENTILS [SET_PORT|SET_VENTIL] [...]
    else if (strcmp(current_command->command,"VENTILS") == 0){

        //VENTILS SET_PORT [VALUE]
        if (strcmp(current_command->subcommands[0],"SET_PORT") == 0){
            uint32_t port_value;
            port_value = strtoull(current_command->subcommands[1],NULL,10);
            RELAY_DRIVER_setVentilPort(port_value,&RELAY_IO_DRIVER);   
            snprintf(current_command->response, sizeof(current_command->response), "OK;VENTILS SET_PORT:%u\n", port_value);    
        }

        //VENTILS SET_VENTIL [INDEX] [0/1]
        else if (strcmp(current_command->subcommands[0],"SET_VENTIL") == 0){
            uint8_t ventil_number = atoi(current_command->subcommands[1]);
            uint8_t ventil_state = atoi(current_command->subcommands[2]);
            RELAY_DRIVER_setVentil(ventil_number,ventil_state,&RELAY_IO_DRIVER);
            snprintf(current_command->response, sizeof(current_command->response), "OK;VENTILS SET_VENTIL:%d TO VALUE:%d\n", ventil_number, ventil_state);
        }

        else{
            snprintf(current_command->response, sizeof(current_command->response), "ERROR;No such subcommand\n");
            return 0;
        }

    }

    //INPUTS [READ_ALL|READ_INPUT] [...]
    else if (strcmp(current_command->command,"INPUTS") == 0){

        //INPUTS READ_ALL
        if (strcmp(current_command->subcommands[0],"READ_ALL") == 0){
            uint64_t port_value;
            port_value = RELAY_DRIVER_readInputsPort(&RELAY_IO_DRIVER);   
            snprintf(current_command->response, sizeof(current_command->response), "OK;INPUTS READ PORT VALUE:%u\n", (uint32_t)port_value);    
        }

        //INPUTS READ_INPUT [INDEX]
        else if (strcmp(current_command->subcommands[0],"READ_INPUT") == 0){
            uint8_t pin_number = atoi(current_command->subcommands[1]);
            uint8_t pin_state;
            pin_state = RELAY_DRIVER_readInput(pin_number,&RELAY_IO_DRIVER);
            snprintf(current_command->response, sizeof(current_command->response), "OK;INPUT PIN:%d VALUE:%d\n", pin_number, pin_state);
        }

        else{
            snprintf(current_command->response, sizeof(current_command->response), "ERROR;No such subcommand\n");
            return 0;
        }

    }
    //ADC2_IN3? [NUMBER_OF_SAMPLES]
    else if (strcmp(current_command->command,"ADC2_IN3?") == 0)
    {
        //READ ADC VALUE FROM ADC2 CHANNEL 3
        static const uint8_t voltage_divider = 3;
        static const uint16_t adc_max_value = 4095;
        static const uint16_t reference_voltage_mv = 3300;
        ADC_ChannelConfTypeDef config = {0};
        uint32_t nr_of_samples = strtoul(current_command->subcommands[0], NULL, 10);
        
        config.Channel = ADC_CHANNEL_3;
        config.Rank = 1;
        config.SamplingTime = ADC_SAMPLETIME_480CYCLES;

        if (HAL_ADC_ConfigChannel(&hadc2, &config) != HAL_OK)
        {
            Error_Handler();
        }

        uint32_t filtered_value = 0;

        for (uint32_t i = 0; i < nr_of_samples; i++)
        {
            //START CONVERSION
            HAL_ADC_Start(&hadc2);
            HAL_ADC_PollForConversion(&hadc2, 100);
            uint32_t adc_value = HAL_ADC_GetValue(&hadc2);
            filtered_value += adc_value;
            HAL_ADC_Stop(&hadc2);
        }
        

        uint32_t voltage = (filtered_value / nr_of_samples) * reference_voltage_mv / adc_max_value * voltage_divider;
        snprintf(current_command->response, sizeof(current_command->response), "OK;ADC2_IN3:%u,Voltage:%u mV\n", filtered_value / nr_of_samples, voltage);
    }

    //ADC2_IN6? [NUMBER_OF_SAMPLES]
    else if (strcmp(current_command->command,"ADC2_IN6?") == 0)
    {
        //READ ADC VALUE FROM ADC2 CHANNEL 6
        static const uint8_t voltage_divider = 2;
        static const uint16_t adc_max_value = 4095;
        static const uint16_t reference_voltage_mv = 3300;
        ADC_ChannelConfTypeDef config = {0};
        uint32_t nr_of_samples = strtoul(current_command->subcommands[0], NULL, 10);

        config.Channel = ADC_CHANNEL_6;
        config.Rank = 1;
        config.SamplingTime = ADC_SAMPLETIME_480CYCLES;
        if (HAL_ADC_ConfigChannel(&hadc2, &config) != HAL_OK)
        {
            Error_Handler();
        }

        uint32_t filtered_value = 0;
        for (uint32_t i = 0; i < nr_of_samples; i++)
        {
            //START CONVERSION
            HAL_ADC_Start(&hadc2);
            HAL_ADC_PollForConversion(&hadc2, 100);
            uint32_t adc_value = HAL_ADC_GetValue(&hadc2);
            filtered_value += adc_value;
            HAL_ADC_Stop(&hadc2);
        }

        uint32_t voltage = (filtered_value / nr_of_samples) * reference_voltage_mv / adc_max_value * voltage_divider;
        snprintf(current_command->response, sizeof(current_command->response), "OK;ADC2_IN6:%u,Voltage:%u mV\n", filtered_value / nr_of_samples, voltage);
    }

    //EEPROM <handled by EEPROMInputsParser>
    else if (strcmp(current_command->command,"EEPROM") == 0)
    {
        HAL_GPIO_WritePin(EEPROM_WP_GPIO_Port, EEPROM_WP_Pin, GPIO_PIN_RESET);
        EEPROMInputsParser(current_command);
        HAL_GPIO_WritePin(EEPROM_WP_GPIO_Port, EEPROM_WP_Pin, GPIO_PIN_SET);
    }

    //*IP?
    else if (strcmp(current_command->command,"*IP?") == 0)
    {
        snprintf(current_command->response, sizeof(current_command->response), "OK;IP address:%s\n", ip4addr_ntoa(netif_ip4_addr(&gnetif)));
    }

    //*INFO?
    else if (strcmp(current_command->command,"*INFO?") == 0){
        char ip_address[16];
        char subnet_address[16];
        char gateway_address[16];

        ip4addr_ntoa_r(netif_ip4_addr(&gnetif), ip_address, sizeof(ip_address));
        ip4addr_ntoa_r(netif_ip4_netmask(&gnetif), subnet_address, sizeof(subnet_address));
        ip4addr_ntoa_r(netif_ip4_gw(&gnetif), gateway_address, sizeof(gateway_address));

        snprintf(current_command->response,
                 sizeof(current_command->response),
                 "OK;RAFIONTECH s.r.o. - E0000010(FUT CTRL BOARD) V1.0,\r"
                 "build date:%s,"
                 "IP addresing mode: Static IPV4,"
                 "MAC:%02X::%02X::%02X::%02X::%02X::%02X,"
                 "IP:%s,"
                 "SUBNET:%s,"
                 "GATEWAY:%s,"
                 "PORT:%d\n",
                 __DATE__,
                 gnetif.hwaddr[0], gnetif.hwaddr[1], gnetif.hwaddr[2],
                 gnetif.hwaddr[3], gnetif.hwaddr[4], gnetif.hwaddr[5],
                 ip_address,
                 subnet_address,
                 gateway_address,
                 TCP_SERVER_PORT);
    }

    else{
        snprintf(current_command->response, sizeof(current_command->response), "ERROR;No such command\n");
        return 0;
    }
    
    return 1;
}

uint8_t updateCommandStruct(char* command_str, commandTemplate* current_command, enum CommandParser_source_of_commands source){
	//takes command, splits it into subcommands and saves them into list_of_commands -> returns list_length
    // IN CASE OF source == TCP_THREAD before calling this function it is mandatory to manualy fill TCP_client_netconn to commandTemplate

    current_command->command[0] = '\0';
	current_command->number_of_subcommands = 0;
	char* token;
    char* saveptr = NULL;
	current_command->delimeter = DELIMETER;
	current_command->source = source;
    token = strtok_r(command_str, current_command->delimeter, &saveptr);
    if (token == NULL){
        return 0;
    }
    if (strlen(token) >= MAXIMUM_COMMAND_LENGTH){
        return 0;
    }
	strcpy(current_command->command, token);
	uint8_t i = 0;
    while (i < MAXIMUM_NUMBER_OF_SUBCOMMANDS){
        token = strtok_r(NULL, current_command->delimeter, &saveptr);
        if (token == NULL){
            break;
        }
        if (strlen(token) >= MAXIMUM_SUBCOMMAND_LENGTH){
            return 0;
        }
        strcpy(current_command->subcommands[i], token);
        i++;
	}

    if (strtok_r(NULL, current_command->delimeter, &saveptr) != NULL){
        current_command->number_of_subcommands = 0;
        current_command->command[0] = '\0';
        return 0;
    }

    current_command->number_of_subcommands = i;
	return 1;
} 
