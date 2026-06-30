////////// PURPOSE OF THIS FILE IS TO CREATE SIMPLE TCP SERVER THAT WILL RECEIVE FIRMWARE FILE FROM PC AND STORE IT IN FLASH MEMORY////////
//IT SHOULD RECEIVE FOLLOWING:
//1. FIRMWARE FILE SIZE (4 BYTES)
//2. FIRMWARE CRC (4 BYTES)
//3. FIRMWARE FILE (FIRMWARE FILE SIZE BYTES)
//4. APP START ADDRESS (4 BYTES)

//EXAMPLE OF RECV DATA:
//FIRST RECEIVE METADATA JSON OBJECT
//json_str =  {"size": 123456, "crc": 12345678, "start_address": 0x08020000}

//THEN RECEIVE FIRMWARE FILE DATA
//json_str =  {"data": "base64_encoded_data"}



#include "RAFION_TCP_OTA.h"
//include for json parsing
#include "cJSON.h"
#include "stdlib.h"

//include for stm32 crc
#include "stm32f4xx_hal.h"

#define FIRMWARE_METADATA_JSON 1
#define FIRMWARE_DATA_JSON 2

typedef struct currentApplicationMetadata
{
    uint32_t size;
    uint32_t crc;
    uint32_t start_address;
    void *ptr_to_firmware_in_flash;
    void *ptr_to_firmware_in_ram;
    bool is_valid;
} currentApplicationMetadata_t;


int8_t RafionHandleIncommingJson(char *json_str, currentApplicationMetadata_t *metadata)
{
    //GET JSON TYPE
    int8_t error_code;
    int8_t json_type = RafionOTAgetReceivedJsonType(json_str, metadata);
    if (json_type == FIRMWARE_METADATA_JSON)
    {
        if(RafionOTAalocateMemoryForFirmware(metadata) != true)
        {
            return -1; // Memory allocation failed
        }

        return FIRMWARE_METADATA_JSON; // successfully parsed metadata JSON
    }
    else if (json_type == FIRMWARE_DATA_JSON)
    {
        // Assuming the firmware data is base64 encoded in the JSON
        cJSON *json = cJSON_Parse(json_str);
        if (json == NULL)
        {
            return -2; // Invalid JSON
        }

        cJSON *data_item = cJSON_GetObjectItem(json, "data");
        if (data_item == NULL || !cJSON_IsString(data_item))
        {
            cJSON_Delete(json);
            return -3; // No data field or not a string
        }

        // Decode base64 data and store it in RAM
        size_t decoded_size;
        uint8_t *decoded_data = base64_decode(data_item->valuestring, &decoded_size);
        if (decoded_data == NULL || decoded_size != metadata->size)
        {
            cJSON_Delete(json);
            return -4; // Base64 decoding failed or size mismatch
        }

        metadata->ptr_to_firmware_in_ram = decoded_data;
        metadata->is_valid = true;

        cJSON_Delete(json);
        return FIRMWARE_DATA_JSON; // successfully parsed firmware data JSON
    }
    else
    {
        return -5; // Unknown JSON type
    }
}


int8_t RafionOTAgetReceivedJsonType(char *json_str, currentApplicationMetadata_t *metadata)
{
    cJSON *json = cJSON_Parse(json_str);
    if (json == NULL)
    {
        metadata->is_valid = false;
        return -1; // Invalid JSON
    }

    if (cJSON_HasObjectItem(json, "size") && cJSON_HasObjectItem(json, "crc") && cJSON_HasObjectItem(json, "start_address"))
    {
        metadata->size = cJSON_GetObjectItem(json, "size")->valueint;
        metadata->crc = cJSON_GetObjectItem(json, "crc")->valueint;
        metadata->start_address = cJSON_GetObjectItem(json, "start_address")->valueint;
        metadata->is_valid = true;
        cJSON_Delete(json);
        return FIRMWARE_METADATA_JSON; // successfully parsed metadata JSON
    }
    else if (cJSON_HasObjectItem(json, "data"))
    {
        metadata->is_valid = true;
        cJSON_Delete(json);
        return FIRMWARE_DATA_JSON; // Firmware data JSON
    }
    else
    {
        metadata->is_valid = false;
        cJSON_Delete(json);
        return -2; // Unknown JSON type
    }
}

bool RafionOTAalocateMemoryForFirmware(currentApplicationMetadata_t *metadata)
{
    // All memory will be allocated in RAM, than copied from RAM to FLASH
    metadata->ptr_to_firmware_in_flash = malloc(metadata->size);
    if (metadata->ptr_to_firmware_in_flash == NULL)
    {
        metadata->is_valid = false;
        return false; // Memory allocation failed
    }
    metadata->is_valid = true;
    return true;
}

int8_t RafionOTACheckCRC32(currentApplicationMetadata_t *metadata, uint32_t expected_crc)
{
    if (metadata->ptr_to_firmware_in_ram == NULL)
    {
        return -2; // No firmware data in RAM
    }

    //CALCULATE CRC32 OF FIRMWARE DATA IN RAM
    uint32_t calculated_crc = HAL_CRC_Calculate(&hcrc, (uint32_t *)metadata->ptr_to_firmware_in_ram, metadata->size / 4); // Assuming firmwareSize is a multiple of 4

    if (calculated_crc == expected_crc)
    {
        return 1; // CRC match
    }
    else
    {
        return 0; // CRC mismatch
    }
}

int32_t RafionOTACopyFromRAMToFlash(currentApplicationMetadata_t *metadata)
{
    if (metadata->ptr_to_firmware_in_ram == NULL || metadata->ptr_to_firmware_in_flash == NULL)
    {
        return -1; // No firmware data in RAM or flash
    }

    if (metadata->size == 0)
    {
        return -2; // No firmware data to copy
    }

    //Alowed ranges for start address are 0x08020000 to 0x080FFFFF
    if (metadata->start_address < 0x08020000 || metadata->start_address > 0x080FFFFF)
    {
        return -3; // Invalid start address
    }

    //Check if the firmware size is within the allowed range
    if (metadata->start_address + metadata->size > 0x080FFFFF)
    {
        return -4; // Firmware size exceeds allowed range
    }

    // Copy firmware data from RAM to flash
    memcpy(metadata->ptr_to_firmware_in_flash, metadata->ptr_to_firmware_in_ram, metadata->size);
    return 1; // Success
}

//}
//bool updateFirmware()






