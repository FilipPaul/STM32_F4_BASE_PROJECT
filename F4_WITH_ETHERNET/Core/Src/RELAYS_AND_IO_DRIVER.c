#include "RELAYS_AND_IO_DRIVER.h"

struct ShiftRegisters DPDT_RELAY_DRIVER = {
    .ser_pin = RELAY_DRIVER_SER_Pin,
    .srclk_pin = RELAY_DRIVER_SRCLK_Pin,
    .rclk_pin = RELAY_DRIVER_RCLK_Pin,
    .srclr_pin = RELAY_DRIVER_nSRCLR_Pin,
    .noe_pin = RELAY_DRIVER_nOE_Pin,
    .ser_port = RELAY_DRIVER_SER_GPIO_Port,
    .srclk_port = RELAY_DRIVER_SRCLK_GPIO_Port,
    .rclk_port = RELAY_DRIVER_RCLK_GPIO_Port,
    .srclr_port = RELAY_DRIVER_nSRCLR_GPIO_Port,
    .noe_port = RELAY_DRIVER_nOE_GPIO_Port,
    .shift_reg_length = 16,
    .current_satus = 0,
    .endianity = REGISTER_ENDIANITY_1_PIN_IS_MSB
};

struct ShiftRegisters VENTIL_DRIVER = {
    .ser_pin = VENTIL_DRIVER_SER_Pin,
    .srclk_pin = VENTIL_DRIVER_SRCLK_Pin,
    .rclk_pin = VENTIL_DRIVER_RCLK_Pin,
    .srclr_pin = VENTIL_DRIVER_nSRCLR_Pin,
    .noe_pin = VENTIL_DRIVER_nOE_Pin,
    .ser_port = VENTIL_DRIVER_SER_GPIO_Port,
    .srclk_port = VENTIL_DRIVER_SRCLK_GPIO_Port,
    .rclk_port = VENTIL_DRIVER_RCLK_GPIO_Port,
    .srclr_port = VENTIL_DRIVER_nSRCLR_GPIO_Port,
    .noe_port = VENTIL_DRIVER_nOE_GPIO_Port,
    .shift_reg_length = 16,
    .current_satus = 0,
    .endianity = REGISTER_ENDIANITY_1_PIN_IS_MSB
};



struct Multiplexers IO_MUX = {
    .A0_pin = MUX_A0_Pin,
    .A1_pin = MUX_A1_Pin,
    .A2_pin = MUX_A2_Pin,
    .data_out_pins = {MUXes_Dout_1_Pin,MUXes_Dout_2_Pin,MUXes_Dout_3_Pin,MUXes_Dout_4_Pin},
    .mux_length = 8,
    .number_of_paralel_muxes = 4,
    .noe_pin = MUXes_nEN_Pin,
    .A0_port = MUX_A0_GPIO_Port,
    .A1_port = MUX_A1_GPIO_Port,
    .A2_port = MUX_A2_GPIO_Port,
    .data_out_ports = {MUXes_Dout_1_GPIO_Port,MUXes_Dout_2_GPIO_Port,MUXes_Dout_3_GPIO_Port,MUXes_Dout_4_GPIO_Port},
    .noe_port = MUXes_nEN_GPIO_Port,
    .current_satus = 0,
    .endianity = REGISTER_ENDIANITY_1_PIN_IS_MSB
};

RELAY_IO_DRIVER_STRUCT RELAY_IO_DRIVER = {
    .RELAY_SHIFT_REGISTER = &DPDT_RELAY_DRIVER,
    .VENTIL_SHIFT_REGISTER = &VENTIL_DRIVER,
    .INPUT_MUX = &IO_MUX
};



uint8_t RELAY_DRIVER_init(struct RELAY_IO_DRIVER_STRUCT* RELAY_IO_DRIVER){
    loadDataToShiftReg(0, RELAY_IO_DRIVER->RELAY_SHIFT_REGISTER);
    loadDataToShiftReg(0, RELAY_IO_DRIVER->VENTIL_SHIFT_REGISTER);
    //ENABLE MUXES
    HAL_GPIO_WritePin(RELAY_IO_DRIVER->INPUT_MUX->noe_port, RELAY_IO_DRIVER->INPUT_MUX->noe_pin, GPIO_PIN_RESET);
    return 1;
}

uint8_t RELAY_DRIVER_setRelay(uint8_t relay_number, uint8_t state, struct RELAY_IO_DRIVER_STRUCT* RELAY_IO_DRIVER){
    return shiftRegSetSinglePin(relay_number,state, RELAY_IO_DRIVER->RELAY_SHIFT_REGISTER);
}
uint8_t RELAY_DRIVER_setRelayPort(uint32_t value, struct RELAY_IO_DRIVER_STRUCT* RELAY_IO_DRIVER){
    return loadDataToShiftReg(value, RELAY_IO_DRIVER->RELAY_SHIFT_REGISTER);
}
uint8_t RELAY_DRIVER_setVentil(uint8_t ventil_number, uint8_t state, struct RELAY_IO_DRIVER_STRUCT* RELAY_IO_DRIVER){
    return shiftRegSetSinglePin(ventil_number,state, RELAY_IO_DRIVER->VENTIL_SHIFT_REGISTER);
}

uint8_t RELAY_DRIVER_setVentilPort(uint32_t value, struct RELAY_IO_DRIVER_STRUCT* RELAY_IO_DRIVER){
    return loadDataToShiftReg(value, RELAY_IO_DRIVER->VENTIL_SHIFT_REGISTER);
}

uint8_t RELAY_DRIVER_readInput(uint8_t input_number, struct RELAY_IO_DRIVER_STRUCT* RELAY_IO_DRIVER){
    return (~MUXreadAdress(input_number, RELAY_IO_DRIVER->INPUT_MUX)) -254; // -254 is a hack to convert 0 to 1 and 1 to 0
}
uint64_t RELAY_DRIVER_readInputsPort(struct RELAY_IO_DRIVER_STRUCT* RELAY_IO_DRIVER){
    return ~ MUXreadAll(RELAY_IO_DRIVER->INPUT_MUX);
}