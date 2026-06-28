## I2C
**Generic form:**

```
I2C [1-3] <ACTION> [...]
```

| Command | Usage Example | Short Description | Possible Responses |
|---------|---------------|------------------|-------------------|
| I2C [1-3] REINIT [SPEED IN HZ] | I2C 1 REINIT 100000 | Reinitialize I2C bus with optional speed | OK;I2C 1 REINIT SPEED_HZ:100000\n<br>ERROR;I2C requires BUS and ACTION\n<br>ERROR;I2C bus must be 1, 2 or 3\n<br>ERROR;I2C REINIT speed must be > 0\n<br>ERROR;I2C 1 REINIT FAILED SPEED_HZ:100000\n<br>ERROR;No such I2C action\n |

---

## RS232
**Generic form:**

```
RS232 [4-5] SEND [MESSAGE...]
```

| Command | Usage Example | Short Description | Possible Responses |
|---------|---------------|------------------|-------------------|
| RS232 [4-5] SEND [MESSAGE...] | RS232 4 SEND Hello from USB with spaces | Send all remaining subcommands joined by spaces to UART4 or UART5 | OK;RS232 4 SENT_BYTES:26\n<br>ERROR;RS232 requires UART and ACTION\n<br>ERROR;RS232 UART must be 4 or 5\n<br>ERROR;RS232 4 SEND requires payload\n<br>ERROR;RS232 4 SEND payload too long\n<br>ERROR;RS232 4 SEND FAILED HAL_ERROR:0x00000000\n<br>ERROR;No such RS232 action\n |

---

## EEPROM
**Generic form:**

```
EEPROM <ACTION> [...]
```

| Command | Usage Example | Short Description | Possible Responses |
|---------|---------------|------------------|-------------------|
| EEPROM WRITE IP [A] [B] [C] [D] | EEPROM WRITE IP 192 168 1 2 | Write custom IP address | OK;Succesfully write IP to EEPROM:192.168.1.2\n<br>ERROR;WRITING CUSTOM IP TO EEPROM FAILED\n<br>ERROR;WRITING CUSTOM IP FLAG TO EEPROM FAILED\n |
| EEPROM WRITE SUBNET [A] [B] [C] [D] | EEPROM WRITE SUBNET 255 255 255 0 | Write custom subnet | OK;Succesfully write SUBNET to EEPROM:255.255.255.0\n<br>ERROR;WRITING CUSTOM SUBNET TO EEPROM FAILED\n<br>ERROR;WRITING CUSTOM SUBNET FLAG TO EEPROM FAILED\n |
| EEPROM WRITE GATEWAY [A] [B] [C] [D] | EEPROM WRITE GATEWAY 192 168 1 1 | Write custom gateway | OK;Succesfully write GATEWAY to EEPROM:192.168.1.1\n<br>ERROR;WRITING CUSTOM GATEWAY TO EEPROM FAILED\n<br>ERROR;WRITING CUSTOM GATEWAY FLAG TO EEPROM FAILED\n |
| EEPROM WRITE CUSTOM_MAC [XX] [XX] [XX] [XX] [XX] [XX] | EEPROM WRITE CUSTOM_MAC 3D 00 00 15 3E D0 | Write custom MAC address | OK;Succesfully write CUSTOM MAC to EEPROM:3D::00::00::15::3E::D0\n<br>ERROR;WRITING CUSTOM MAC TO EEPROM FAILED\n<br>ERROR;WRITING CUSTOM MAC FLAG TO EEPROM FAILED\n |
| EEPROM WRITE TEMPERATURE_OFFSET [VALUE_IN_C] | EEPROM WRITE TEMPERATURE_OFFSET 10 | Write custom temperature offset | OK;Succesfully write TEMPERATURE OFFSET TO EEPROM:10\n<br>ERROR;TEMPERATURE OFFSET IS OUT OF RANGE should be from -32 to 31\n<br>ERROR;WRITING TEMPERATURE OFFSET TO EEPROM FAILED\n<br>ERROR;WRITING CUSTOM MAC FLAG TO FROM EEPROM FAILED\n |
| EEPROM READ IP | EEPROM READ IP | Read custom IP address | OK;IP 192.168.1.2\n<br>ERROR;CUSTOM IP IS NOT DEFINED IN EEPROM\n<br>ERROR;ERROR IN READING CUSTOM IP\n<br>ERROR;ERROR IN READING CUSTOM IP FLAG\n |
| EEPROM READ SUBNET | EEPROM READ SUBNET | Read custom subnet | OK;SUBNET 255.255.255.0\n<br>ERROR;CUSTOM SUBNET IS NOT DEFINED IN EEPROM\n<br>ERROR;ERROR IN READING CUSTOM SUBNET\n<br>ERROR;ERROR IN READING CUSTOM SUBNET FLAG\n |
| EEPROM READ GATEWAY | EEPROM READ GATEWAY | Read custom gateway | OK;GATEWAY 192.168.1.1\n<br>ERROR;CUSTOM GATEWAY IS NOT DEFINED IN EEPROM\n<br>ERROR;ERROR IN READING CUSTOM GATEWAY\n<br>ERROR;ERROR IN READING CUSTOM GATEWAY FLAG\n |
| EEPROM READ CUSTOM_MAC | EEPROM READ CUSTOM_MAC | Read custom MAC address | OK;CUSTOM MAC 3D:00:00:15:3E:D0\n<br>ERROR;CUSTOM MAC IS NOT DEFINED IN EEPROM\n<br>ERROR;ERROR IN READING CUSTOM MAC\n<br>ERROR;ERROR IN READING CUSTOM MAC FLAG\n |
| EEPROM READ DEFAULT_MAC | EEPROM READ DEFAULT_MAC | Read default MAC address | OK;DEFAULT MAC 3D::00::00::15::3E::D0\n<br>ERROR;ERROR IN READING DEFAULT MAC\n |
| EEPROM READ TEMPERATURE_OFFSET | EEPROM READ TEMPERATURE_OFFSET | Read custom temperature offset | OK;TEMPERATURE OFFSET:10\n<br>ERROR;CUSTOM TEMPERATURE OFFSET IS NOT DEFINED IN EEPROM\n<br>ERROR;ERROR IN READING CUSTOM TEMPERATURE OFFSET\n<br>ERROR;ERROR IN READING CUSTOM TEMPERATURE OFFSET FLAG\n |
| EEPROM ERASE | EEPROM ERASE | Erase all EEPROM data | OK; EEPROM ERASED\n<br>ERROR; EEPROM ERASE FAILED\n |
| EEPROM ERASE_CUSTOM [IP\|SUBNET\|GATEWAY\|CUSTOM_MAC\|TEMPERATURE_OFFSET] | EEPROM ERASE_CUSTOM IP | Erase custom field | OK;CUSTOM IP ERASED\n<br>ERROR; Unknown subcommand: EEPROM ERASE_CUSTOM <field>\n |

---

## I2C_EXPANDER
**Generic form:**

```
I2C_EXPANDER <ACTION> [...]
```

| Command | Usage Example | Short Description | Possible Responses |
|---------|---------------|------------------|-------------------|
| I2C_EXPANDER SELECT_CHANNEL [0-7] | I2C_EXPANDER SELECT_CHANNEL 2 | Select expander channel | OK;I2C_EXPANDER SELECT_CHANNEL:2\n<br>ERROR;I2C_EXPANDER SELECT_CHANNEL FAILED HAL_ERROR:<code>\n |
| I2C_EXPANDER GET_SELECTED_CHANNEL | I2C_EXPANDER GET_SELECTED_CHANNEL | Get selected channel | OK;I2C_EXPANDER SELECTED_CHANNEL:2\n<br>OK;I2C_EXPANDER SELECTED_CHANNEL:NONE\n<br>ERROR;I2C_EXPANDER GET_SELECTED_CHANNEL FAILED HAL_ERROR:<code>\n |
| I2C_EXPANDER RESET [0/1] | I2C_EXPANDER RESET 1 | Reset expander | OK;I2C_EXPANDER RESET PERFORMED:1\n |
| I2C_EXPANDER INIT | I2C_EXPANDER INIT | Initialize expander | OK;I2C_EXPANDER INITIALIZED SUCCESSFULLY\n<br>ERROR;I2C_EXPANDER INITIALIZATION FAILED HAL_ERROR:<code>\n |
| I2C_EXPANDER WRITE_CONTROL_REGISTER [VALUE] | I2C_EXPANDER WRITE_CONTROL_REGISTER 255 | Write control register | OK;I2C_EXPANDER WRITE_CONTROL_REGISTER VALUE:255\n<br>ERROR;I2C_EXPANDER WRITE_CONTROL_REGISTER FAILED HAL_ERROR:<code>\n |
| I2C_EXPANDER READ_CONTROL_REGISTER | I2C_EXPANDER READ_CONTROL_REGISTER | Read control register | OK;I2C_EXPANDER CONTROL_REGISTER_VALUE:255\n<br>ERROR;I2C_EXPANDER READ_CONTROL_REGISTER FAILED HAL_ERROR:<code>\n |
| I2C_EXPANDER SET_SPEED [VALUE IN HZ] | I2C_EXPANDER SET_SPEED 100000 | Set expander I2C speed | OK;I2C_EXPANDER SET_SPEED:100000\n |

---

## EXT_32_RELAY_CARD
**Generic form:**

```
EXT_32_RELAY_CARD <ACTION> [...]
```

| Command | Usage Example | Short Description | Possible Responses |
|---------|---------------|------------------|-------------------|
| EXT_32_RELAY_CARD RELAY [1-32] [0/1] | EXT_32_RELAY_CARD RELAY 1 1 | Set relay state | OK;EXT_32_RELAY_CARD NEWPORTVALUE:0, RELAY:0 SET TO VALUE:1\n |
| EXT_32_RELAY_CARD PORT [VALUE] | EXT_32_RELAY_CARD PORT 65535 | Set all relays by port value | OK;EXT_32_RELAY_CARD SET TO PORT VALUE:65535\n |

---

## DPDT_RELAYS
**Generic form:**

```
DPDT_RELAYS <ACTION> [...]
```

| Command | Usage Example | Short Description | Possible Responses |
|---------|---------------|------------------|-------------------|
| DPDT_RELAYS SET_PORT [VALUE] | DPDT_RELAYS SET_PORT 255 | Set DPDT relay port value | OK;DPDT_RELAYS SET_PORT:255\n |
| DPDT_RELAYS SET_RELAY [INDEX] [0/1] | DPDT_RELAYS SET_RELAY 1 1 | Set single DPDT relay | OK;DPDT_RELAYS SET_RELAY:1 TO VALUE:1\n |

---

## VENTILS
**Generic form:**

```
VENTILS <ACTION> [...]
```

| Command | Usage Example | Short Description | Possible Responses |
|---------|---------------|------------------|-------------------|
| VENTILS SET_PORT [VALUE] | VENTILS SET_PORT 255 | Set ventil port value | OK;VENTILS SET_PORT:255\n |
| VENTILS SET_VENTIL [INDEX] [0/1] | VENTILS SET_VENTIL 1 1 | Set single ventil | OK;VENTILS SET_VENTIL:1 TO VALUE:1\n |

---

## INPUTS
**Generic form:**

```
INPUTS <ACTION> [...]
```

| Command | Usage Example | Short Description | Possible Responses |
|---------|---------------|------------------|-------------------|
| INPUTS READ_ALL | INPUTS READ_ALL | Read all input port values | OK;INPUTS READ PORT VALUE:255\n |
| INPUTS READ_INPUT [INDEX] | INPUTS READ_INPUT 1 | Read single input pin | OK;INPUT PIN:1 VALUE:1\n |

---

## ADC
**Generic form:**

```
ADC2_IN3? [NUMBER_OF_SAMPLES]
ADC2_IN6? [NUMBER_OF_SAMPLES]
```

| Command | Usage Example | Short Description | Possible Responses |
|---------|---------------|------------------|-------------------|
| ADC2_IN3? [NUMBER_OF_SAMPLES] | ADC2_IN3? 100 | Read ADC2 channel 3 and return the average value from the requested number of samples | OK;ADC2_IN3:1234\n |
| ADC2_IN6? [NUMBER_OF_SAMPLES] | ADC2_IN6? 100 | Read ADC2 channel 6 and return the average value from the requested number of samples | OK;ADC2_IN6:1234\n |

---

## SYSTEM/INFO COMMANDS
**Generic form:**

```
HELP
*IDN?
*ENTER_BOOTLOADER
*RST
*IP?
*INFO?
```

| Command | Usage Example | Short Description | Possible Responses |
|---------|---------------|------------------|-------------------|
| HELP | HELP | Show help | OK;Help command received\n |
| *IDN? | *IDN? | Get board identification | OK;RAFIONTECH s.r.o. - E0000010(FUT CTRL BOARD) V1.0\n |
| *ENTER_BOOTLOADER | *ENTER_BOOTLOADER | Enter MCU bootloader | OK;GOING TO BOOTLOADER\n |
| *RST | *RST | Reset MCU | (no response, MCU resets) |
| *IP? | *IP? | Get current IP address | OK;IP address:192.168.1.2\n |
| *INFO? | *INFO? | Get board info and network config | OK;RAFIONTECH s.r.o. - E0000010(FUT CTRL BOARD) V1.0, ...\n |

---

# Command Parser Reference

## TOF SENSORS
**Generic form:**

```
TOF [0-7] <ACTION> [...]
TOF [0-7] GET <FLAG|LAST_MEASURED_DISTANCE|LIMITS>
TOF [0-7] RESET <FLAG>
```

### Measurement Control
| Command | Usage Example | Description | Response | Allowed During Auto-Measurement? |
|---------|---------------|-------------|----------|-------------------------------|
| TOF [0-7] START_AUTO_MEASUREMENT | TOF 0 START_AUTO_MEASUREMENT | Start automated ranging | OK;TOF 0 AUTO_MEASUREMENT:STARTED<br>ERROR;TOF sensor index must be 0-7 and initialized | YES |
| TOF [0-7] STOP_AUTO_MEASUREMENT | TOF 0 STOP_AUTO_MEASUREMENT | Stop automated ranging | OK;TOF 0 AUTO_MEASUREMENT:STOPPED<br>ERROR;TOF sensor index must be 0-7 and initialized | YES |
| TOF [0-7] IS_AUTO_MEASUREMENT_RUNNING | TOF 0 IS_AUTO_MEASUREMENT_RUNNING | Check if auto-measurement is running | OK;TOF 0 AUTO_MEASUREMENT_RUNNING:1 | YES |

### Status & Flags
| Command | Usage Example | Description | Response | Allowed During Auto-Measurement? |
|---------|---------------|-------------|----------|-------------------------------|
| TOF [0-7] GET IN_LIMIT_FLAG | TOF 0 GET IN_LIMIT_FLAG | Get inside-limit detection flag | OK;TOF 0 IN_LIMIT_FLAG:1 | YES |
| TOF [0-7] GET OUT_OF_LIMIT_FLAG | TOF 0 GET OUT_OF_LIMIT_FLAG | Get out-of-limit detection flag | OK;TOF 0 OUT_OF_LIMIT_FLAG:0 | YES |
| TOF [0-7] GET ERROR_FLAG | TOF 0 GET ERROR_FLAG | Get error flag (see error codes) | OK;TOF 0 ERROR_FLAG:<code> | YES |
| TOF [0-7] RESET IN_LIMIT_FLAG | TOF 0 RESET IN_LIMIT_FLAG | Reset inside-limit flag | OK;TOF 0 IN_LIMIT_FLAG_RESET | YES |
| TOF [0-7] RESET OUT_OF_LIMIT_FLAG | TOF 0 RESET OUT_OF_LIMIT_FLAG | Reset out-of-limit flag | OK;TOF 0 OUT_OF_LIMIT_FLAG_RESET | YES |
| TOF [0-7] RESET ERROR_FLAG | TOF 0 RESET ERROR_FLAG | Reset error flag | OK;TOF 0 ERROR_FLAG_RESET | YES |

### Measurement Results
| Command | Usage Example | Description | Response | Allowed During Auto-Measurement? |
|---------|---------------|-------------|----------|-------------------------------|
| TOF [0-7] GET LAST_MEASURED_DISTANCE | TOF 0 GET LAST_MEASURED_DISTANCE | Get last measured distance | OK;TOF 0 LAST_MEASURED_DISTANCE_MM:1234 | YES |
| TOF [0-7] GET_DISTANCE [TIMEOUT IN MS] | TOF 0 GET_DISTANCE 100 | Get distance (manual, with timeout) | OK;TOF 0 DISTANCE_MM:123<br>ERROR;TOF 0 GET_DISTANCE FAILED | NO |

### Configuration
| Command | Usage Example | Description | Response | Allowed During Auto-Measurement? |
|---------|---------------|-------------|----------|-------------------------------|
| TOF [0-7] SET_MODE [SHORT/LONG\|1/2] | TOF 0 SET_MODE SHORT | Set ranging mode | OK;TOF 0 MODE:1<br>ERROR;TOF 0 SET_MODE requires SHORT/LONG or 1/2<br>ERROR;TOF 0 SET_MODE FAILED | NO |
| TOF [0-7] SET_TIMING_BUDGET [BUDGET IN MS] | TOF 0 SET_TIMING_BUDGET 20 | Set timing budget | OK;TOF 0 TIMING_BUDGET_MS:20<br>ERROR;TOF 0 SET_TIMING_BUDGET requires value in ms<br>ERROR;TOF 0 SET_TIMING_BUDGET FAILED | NO |
| TOF [0-7] SET_INTERMEASUREMENT [PERIOD IN MS] | TOF 0 SET_INTERMEASUREMENT 50 | Set intermeasurement period | OK;TOF 0 INTERMEASUREMENT_MS:50<br>ERROR;TOF 0 SET_INTERMEASUREMENT requires value in ms<br>ERROR;TOF 0 SET_INTERMEASUREMENT FAILED | NO |
| TOF [0-7] SET_LIMITS [MIN IN MM] [MAX IN MM] | TOF 0 SET_LIMITS 10 1000 | Set min/max distance limits | OK;TOF 0 LIMITS MIN_MM:10 MAX_MM:1000<br>ERROR;TOF 0 SET_LIMITS requires MIN and MAX in mm | YES |
| TOF [0-7] DO_LIMIT_CALIBRATION [TIME_MS] [PERCENTAGE_OFFSET] | TOF 0 DO_LIMIT_CALIBRATION 1000 10 | Calibrate min/max distance limits automatically. TIME_MS: calibration duration, PERCENTAGE_OFFSET: offset 0-100 | OK;TOF 0 LIMIT CALIBRATION DONE MIN:<min>,MAX:<max><br>ERROR;TOF 0 DO_LIMIT_CALIBRATION FAILED<br>ERROR;TOF 0 DO_LIMIT_CALIBRATION requires TIME_OF_CALIBRATION in ms and PERCENTAGE_OFFSET | NO |
| TOF [0-7] GET LIMITS | TOF 0 GET LIMITS | Get min/max distance limits | OK;TOF 0 LIMITS MIN_MM:10 MAX_MM:1000 | YES |
| TOF [0-7] REINIT | TOF 0 REINIT | Reinitialize sensor | OK;TOF 0 REINITIALIZED<br>ERROR;TOF 0 REINIT CHANNEL_SELECT_FAILED<br>ERROR;TOF 0 REINIT FAILED | NO |
| TOF [0-7] FORCE_HARD_RESET_WITHOUT_SELECT | TOF 0 FORCE_HARD_RESET_WITHOUT_SELECT | Reinitialize sensor | OK;TOF 0 REINITIALIZED<br>ERROR;TOF 0 REINIT CHANNEL_SELECT_FAILED<br>ERROR;TOF 0 REINIT FAILED | NO |

#### Error Flag Codes

All ToF sensor errors during automated measurement and initialization are reported via the `detected_error_during_automated_measurement` flag. Use `TOF [0-7] GET ERROR_FLAG` to read it, and `TOF [0-7] RESET ERROR_FLAG` to clear it.

| Code | Meaning |
|------|---------|
| 0 | No error |
| 1 | I2C error |
| 2 | Timeout |
| 3 | Invalid parameter |
| 4 | Not supported |
| 5 | Unknown error |
| 6 | I2C expander initialization failed |
| 7 | I2C expander channel select failed |
| 8 | I2C bus restore failed |
| 9 | ToF sensor initialization failed |
| 10 | ToF sensor reinitialization failed |
| 11 | Failed to set distance mode |
| 12 | Failed to set timing budget |
| 13 | Failed to set intermeasurement period |
| 14 | Measurement error (distance read failure) |

**Example:**

```
TOF 0 GET ERROR_FLAG
OK;TOF 0 ERROR_FLAG:0

TOF 0 RESET ERROR_FLAG
OK;TOF 0 ERROR_FLAG_RESET
```

---