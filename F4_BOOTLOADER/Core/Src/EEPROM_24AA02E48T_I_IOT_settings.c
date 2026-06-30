#include "EEPROM_24AA02E48T_I_IOT_settings.h"
static HAL_StatusTypeDef i2c_status;

int8_t EEPROMsetFlag(uint16_t flagAddress, uint8_t flagValue){
    //retunrs 1 if success, 
    //-1 if error in EEPROM WRITING
    uint8_t flag[1] = {flagValue};
    i2c_status =  HAL_I2C_Mem_Write(&hi2c2, EEPROM_I2C_ADDR_FOR_WRITING_CUSTOM_VALUES, flagAddress, I2C_MEMADD_SIZE_8BIT, flag, 1,HAL_MAX_DELAY);
    if(i2c_status != HAL_OK){
        return -1;
    }
    return 1;
}

int8_t EEPROMEraseAll(){
    //retunrs 1 if success, 
    //-1 if error in EEPROM WRITING
    uint8_t flag[1] = {0xFF};
    for (uint8_t i = 0; i < 0xff; i++){
        HAL_I2C_IsDeviceReady(&hi2c2, EEPROM_I2C_ADDR_FOR_WRITING_CUSTOM_VALUES, 100, 1000);
        i2c_status =  HAL_I2C_Mem_Write(&hi2c2, EEPROM_I2C_ADDR_FOR_WRITING_CUSTOM_VALUES, i, I2C_MEMADD_SIZE_8BIT, flag, 1,HAL_MAX_DELAY);
        if(i2c_status != HAL_OK){
            return -1;
        }
    }
    return 1;
}

int8_t EEPROMCheckFlag(uint16_t flagAddress){
    //retunrs 1 if flag is set
    //0 if flag is not set
    // -1 if error in EEPROM READING and 
    static uint8_t custom_flag[1];
    i2c_status =  HAL_I2C_Mem_Read(&hi2c2, EEPROM_I2C_ADDR_FOR_READING_CUSTOM_VALUES, flagAddress, I2C_MEMADD_SIZE_8BIT, custom_flag, 1, HAL_MAX_DELAY);
    if(i2c_status != HAL_OK){ //ERROR IN READING EEPROM
            return -1;
    }
    if (custom_flag[0] != CUSTOM_FLAG_IS_SET_VALUE){ // in case, that custom IP is not set IN EEPROM
        return 0;
    }
    return 1;
}
//####################################TEMPERATURE OFFSET ####################################
int8_t EEPROMgetCustomTemperatureOffset(uint8_t temperature_offset[1]){
    static int8_t check_flag_response;
    check_flag_response = EEPROMCheckFlag(EEPROM_TEMPERATURE_OFFSET_FLAG);
    if ( check_flag_response == 0){
        return 0;
    } // in case, that custom IP is not set IN EEPROM -> return 0

    else if (check_flag_response == -1){
        return -2; // in case of error in reading flag, which represents, that custom IP is set -> return -2
    } 

    //Try to read IP address from EEPROM and save it to ipAddress buffer
    i2c_status =  HAL_I2C_Mem_Read(&hi2c2, EEPROM_I2C_ADDR_FOR_READING_CUSTOM_VALUES, EEPROM_TEMPERATURE_OFFSET, I2C_MEMADD_SIZE_8BIT, temperature_offset, 1, HAL_MAX_DELAY);
    if(i2c_status != HAL_OK){
        return -1;
    }
    return 1;
}

//########################################### MAC ###########################################
int8_t EEPROMgetDefaultMacAddress(uint8_t macAddress[6]){
    //This function returns 1 if reading was succesfull -> EEPROM contain default MAC address from manufacturer -> MAC address is set in macAddress buffer
    // return -1 in case of error in EEPROM READING
    static uint8_t EUI48_ADDR[6];
    i2c_status =  HAL_I2C_Mem_Read(&hi2c2, EEPROM_I2C_ADDR_FOR_MAC, EEPROM_I2C_STARTADDR_OF_MAC, I2C_MEMADD_SIZE_8BIT, EUI48_ADDR, EEPROM_I2C_MAC_ADDR_LEN, HAL_MAX_DELAY);
    macAddress[0] = EUI48_ADDR[0];
    macAddress[1] = EUI48_ADDR[1];
    macAddress[2] = EUI48_ADDR[2];
    macAddress[3] = EUI48_ADDR[3];
    macAddress[4] = EUI48_ADDR[4];
    macAddress[5] = EUI48_ADDR[5];
    if(i2c_status != HAL_OK){ 
        return -1;//ERROR IN READING EEPROM
    }
    return 1;
}

//########################################### IP ###########################################
int8_t EEPROMgetCustomIPAddress(uint8_t ipAddress[4]){
    // retunrs 1 if reading was succesfull -> IP address is set in ipAddress buffer
    // return 0 if IP address is not specified in EEPROM
    // returns -1 if error in EEPROM READING
    // returns -2 in case of error in reading flag, which represents, that custom IP is set
    static int8_t check_flag_response;
    check_flag_response = EEPROMCheckFlag(EEPROM_FLAG_ADDR_IP);
    if ( check_flag_response == 0){
        return 0;
    } // in case, that custom IP is not set IN EEPROM -> return 0

    else if (check_flag_response == -1){
        return -2; // in case of error in reading flag, which represents, that custom IP is set -> return -2
    } 

    //Try to read IP address from EEPROM and save it to ipAddress buffer
    i2c_status =  HAL_I2C_Mem_Read(&hi2c2, EEPROM_I2C_ADDR_FOR_READING_CUSTOM_VALUES, EEPROM_CUSTOM_ADDR_OF_IP, I2C_MEMADD_SIZE_8BIT, ipAddress, 4, HAL_MAX_DELAY);
    if(i2c_status != HAL_OK){
        return -1;
    }
    return 1;
}

int8_t EEPROMWriteCustomIP(uint8_t ipAddress[4]){
    // retunrs 1 if writing was succesfull
    // returns -1 if error in EEPROM WRITING IP
    // returns -2 in case of error writing reading flag, which represents, that custom IP is set

    //WRITE IP ADDRESS
    i2c_status =  HAL_I2C_Mem_Write(&hi2c2, EEPROM_I2C_ADDR_FOR_WRITING_CUSTOM_VALUES, EEPROM_CUSTOM_ADDR_OF_IP, I2C_MEMADD_SIZE_8BIT, ipAddress, 4, 1000);
    if(i2c_status != HAL_OK){
        return -1;
    }
    
    HAL_I2C_IsDeviceReady(&hi2c2, EEPROM_I2C_ADDR_FOR_WRITING_CUSTOM_VALUES, 100, 1000);
    
    //WRITE IP ADDRESS FLAG
    static int8_t set_flag_response;
    set_flag_response = EEPROMsetFlag(EEPROM_FLAG_ADDR_IP, CUSTOM_FLAG_IS_SET_VALUE);
    if (set_flag_response == -1){
        return -2; // in case of error writing reading flag, which represents, that custom IP is set -> return -2
    }

    return 1;
}
//########################################### SUBNET ###########################################
int8_t EEPROMgetCustomSubnet(uint8_t subnet[4]){
    // retunrs 1 if reading was succesfull -> SUBNET address is set in subnet buffer
    // return 0 if SUBNET address is not specified in EEPROM
    // returns -1 if error in EEPROM READING
    // returns -2 in case of error in reading flag, which represents, that custom SUBNET is set
    static int8_t check_flag_response;
    check_flag_response = EEPROMCheckFlag(EEPROM_FLAG_ADDR_SUBNET);
    if ( check_flag_response == 0){
        return 0;
    } // in case, that custom SUBNET is not set IN EEPROM -> return 0

    else if (check_flag_response == -1){
        return -2; // in case of error in reading flag, which represents, that custom SUBNET is set -> return -2
    } 

    //Try to read SUBNET address from EEPROM and save it to  subnet buffer
    i2c_status =  HAL_I2C_Mem_Read(&hi2c2, EEPROM_I2C_ADDR_FOR_READING_CUSTOM_VALUES, EEPROM_CUSTOM_ADDR_OF_SUBNET, I2C_MEMADD_SIZE_8BIT, subnet, 4, HAL_MAX_DELAY);
    if(i2c_status != HAL_OK){
        return -1;
    }
    return 1;
}

int8_t EEPROMWriteCustomSubnet(uint8_t subnet[4]){
    // retunrs 1 if writing was succesfull
    // returns -1 if error in EEPROM WRITING SUBNET
    // returns -2 in case of error writing reading flag, which represents, that custom SUBNET is set

    //WRITE SUBNET ADDRESS
    i2c_status =  HAL_I2C_Mem_Write(&hi2c2, EEPROM_I2C_ADDR_FOR_WRITING_CUSTOM_VALUES, EEPROM_CUSTOM_ADDR_OF_SUBNET, I2C_MEMADD_SIZE_8BIT, subnet, 4, 1000);
    if(i2c_status != HAL_OK){
        return -1;
    }
    
    HAL_I2C_IsDeviceReady(&hi2c2, EEPROM_I2C_ADDR_FOR_WRITING_CUSTOM_VALUES, 100, 1000);
    
    //WRITE SUBNET ADDRESS FLAG
    static int8_t set_flag_response;
    set_flag_response = EEPROMsetFlag(EEPROM_FLAG_ADDR_SUBNET, CUSTOM_FLAG_IS_SET_VALUE);
    if (set_flag_response == -1){
        return -2; // in case of error writing reading flag, which represents, that custom SUBNET is set -> return -2
    }

    return 1;
}

//########################################### GATEWAY ###########################################
int8_t EEPROMgetCustomGateway(uint8_t gateway[4]){
    // retunrs 1 if reading was succesfull -> GATEWAY address is set in gateway buffer
    // return 0 if GATEWAY address is not specified in EEPROM
    // returns -1 if error in EEPROM READING
    // returns -2 in case of error in reading flag, which represents, that custom GATEWAY is set
    static int8_t check_flag_response;
    check_flag_response = EEPROMCheckFlag(EEPROM_FLAG_ADDR_GATEWAY);
    if ( check_flag_response == 0){
        return 0;
    } // in case, that custom GATEWAY is not set IN EEPROM -> return 0

    else if (check_flag_response == -1){
        return -2; // in case of error in reading flag, which represents, that custom GATEWAY is set -> return -2
    } 

    //Try to read GATEWAY address from EEPROM and save it to  gateway buffer
    i2c_status =  HAL_I2C_Mem_Read(&hi2c2, EEPROM_I2C_ADDR_FOR_READING_CUSTOM_VALUES, EEPROM_CUSTOM_ADDR_OF_GATEWAY, I2C_MEMADD_SIZE_8BIT, gateway, 4, HAL_MAX_DELAY);
    if(i2c_status != HAL_OK){
        return -1;
    }
    return 1;
}

int8_t EEPROMWriteCustomGateway(uint8_t gateway[4]){
    // retunrs 1 if writing was succesfull
    // returns -1 if error in EEPROM WRITING GATEWAY
    // returns -2 in case of error writing reading flag, which represents, that custom GATEWAY is set

    //WRITE GATEWAY ADDRESS
    i2c_status =  HAL_I2C_Mem_Write(&hi2c2, EEPROM_I2C_ADDR_FOR_WRITING_CUSTOM_VALUES, EEPROM_CUSTOM_ADDR_OF_GATEWAY, I2C_MEMADD_SIZE_8BIT, gateway, 4, 1000);
    if(i2c_status != HAL_OK){
        return -1;
    }
    
    HAL_I2C_IsDeviceReady(&hi2c2, EEPROM_I2C_ADDR_FOR_WRITING_CUSTOM_VALUES, 100, 1000);
    
    //WRITE GATEWAY ADDRESS FLAG
    static int8_t set_flag_response;
    set_flag_response = EEPROMsetFlag(EEPROM_FLAG_ADDR_GATEWAY, CUSTOM_FLAG_IS_SET_VALUE);
    if (set_flag_response == -1){
        return -2; // in case of error writing reading flag, which represents, that custom GATEWAY is set -> return -2
    }

    return 1;
}

//########################################### CUSTOM MAC ###########################################
int8_t EEPROMgetCustomMac(uint8_t mac[6]){
    // retunrs 1 if reading was succesfull -> MAC address is set in mac buffer
    // return 0 if MAC address is not specified in EEPROM
    // returns -1 if error in EEPROM READING
    // returns -2 in case of error in reading flag, which represents, that custom MAC is set
    static int8_t check_flag_response;
    check_flag_response = EEPROMCheckFlag(EEPROM_FLAG_ADDR_MAC);
    if ( check_flag_response == 0){
        return 0;
    } // in case, that custom MAC is not set IN EEPROM -> return 0

    else if (check_flag_response == -1){
        return -2; // in case of error in reading flag, which represents, that custom MAC is set -> return -2
    } 

    //Try to read MAC address from EEPROM and save it to  mac buffer
    i2c_status =  HAL_I2C_Mem_Read(&hi2c2, EEPROM_I2C_ADDR_FOR_READING_CUSTOM_VALUES, EEPROM_CUSTOM_ADDR_OF_MAC, I2C_MEMADD_SIZE_8BIT, mac, 6, HAL_MAX_DELAY);
    if(i2c_status != HAL_OK){
        return -1;
    }
    return 1;
}

int8_t EEPROMWriteCustomMac(uint8_t mac[6]){
    // retunrs 1 if writing was succesfull
    // returns -1 if error in EEPROM WRITING MAC
    // returns -2 in case of error writing reading flag, which represents, that custom MAC is set

    //WRITE MAC ADDRESS
    i2c_status =  HAL_I2C_Mem_Write(&hi2c2, EEPROM_I2C_ADDR_FOR_WRITING_CUSTOM_VALUES, EEPROM_CUSTOM_ADDR_OF_MAC, I2C_MEMADD_SIZE_8BIT, mac, 6, 1000);
    if(i2c_status != HAL_OK){
        return -1;
    }
    
    HAL_I2C_IsDeviceReady(&hi2c2, EEPROM_I2C_ADDR_FOR_WRITING_CUSTOM_VALUES, 100, 1000);
    
    //WRITE MAC ADDRESS FLAG
    static int8_t set_flag_response;
    set_flag_response = EEPROMsetFlag(EEPROM_FLAG_ADDR_MAC, CUSTOM_FLAG_IS_SET_VALUE);
    if (set_flag_response == -1){
        return -2; // in case of error writing reading flag, which represents, that custom MAC is set -> return -2
    }

    return 1;
}

int8_t EEPROMWriteCustomTemperatureOffset(int8_t temperature_offset[1]){
    // retunrs 1 if writing was succesfull
    // returns -1 if error in EEPROM TEMPERATURE OFFSET
    // returns -2 in case of error writing reading flag, which represents, that custom TEMP_OFFSET is set

    //WRITE MAC ADDRESS
    i2c_status =  HAL_I2C_Mem_Write(&hi2c2, EEPROM_I2C_ADDR_FOR_WRITING_CUSTOM_VALUES, EEPROM_TEMPERATURE_OFFSET, I2C_MEMADD_SIZE_8BIT, (uint8_t*)(temperature_offset), 1, 1000);
    if(i2c_status != HAL_OK){
        return -1;
    }
    
    HAL_I2C_IsDeviceReady(&hi2c2, EEPROM_I2C_ADDR_FOR_WRITING_CUSTOM_VALUES, 100, 1000);
    
    //WRITE MAC ADDRESS FLAG
    static int8_t set_flag_response;
    set_flag_response = EEPROMsetFlag(EEPROM_TEMPERATURE_OFFSET_FLAG, CUSTOM_FLAG_IS_SET_VALUE);
    if (set_flag_response == -1){
        return -2; // in case of error writing reading flag, which represents, that custom MAC is set -> return -2
    }

    return 1;
}



static void readCommands(commandTemplate* current_command){

    if (current_command->number_of_subcommands == 1){
        sprintf(current_command->response, "ERROR;Subcommand is missing, received: %s %s\n", current_command->command, current_command->subcommands[0]);	
    return;
    }

    //EEPROM READ IP
    if(strcmp(current_command->subcommands[1],"IP") == 0){
        uint8_t ip_addr[4] = {0};
        int8_t response = EEPROMgetCustomIPAddress(ip_addr);
        if (response == 1){
            sprintf(current_command->response, "OK;IP %d.%d.%d.%d\n", ip_addr[0], ip_addr[1], ip_addr[2], ip_addr[3]);
            return;
        }
        else if (response == 0){
            sprintf(current_command->response, "ERROR;CUSTOM IP IS NOT DEFINED IN EEPROM\n");
            return;
        }
        else if (response == -1){
            sprintf(current_command->response, "ERROR;ERROR IN READING CUSTOM IP\n");
            return;
        }
        else if (response == -2){
            sprintf(current_command->response, "ERROR;ERROR IN READING CUSTOM IP FLAG\n");
            return;
        }
    }

    //EEPROM READ CUSTOM_MAC
    else if(strcmp(current_command->subcommands[1],"CUSTOM_MAC") == 0){
        uint8_t mac_addr[6] = {0};
        int8_t response = EEPROMgetCustomMac(mac_addr);
        if (response == 1){
            sprintf(current_command->response, "OK;CUSTOM MAC %02X:%02X:%02X:%02X:%02X:%02X\n", mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
            return;
        }
        else if (response == 0){
            sprintf(current_command->response, "ERROR;CUSTOM MAC IS NOT DEFINED IN EEPROM\n");
            return;
        }
        else if (response == -1){
            sprintf(current_command->response, "ERROR;ERROR IN READING CUSTOM MAC\n");
            return;
        }
        else if (response == -2){
            sprintf(current_command->response, "ERROR;ERROR IN READING CUSTOM MAC FLAG\n");
            return;
        }
    }

    //EEPROM READ GATEWAY
    else if(strcmp(current_command->subcommands[1],"GATEWAY") == 0){
        uint8_t gateway_addr[4] = {0};
        int8_t response = EEPROMgetCustomGateway(gateway_addr);
        if (response == 1){
            sprintf(current_command->response, "OK;GATEWAY %d.%d.%d.%d\n", gateway_addr[0], gateway_addr[1], gateway_addr[2], gateway_addr[3]);
            return;
        }
        else if (response == 0){
            sprintf(current_command->response, "ERROR;CUSTOM GATEWAY IS NOT DEFINED IN EEPROM\n");
            return;
        }
        else if (response == -1){
            sprintf(current_command->response, "ERROR;ERROR IN READING CUSTOM GATEWAY\n");
            return;
        }
        else if (response == -2){
            sprintf(current_command->response, "ERROR;ERROR IN READING CUSTOM GATEWAY FLAG\n");
            return;
        }
    }

    //EEPROM READ SUBNET
    else if(strcmp(current_command->subcommands[1],"SUBNET") == 0){
        uint8_t subnet_addr[4] = {0};
        int8_t response = EEPROMgetCustomSubnet(subnet_addr);
        if (response == 1){
            sprintf(current_command->response, "OK;SUBNET %d.%d.%d.%d\n", subnet_addr[0], subnet_addr[1], subnet_addr[2], subnet_addr[3]);
            return;
        }
        else if (response == 0){
            sprintf(current_command->response, "ERROR;CUSTOM SUBNET IS NOT DEFINED IN EEPROM\n");
            return;
        }
        else if (response == -1){
            sprintf(current_command->response, "ERROR;ERROR IN READING CUSTOM SUBNET\n");
            return;
        }
        else if (response == -2){
            sprintf(current_command->response, "ERROR;ERROR IN READING CUSTOM SUBNET FLAG\n");
            return;
        }
    }

    //EEPROM READ DEFAULT_MAC
    else if(strcmp(current_command->subcommands[1],"DEFAULT_MAC") == 0){
        uint8_t mac_addr[6] = {0};
        int8_t response = EEPROMgetDefaultMacAddress(mac_addr);
        if (response == 1){
            sprintf(current_command->response, "OK;DEFAULT MAC %02X::%02X::%02X::%02X::%02X::%02X\n", mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
            return;
        }

        if (response == -1){
            sprintf(current_command->response, "ERROR;ERROR IN READING DEFAULT MAC\n");
            return;
        }
    }

    //EEPROM READ TEMPERATURE_OFFSET
    else if(strcmp(current_command->subcommands[1],"TEMPERATURE_OFFSET") == 0){
        uint8_t temp_offset[1] = {0};
        int8_t response = EEPROMgetCustomTemperatureOffset(temp_offset);
        if (response == 1){
            sprintf(current_command->response, "OK;TEMPERATURE OFFSET:%d\n", (int8_t)(temp_offset[0])/4);
            return;
        }
        else if (response == 0){
            sprintf(current_command->response, "ERROR;CUSTOM TEMPERATURE OFFSET IS NOT DEFINED IN EEPROM\n");
            return;
        }
        else if (response == -1){
            sprintf(current_command->response, "ERROR;ERROR IN READING CUSTOM TEMPERATURE OFFSET\n");
            return;
        }
        else if (response == -2){
            sprintf(current_command->response, "ERROR;ERROR IN READING CUSTOM TEMPERATURE OFFSET FLAG\n");
            return;
        }
    }

    else{
        sprintf(current_command->response, "ERROR;Unknown subcommand: %s %s\n", current_command->command, current_command->subcommands[1]);
        return;
    }
}

static void writeCommands(commandTemplate* current_command){

    if (current_command->number_of_subcommands == 1){
        sprintf(current_command->response, "ERROR;Subcommand is missing, received: %s %s\n", current_command->command, current_command->subcommands[0]);	
    return;
    }

    //EEPROM WRITE IP [A] [B] [C] [D]
    if(strcmp(current_command->subcommands[1],"IP") == 0){
        uint8_t ip[4] = {0,0,0,0};
        ip[0] = atoi(current_command->subcommands[2]);
        ip[1] = atoi(current_command->subcommands[3]);
        ip[2] = atoi(current_command->subcommands[4]);
        ip[3] = atoi(current_command->subcommands[5]);
        int8_t response = EEPROMWriteCustomIP(ip);
        if (response == 1){
            sprintf(current_command->response,"OK;Succesfully write IP to EEPROM:%d.%d.%d.%d\n",ip[0],ip[1],ip[2],ip[3]);
            return;
        }
        else if (response == -1){
            sprintf(current_command->response,"ERROR;WRITING CUSTOM IP TO EEPROM FAILED\n");
            return;
        }
        else if (response == -2){
            sprintf(current_command->response,"ERROR;WRITING CUSTOM IP FLAG TO EEPROM FAILED\n");
            return;
        }
    }

    //EEPROM WRITE SUBNET [A] [B] [C] [D]
    else if (strcmp(current_command->subcommands[1],"SUBNET") == 0){
        uint8_t subnet[4] = {0,0,0,0};
        subnet[0] = atoi(current_command->subcommands[2]);
        subnet[1] = atoi(current_command->subcommands[3]);
        subnet[2] = atoi(current_command->subcommands[4]);
        subnet[3] = atoi(current_command->subcommands[5]);
        int8_t response = EEPROMWriteCustomSubnet(subnet);
        if (response == 1){
            sprintf(current_command->response,"OK;Succesfully write SUBNET to EEPROM:%d.%d.%d.%d\n",subnet[0],subnet[1],subnet[2],subnet[3]);
            return;
        }
        else if (response == -1){
            sprintf(current_command->response,"ERROR;WRITING CUSTOM SUBNET TO EEPROM FAILED\n");
            return;
        }
        else if (response == -2){
            sprintf(current_command->response,"ERROR;WRITING CUSTOM SUBNET FLAG TO EEPROM FAILED\n");
            return;
        }
    }

    //EEPROM WRITE GATEWAY [A] [B] [C] [D]
    else if (strcmp(current_command->subcommands[1],"GATEWAY") == 0){
        uint8_t gateway[4] = {0,0,0,0};
        gateway[0] = atoi(current_command->subcommands[2]);
        gateway[1] = atoi(current_command->subcommands[3]);
        gateway[2] = atoi(current_command->subcommands[4]);
        gateway[3] = atoi(current_command->subcommands[5]);
        int8_t response = EEPROMWriteCustomGateway(gateway);
        if (response == 1){
            sprintf(current_command->response,"OK;Succesfully write GATEWAY to EEPROM:%d.%d.%d.%d\n",gateway[0],gateway[1],gateway[2],gateway[3]);
            return;
        }
        else if (response == -1){
            sprintf(current_command->response,"ERROR;WRITING CUSTOM GATEWAY TO EEPROM FAILED\n");
            return;
        }
        else if (response == -2){
            sprintf(current_command->response,"ERROR;WRITING CUSTOM GATEWAY FLAG TO EEPROM FAILED\n");
            return;
        }
    }

    //EEPROM WRITE CUSTOM_MAC [XX] [XX] [XX] [XX] [XX] [XX]
    else if (strcmp(current_command->subcommands[1],"CUSTOM_MAC") == 0){
        uint8_t mac[6] = {0,0,0,0,0,0};
        mac[0] = strtol(current_command->subcommands[2],NULL,16);
        mac[1] = strtol(current_command->subcommands[3],NULL,16);
        mac[2] = strtol(current_command->subcommands[4],NULL,16);
        mac[3] = strtol(current_command->subcommands[5],NULL,16);
        mac[4] = strtol(current_command->subcommands[6],NULL,16);
        mac[5] = strtol(current_command->subcommands[7],NULL,16);
        int8_t response = EEPROMWriteCustomMac(mac);
        if (response == 1){
            sprintf(current_command->response,"OK;Succesfully write CUSTOM MAC to EEPROM:%02X::%02X::%02X::%02X::%02X::%02X\n",mac[0],mac[1],mac[2],mac[3],mac[4],mac[5]);
            return;
        }
        else if (response == -1){
            sprintf(current_command->response,"ERROR;WRITING CUSTOM MAC TO EEPROM FAILED\n");
            return;
        }
        else if (response == -2){
            sprintf(current_command->response,"ERROR;WRITING CUSTOM MAC FLAG TO EEPROM FAILED\n");
        }
    }

    //EEPROM WRITE TEMPERATURE_OFFSET [VALUE_IN_C]
        else if (strcmp(current_command->subcommands[1],"TEMPERATURE_OFFSET") == 0){
        int8_t tempereature_offset[] = {0};
        int32_t temp_offset = 0;
        temp_offset = strtol(current_command->subcommands[2],NULL,10)*4;
        if (temp_offset > 127 || temp_offset < -128){
            sprintf(current_command->response,"ERROR;TEMPERATURE OFFSET IS OUT OF RANGE should be from %d to %d\n", -32, 31);
            return;
        }

        tempereature_offset[0] = (int8_t)(temp_offset);
        int8_t response = EEPROMWriteCustomTemperatureOffset(tempereature_offset);
        if (response == 1){
            sprintf(current_command->response,"OK;Succesfully write TEMPERATURE OFFSET TO EEPROM:%d\n",tempereature_offset[0]/4);
            return;
        }
        else if (response == -1){
            sprintf(current_command->response,"ERROR;WRITING TEMPERATURE OFFSET TO EEPROM FAILED\n");
            return;
        }
        else if (response == -2){
            sprintf(current_command->response,"ERROR;WRITING CUSTOM MAC FLAG TO FROM EEPROM FAILED\n");
        }
    }
    

    else{
        sprintf(current_command->response, "ERROR;Unknown subcommand: %s %s\n", current_command->command, current_command->subcommands[1]);
        return;
    }
}


void EEPROMInputsParser(commandTemplate* current_command){
    //"""Implementation of commands"""
    if (current_command->number_of_subcommands == 0){
        sprintf(current_command->response, "ERROR; Subcommand is missing, received: %s\n", current_command->command);	
        return;
    }
 
    //EEPROM WRITE <TARGET> [...]
    if (strcmp(current_command->subcommands[0],"WRITE") == 0){
        writeCommands(current_command);
    }

    //EEPROM READ <TARGET>
    else if (strcmp(current_command->subcommands[0],"READ") == 0){
        readCommands(current_command);
    }

    //EEPROM ERASE
    else if (strcmp(current_command->subcommands[0],"ERASE") == 0){
        if (current_command->number_of_subcommands > 1){
            sprintf(current_command->response, "ERROR; Subcommand ERASE does not have any arguments, received: %s %s\n", current_command->command, current_command->subcommands[1]);	
            return;
        }
        uint8_t result = EEPROMEraseAll();
        if (result == 1){
            sprintf(current_command->response, "OK; EEPROM ERASED\n");
        }
        else if (result == -1){
            sprintf(current_command->response, "ERROR; EEPROM ERASE FAILED\n");
        }
    }

    //EEPROM ERASE_CUSTOM [IP|SUBNET|GATEWAY|CUSTOM_MAC|TEMPERATURE_OFFSET]
    else if (strcmp(current_command->subcommands[0],"ERASE_CUSTOM") == 0){
        //EEPROM ERASE_CUSTOM IP
        if (strcmp(current_command->subcommands[1],"IP") == 0) { EEPROMsetFlag(EEPROM_FLAG_ADDR_IP,0xFF); }
        //EEPROM ERASE_CUSTOM SUBNET
        else if (strcmp(current_command->subcommands[1],"SUBNET") == 0) { EEPROMsetFlag(EEPROM_FLAG_ADDR_SUBNET,0xFF); }
        //EEPROM ERASE_CUSTOM GATEWAY
        else if (strcmp(current_command->subcommands[1],"GATEWAY") == 0) { EEPROMsetFlag(EEPROM_FLAG_ADDR_GATEWAY,0xFF); }
        //EEPROM ERASE_CUSTOM CUSTOM_MAC
        else if (strcmp(current_command->subcommands[1],"CUSTOM_MAC") == 0) { EEPROMsetFlag(EEPROM_FLAG_ADDR_MAC,0xFF); }
        //EEPROM ERASE_CUSTOM TEMPERATURE_OFFSET
        else if (strcmp(current_command->subcommands[1],"TEMPERATURE_OFFSET") == 0) { EEPROMsetFlag(EEPROM_TEMPERATURE_OFFSET_FLAG,0xFF); }
        else {
            sprintf(current_command->response, "ERROR; Unknown subcommand: %s %s\n", current_command->command, current_command->subcommands[1]);
            return;
        }

        sprintf(current_command->response, "OK;CUSTOM %s ERASED\n",current_command->subcommands[1]);        
    }

    else{
        sprintf(current_command->response, "ERROR; Unknown command: %s\n", current_command->subcommands[0]);
    }
    

}




