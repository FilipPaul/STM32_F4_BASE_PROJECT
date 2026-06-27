#include "stdio.h"
#include "stdlib.h"
#include "main.h"
#include "string.h"
#include "globals.h"
#include "IPsettings.h"
#include "lwip/opt.h"
#include "lwip/api.h"
#include "lwip/sys.h"
#include "lwip.h"
#include "ethernetif.h"
extern struct netif gnetif;
extern ip4_addr_t ipaddr;
extern ip4_addr_t netmask;
extern ip4_addr_t gw;
extern uint8_t IP_ADDRESS[4];
extern uint8_t NETMASK_ADDRESS[4];
extern uint8_t GATEWAY_ADDRESS[4];
extern ETH_HandleTypeDef heth;


static uint8_t checkNumberOfSubcommands(uint8_t expected_number, uint8_t current_subcommand_index ,commandTemplate* current_command){
    //Function that check if the command has the correct number of subcommands
    if (current_command->number_of_subcommands-current_subcommand_index-1 != expected_number){
        sprintf(current_command->response,"ERROR;Wrong number of arguments after command: %s \
(Expected %d, got %d)\n",current_command->subcommands[current_subcommand_index] ,expected_number,current_command->number_of_subcommands-current_subcommand_index-1);
        return 0;
        }
    return 1;
}

void IPsettingParser(commandTemplate* current_command){
    //"""Implementation of commands"""
    if (current_command->number_of_subcommands == 0){
        sprintf(current_command->response, "ERROR; Subcommand is missing, received: %s\n", current_command->command);	
        return;
    }
 
    if (strcmp(current_command->subcommands[0],"IP") == 0){
        //check number of subcommands
        if (!checkNumberOfSubcommands(4,0,current_command)) return;
        // UPDATE UP ADDRESS
        LOCK_TCPIP_CORE();
        IP_ADDRESS[0] = atoi(current_command->subcommands[1]);
        IP_ADDRESS[1] = atoi(current_command->subcommands[2]);
        IP_ADDRESS[2] = atoi(current_command->subcommands[3]);
        IP_ADDRESS[3] = atoi(current_command->subcommands[4]);
        IP4_ADDR(&ipaddr, IP_ADDRESS[0], IP_ADDRESS[1], IP_ADDRESS[2], IP_ADDRESS[3]);
        //netif_set_link_down(&gnetif);
        netif_set_ipaddr(&gnetif, &ipaddr);
        //netif_set_link_up(&gnetif);
        UNLOCK_TCPIP_CORE();

        sprintf(current_command->response,"OK;IP changed to:%d.%d.%d.%d\n",\
         IP_ADDRESS[0],IP_ADDRESS[1],\
         IP_ADDRESS[2],IP_ADDRESS[3]);
    }

    else if (strcmp(current_command->subcommands[0],"SUBNET") == 0){
        //check number of subcommands
        if (!checkNumberOfSubcommands(4,0,current_command)) return;
        // UPDATE UP ADDRESS
        LOCK_TCPIP_CORE();
        NETMASK_ADDRESS[0] = atoi(current_command->subcommands[1]);
        NETMASK_ADDRESS[1] = atoi(current_command->subcommands[2]);
        NETMASK_ADDRESS[2] = atoi(current_command->subcommands[3]);
        NETMASK_ADDRESS[3] = atoi(current_command->subcommands[4]);
        IP4_ADDR(&netmask, NETMASK_ADDRESS[0], NETMASK_ADDRESS[1], NETMASK_ADDRESS[2], NETMASK_ADDRESS[3]);
        //netif_set_link_down(&gnetif);
        netif_set_netmask(&gnetif, &netmask);
        //netif_set_link_up(&gnetif);
        UNLOCK_TCPIP_CORE();

        sprintf(current_command->response,"OK;SUBNET changed to:%d.%d.%d.%d\n",\
        NETMASK_ADDRESS[0],NETMASK_ADDRESS[1],\
        NETMASK_ADDRESS[2],NETMASK_ADDRESS[3]);
    }

    else if (strcmp(current_command->subcommands[0],"GATEWAY") == 0){
        if (!checkNumberOfSubcommands(4,0,current_command)) return;
        // UPDATE UP ADDRESS
        LOCK_TCPIP_CORE();
        GATEWAY_ADDRESS[0] = atoi(current_command->subcommands[1]);
        GATEWAY_ADDRESS[1] = atoi(current_command->subcommands[2]);
        GATEWAY_ADDRESS[2] = atoi(current_command->subcommands[3]);
        GATEWAY_ADDRESS[3] = atoi(current_command->subcommands[4]);
        IP4_ADDR(&gw, GATEWAY_ADDRESS[0], GATEWAY_ADDRESS[1], GATEWAY_ADDRESS[2], GATEWAY_ADDRESS[3]);
        //netif_set_link_down(&gnetif);
        netif_set_gw(&gnetif, &gw);
        //netif_set_link_up(&gnetif);
        UNLOCK_TCPIP_CORE();

        sprintf(current_command->response,"OK;GATEWAY changed to:%d.%d.%d.%d\n",\
        GATEWAY_ADDRESS[0],GATEWAY_ADDRESS[1],\
        GATEWAY_ADDRESS[2],GATEWAY_ADDRESS[3]);
    }

    else if (strcmp(current_command->subcommands[0],"MAC") == 0){
        if (!checkNumberOfSubcommands(6,0,current_command)) return;
        // UPDATE UP ADDRESS
        LOCK_TCPIP_CORE();
        uint8_t fLink = netif_is_link_up(&gnetif);
        netif_set_link_down(&gnetif);
        uint8_t MACAddr[6] ;
        MACAddr[0] = strtol(current_command->subcommands[1],NULL,16);
        MACAddr[1] = strtol(current_command->subcommands[2],NULL,16);
        MACAddr[2] = strtol(current_command->subcommands[3],NULL,16);
        MACAddr[3] = strtol(current_command->subcommands[4],NULL,16);
        MACAddr[4] = strtol(current_command->subcommands[5],NULL,16);
        MACAddr[5] = strtol(current_command->subcommands[6],NULL,16);
        heth.Init.MACAddr = &MACAddr[0];
        gnetif.hwaddr[0] =  heth.Init.MACAddr[0];
        gnetif.hwaddr[1] =  heth.Init.MACAddr[1];
        gnetif.hwaddr[2] =  heth.Init.MACAddr[2];
        gnetif.hwaddr[3] =  heth.Init.MACAddr[3];
        gnetif.hwaddr[4] =  heth.Init.MACAddr[4];
        gnetif.hwaddr[5] =  heth.Init.MACAddr[5];

        if (fLink) {
	        netif_set_link_up(&gnetif);
        }
        
        UNLOCK_TCPIP_CORE();

        sprintf(current_command->response,"OK;GATEWAY changed to:%2X::%2X::%2X::%2X::%2X::%2X\n",\
         MACAddr[0],MACAddr[1],\
         MACAddr[2],MACAddr[3],\
         MACAddr[4],MACAddr[5]);
    }

    else{
        sprintf(current_command->response, "ERROR; Unknown command: %s\n", current_command->subcommands[0]);
    }
    

}