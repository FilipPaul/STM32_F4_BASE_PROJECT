#include "relayCards.h"
#include "main.h"


ExternalPortMapping EXTERNAL_PORT_MAPPING = {
    .EXT_PIN_1_pin = EXT_PIN_1_Pin,
    .EXT_PIN_2_pin = EXT_PIN_2_Pin,
    .EXT_PIN_3_pin = EXT_PIN_3_Pin,
    .EXT_PIN_4_pin = EXT_PIN_4_Pin,
    .EXT_PIN_5_pin = EXT_PIN_5_Pin,
    .EXT_PIN_6_pin = EXT_PIN_6_Pin,
    .EXT_PIN_7_pin = EXT_PIN_7_Pin,
    .EXT_PIN_8_pin = EXT_PIN_8_Pin,
    .EXT_OE_pin = EXT_OE_Pin,

    .EXT_PIN_1_port = EXT_PIN_1_GPIO_Port,
    .EXT_PIN_2_port = EXT_PIN_2_GPIO_Port,
    .EXT_PIN_3_port = EXT_PIN_3_GPIO_Port,
    .EXT_PIN_4_port = EXT_PIN_4_GPIO_Port,
    .EXT_PIN_5_port = EXT_PIN_5_GPIO_Port,
    .EXT_PIN_6_port = EXT_PIN_6_GPIO_Port,
    .EXT_PIN_7_port = EXT_PIN_7_GPIO_Port,
    .EXT_PIN_8_port = EXT_PIN_8_GPIO_Port,
    .EXT_OE_port = EXT_OE_GPIO_Port
};

ExternalRelayCard EXTERNAL_32_RELAY_CARD = {
    .pin_E3 = EXT_PIN_4_Pin,
    .pin_A = EXT_PIN_1_Pin,
    .pin_B = EXT_PIN_2_Pin,
    .pin_C = EXT_PIN_3_Pin,
    .pin_D1 = EXT_PIN_5_Pin,
    .pin_D2 = EXT_PIN_6_Pin,
    .pin_D3 = EXT_PIN_7_Pin,
    .pin_D4 = EXT_PIN_8_Pin,
    
    .pin_E3_port = EXT_PIN_4_GPIO_Port,
    .pin_A_port = EXT_PIN_1_GPIO_Port,
    .pin_B_port = EXT_PIN_2_GPIO_Port,
    .pin_C_port = EXT_PIN_3_GPIO_Port,
    .pin_D1_port = EXT_PIN_5_GPIO_Port,
    .pin_D2_port = EXT_PIN_6_GPIO_Port,
    .pin_D3_port = EXT_PIN_7_GPIO_Port,
    .pin_D4_port = EXT_PIN_8_GPIO_Port,

    .current_satus = 0,
    .number_of_relays = 32
};


ExternalRelayCard EXTERNAL_8_RELAY_CARD = {
    .pin_E3 = EXT_PIN_1_Pin,
    .pin_A = EXT_PIN_2_Pin,
    .pin_B = EXT_PIN_3_Pin,
    .pin_C = EXT_PIN_4_Pin,
    .pin_D1 = EXT_PIN_5_Pin,
    .pin_D2 = EXT_PIN_6_Pin,
    .pin_D3 = EXT_PIN_7_Pin,
    .pin_D4 = EXT_PIN_8_Pin,
    
    .pin_E3_port = EXT_PIN_1_GPIO_Port,
    .pin_A_port = EXT_PIN_2_GPIO_Port,
    .pin_B_port = EXT_PIN_3_GPIO_Port,
    .pin_C_port = EXT_PIN_4_GPIO_Port,
    .pin_D1_port = EXT_PIN_5_GPIO_Port,
    .pin_D2_port = EXT_PIN_6_GPIO_Port,
    .pin_D3_port = EXT_PIN_7_GPIO_Port,
    .pin_D4_port = EXT_PIN_8_GPIO_Port,

    .current_satus = 0,
    .number_of_relays = 8
};

uint8_t relayCardSetSingleRelay(uint8_t relay_number, uint8_t relay_state, ExternalRelayCard* RelayCard){
    if(RelayCard->number_of_relays == 8){
        _setSingleRelayFor8Cards(relay_number, relay_state, RelayCard);
    }
    else if(RelayCard->number_of_relays == 32){
        _relaySetSingleRelayFor32Cards(relay_number, relay_state, RelayCard);
    }
    //SAVE NEW STATE of all 8 relays:
    else{
        return 0;
    }
    return 1;
}

uint8_t _setSingleRelayFor8Cards(uint8_t relay_number, uint8_t relay_state, ExternalRelayCard* RelayCard){
    // RELAY NUMBERS INDEXED FROM 0
    if (relay_number == 0){
        HAL_GPIO_WritePin(RelayCard->pin_E3_port, RelayCard->pin_E3, relay_state);
    }
    else if (relay_number == 1){
        HAL_GPIO_WritePin(RelayCard->pin_A_port, RelayCard->pin_A, relay_state);
    }
    else if (relay_number == 2){
        HAL_GPIO_WritePin(RelayCard->pin_B_port, RelayCard->pin_B, relay_state);
    }
    else if (relay_number == 3){
        HAL_GPIO_WritePin(RelayCard->pin_C_port, RelayCard->pin_C, relay_state);
    }
    else if (relay_number == 4){
        HAL_GPIO_WritePin(RelayCard->pin_D1_port, RelayCard->pin_D1, relay_state);
    }
    else if (relay_number == 5){
        HAL_GPIO_WritePin(RelayCard->pin_D2_port, RelayCard->pin_D2, relay_state);
    }
    else if (relay_number == 6){
        HAL_GPIO_WritePin(RelayCard->pin_D3_port, RelayCard->pin_D3, relay_state);
    }
    else if (relay_number == 7){
        HAL_GPIO_WritePin(RelayCard->pin_D4_port, RelayCard->pin_D4, relay_state);
    }
    else{
        return 0;
    }

    //Update new relay status
    if(relay_state == 1){
        RelayCard->current_satus = RelayCard->current_satus | (1 << relay_number);
    }
    else{
        RelayCard->current_satus= RelayCard->current_satus & ~(1 << relay_number);
    }
    return 1;
}



uint8_t _relaySetSingleRelayFor32Cards(uint8_t relay_number, uint8_t relay_state, ExternalRelayCard* RelayCard){
    //*First of all set E3 to 0 -> CLK pin of all flipflops (connected to the SN74..238) is set to 0.
    //set clk of all flipflops to 0
    //shiftRegSetSinglePin(pin_E3, 0, RelayCard->shift_register);
    HAL_GPIO_WritePin(RelayCard->pin_E3_port, RelayCard->pin_E3, GPIO_PIN_RESET);
    _2500_ns_delay(5);
    
    //SET value to the D1,D2,D3,D4 pins -> they are grouped by 4 - current state of 4 pins:
    static uint8_t Dpins_state;
    Dpins_state = (RelayCard->current_satus >> ( (uint8_t)(relay_number / 4) *4 ) ) & 0x0F;
    //printf("Dpins_state before: %d\n", Dpins_state);

    if (relay_state == 1){
        Dpins_state = Dpins_state | (1 << relay_number % 4);
    }
    else{
        Dpins_state = Dpins_state & ~(1 << relay_number % 4);
    }
    //printf("Dpins_state after: %d\n", Dpins_state);
    // set new states:
    //shiftRegSetSinglePin(pin_D1, Dpins_state & 0x01, RelayCard->shift_register);
    HAL_GPIO_WritePin(RelayCard->pin_D1_port, RelayCard->pin_D1, Dpins_state & 0x01);
    //shiftRegSetSinglePin(pin_D2, Dpins_state & 0x02, RelayCard->shift_register);
    HAL_GPIO_WritePin(RelayCard->pin_D2_port, RelayCard->pin_D2, Dpins_state & 0x02);
    //shiftRegSetSinglePin(pin_D3, Dpins_state & 0x04, RelayCard->shift_register);
    HAL_GPIO_WritePin(RelayCard->pin_D3_port, RelayCard->pin_D3, Dpins_state & 0x04);
    //shiftRegSetSinglePin(pin_D4, Dpins_state & 0x08, RelayCard->shift_register);
    HAL_GPIO_WritePin(RelayCard->pin_D4_port, RelayCard->pin_D4, Dpins_state & 0x08);


    //set A,B,C pins to the value coresponding to the relay number
    //shiftRegSetSinglePin(pin_A, (relay_number / 4) & 0x01, RelayCard->shift_register);
    HAL_GPIO_WritePin(RelayCard->pin_A_port, RelayCard->pin_A, (relay_number / 4) & 0x01);
    //shiftRegSetSinglePin(pin_B, (relay_number / 4) & 0x02, RelayCard->shift_register);
    HAL_GPIO_WritePin(RelayCard->pin_B_port, RelayCard->pin_B, (relay_number / 4) & 0x02);
    //shiftRegSetSinglePin(pin_C, (relay_number / 4) & 0x04, RelayCard->shift_register);
    HAL_GPIO_WritePin(RelayCard->pin_C_port, RelayCard->pin_C, (relay_number / 4) & 0x04);
    
    //set clk of all flipflops to 1
    _2500_ns_delay(5);
    //shiftRegSetSinglePin(pin_E3, 1, RelayCard->shift_register);
    HAL_GPIO_WritePin(RelayCard->pin_E3_port, RelayCard->pin_E3, GPIO_PIN_SET);
    _2500_ns_delay(5);
    //set clk of all flipflops to 0
    //shiftRegSetSinglePin(pin_E3, 0, RelayCard->shift_register);
    HAL_GPIO_WritePin(RelayCard->pin_E3_port, RelayCard->pin_E3, GPIO_PIN_RESET);
    //Update new relay status

    if(relay_state == 1){
        RelayCard->current_satus = RelayCard->current_satus | (1 << relay_number);
    }
    else{
        RelayCard->current_satus= RelayCard->current_satus & ~(1 << relay_number);
    }

    return 1;
    
}
uint8_t relayCardSetAllRelays(uint8_t state, ExternalRelayCard* RelayCard){
    for (uint8_t i = 0; i < RelayCard->number_of_relays; i++)
    {
        relayCardSetSingleRelay(i, state, RelayCard);
    }
    
    return 1;
}

uint8_t relayCardSetPort(uint32_t value, ExternalRelayCard* RelayCard){
    for (uint8_t i = 0; i < RelayCard->number_of_relays; i++)
    {
        relayCardSetSingleRelay(i, (value >> i) & 0x01, RelayCard);
    }
    
    return 1;
}