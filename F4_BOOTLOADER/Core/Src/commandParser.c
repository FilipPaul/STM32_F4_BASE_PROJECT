#include "commandParser.h"
#include "string.h"
#include "stdio.h"
#include "stdlib.h"
#include "globals.h"
#include "main.h"
#include "lwip/opt.h"
#include "lwip/api.h"
#include "lwip/sys.h"
#include "lwip.h"
#include "IPsettings.h"
#include "EEPROM_24AA02E48T_I_IOT_settings.h"
#include "tcpserver.h"

extern struct netif gnetif;
extern osMutexId take_measurement_mutex;

/////// ADDRESSES FOR USB DFU 
#define USB_BOOT_ADDR	0x1FFF0000U	// STM32F407 system memory (ROM bootloader) base address
#define	MCU_IRQS	70u	// no. of NVIC IRQ inputs

struct boot_vectable_ {
    uint32_t Initial_SP;
    void (*Reset_Handler)(void);
};

#define BOOTVTAB	((struct boot_vectable_ *)USB_BOOT_ADDR)


#define APPLICATION_ADDRESS 0x08020000U

typedef void (*pFunction)(void);

void JumpToApplication(void)
{
    uint32_t appStack;
    uint32_t appResetHandlerAddress;
    pFunction appResetHandler;

    // 1. Basic validity check (stack must be in RAM range)
    appStack = *(__IO uint32_t*)APPLICATION_ADDRESS;

    if (appStack < 0x20000000 || appStack > (0x20000000 + 128*1024))    {
        // invalid app
        printf("Invalid application stack pointer: 0x%08lX\n", appStack);
        return;
    }
    // 9. Get reset handler
    appResetHandlerAddress = *(__IO uint32_t*)(APPLICATION_ADDRESS + 4U);
    appResetHandler = (pFunction)appResetHandlerAddress;

    // 2. Disable interrupts FIRST (important)
    __disable_irq();

    // 3. Stop SysTick completely
    SysTick->CTRL = 0;
    SysTick->LOAD = 0;
    SysTick->VAL  = 0;

    // 4. Disable all NVIC interrupts
    for (int i = 0; i < 8; i++)
    {
        NVIC->ICER[i] = 0xFFFFFFFF;
        NVIC->ICPR[i] = 0xFFFFFFFF;
    }

    // 5. Deinit HAL AFTER interrupts are off
    HAL_DeInit();

    // 6. Set vector table EARLY
    SCB->VTOR = APPLICATION_ADDRESS;

    // 7. Data/instruction sync barrier (CRITICAL)
    __DSB();
    __ISB();

    // 8. Set MSP from application
    __set_MSP(appStack);

    // 10. Jump
    appResetHandler();
}

void JumpToUsbBootloader(void)
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
        snprintf(current_command->response, sizeof(current_command->response), "OK;RAFIONTECH s.r.o. - F4 BOOTLOADER V1.0\n");
    }

    //*ENTER_USB_BOOTLOADER
    else if (strcmp(current_command->command,"*ENTER_USB_BOOTLOADER") == 0){
        printf("OK;GOING TO BOOTLOADER\n");
        //WAIT UNTIL PRINTF FINISHES
        HAL_Delay(100);
        JumpToUsbBootloader();
    }

    //*ENTER_APPLICATION
    else if (strcmp(current_command->command,"*ENTER_APPLICATION") == 0){
        printf("OK;GOING TO APPLICATION\n");
        //WAIT UNTIL PRINTF FINISHES
        HAL_Delay(100);
        JumpToApplication();
    }

    //*RST
    else if (strcmp(current_command->command,"*RST") == 0){
        NVIC_SystemReset();
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
                 "OK;RAFIONTECH s.r.o. - F4 BOOTLOADER V1.0,\r"
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
