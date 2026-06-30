#include "ETHIXRelayPwrCard.h"

uint8_t setControlRegisterValue(ETHIXRelayPwrCard* card, uint8_t value){

  //LOAD VALUE TO CONTROL BIT REGISTER
  //MSB OF VALUE CORRESPONDS TO THE QA PIN OF THE SHIFT REGISTER
  //LSB OF VALUE CORRESPONDS TO THE QH PIN OF THE SHIFT REGISTER

  //PUT LATCH PIN TO LOW
  HAL_GPIO_WritePin( card->SHFT_RCLK_port,card->SHFT_RCLK_pin, GPIO_PIN_RESET);

  //PUT CLK PIN TO LOW:
  HAL_GPIO_WritePin( card->SHFT_SRCLK_port,card->SHFT_SRCLK_pin, GPIO_PIN_RESET);

  //PUT CLEAR PIN TO HIGH -> disable clear
  HAL_GPIO_WritePin( card->SHFT_nSRCLR_port,card->SHFT_nSRCLR_pin, GPIO_PIN_SET);
  
  //ENABLE SHIFT REGISTER:
  HAL_GPIO_WritePin( card->SHFT_nOE_port,card->SHFT_nOE_pin, GPIO_PIN_RESET);

  _2500_ns_delay(DELAY_MULTIPLES_OF_2500_NS);
  card->last_control_register_value = value;

  for (uint8_t i = 0; i < 8; i++)
    {
        uint8_t pin_phy_value = (card->last_control_register_value >> i) & 0x01;
        HAL_GPIO_WritePin(card->SHFT_SER_port, card->SHFT_SER_pin, pin_phy_value);
        _2500_ns_delay(DELAY_MULTIPLES_OF_2500_NS);
        
        DO_CLK_PULSE(card->SHFT_SRCLK_port, card->SHFT_SRCLK_pin);
        _2500_ns_delay(DELAY_MULTIPLES_OF_2500_NS*2);
    }
  //DO LATCH PULSE
  DO_CLK_PULSE(card->SHFT_RCLK_port,card->SHFT_RCLK_pin);

  return 0;
}

uint8_t setPowerCardRegisterValue(ETHIXRelayPwrCard* card, uint32_t value){
  //LOAD VALUE TO RELAYCARD REGISTER
  //MSB OF VALUE CORRESPONDS TO THE QA PIN OF THE FIRST SHIFT REGISTER
  //LSB OF VALUE CORRESPONDS TO THE QH PIN OF THE LAST SHIFT REGISTER

  card->last_Card_register_value = value;

  //PUT SER,LATCH, CLK, OE, TO LOW and leave other pins unchanged
  card-> last_control_register_value &= ~(1UL << PWR_CARD_CTR_SHFT_SER);
  card-> last_control_register_value &= ~(1UL << PWR_CARD_CTR_SHFT_SRCLK);
  card-> last_control_register_value &= ~(1UL << PWR_CARD_CTR_SHFT_nOE);
  card-> last_control_register_value &= ~(1UL << PWR_CARD_CTR_SHFT_RCLK);
  //PIT nSRCLR TO HIGH
  card-> last_control_register_value |= (1UL << PWR_CARD_CTR_SHFT_nSRCLR);
  //SO THE NEW VALUE SHOULD BE: 0bXXX01000

  setControlRegisterValue(card, card->last_control_register_value);
  _2500_ns_delay(DELAY_MULTIPLES_OF_2500_NS);

  
  for (uint32_t i = 0; i < 24; i++) //3 shift regs in series -> 24 bits
  {
    //PARSE BITS OF VALUE
    uint8_t pin_phy_value = (card->last_Card_register_value >> i) & 0x01;
    if(pin_phy_value){ //SET SER PIN TO HIGH
      card-> last_control_register_value |= (1UL << PWR_CARD_CTR_SHFT_SER);
    }
    else{ //SET SER PIN TO LOW
      card-> last_control_register_value &= ~(1UL << PWR_CARD_CTR_SHFT_SER);
    }

    setControlRegisterValue(card, card->last_control_register_value);

    //DO CLK PULSE
    card-> last_control_register_value |= (1UL << PWR_CARD_CTR_SHFT_SRCLK);//HIGH
    setControlRegisterValue(card, card->last_control_register_value);
    _2500_ns_delay(DELAY_MULTIPLES_OF_2500_NS);
    card-> last_control_register_value &= ~(1UL << PWR_CARD_CTR_SHFT_SRCLK);//LOW
    setControlRegisterValue(card, card->last_control_register_value);
    _2500_ns_delay(DELAY_MULTIPLES_OF_2500_NS);

  }

  //DO LATCH PULSE
  card-> last_control_register_value |= (1UL << PWR_CARD_CTR_SHFT_RCLK);//HIGH
  setControlRegisterValue(card, card->last_control_register_value);
  _2500_ns_delay(DELAY_MULTIPLES_OF_2500_NS);
  card-> last_control_register_value &= ~(1UL << PWR_CARD_CTR_SHFT_RCLK);//LOW
  setControlRegisterValue(card, card->last_control_register_value);
  return 0;
}

uint8_t initPowerCard(ETHIXRelayPwrCard* card){
    //Put All SHFT pins to GPIO output
    GPIO_InitTypeDef GPIO_structure = {0};
    GPIO_structure.Pin = card->SHFT_SER_pin;
    GPIO_structure.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_structure.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(card->SHFT_SER_port, &GPIO_structure);
    HAL_GPIO_WritePin(card->SHFT_SER_port, card->SHFT_SER_pin, GPIO_PIN_RESET);

    GPIO_structure.Pin = card->SHFT_SRCLK_pin;
    HAL_GPIO_Init(card->SHFT_SRCLK_port, &GPIO_structure);
    HAL_GPIO_WritePin(card->SHFT_SRCLK_port, card->SHFT_SRCLK_pin, GPIO_PIN_RESET);

    GPIO_structure.Pin = card->SHFT_RCLK_pin;
    HAL_GPIO_Init(card->SHFT_RCLK_port, &GPIO_structure);
    HAL_GPIO_WritePin(card->SHFT_RCLK_port, card->SHFT_RCLK_pin, GPIO_PIN_RESET);

    GPIO_structure.Pin = card->SHFT_nSRCLR_pin;
    HAL_GPIO_Init(card->SHFT_nSRCLR_port, &GPIO_structure);
    HAL_GPIO_WritePin(card->SHFT_nSRCLR_port, card->SHFT_nSRCLR_pin, GPIO_PIN_SET);

    GPIO_structure.Pin = card->SHFT_nOE_pin;
    HAL_GPIO_Init(card->SHFT_nOE_port, &GPIO_structure);
    HAL_GPIO_WritePin(card->SHFT_nOE_port, card->SHFT_nOE_pin, GPIO_PIN_SET);

    //Put All MUX DATA PINS TO INPUT PULLUP
    GPIO_structure.Pin = card->MUX_DATA_1_pin;
    GPIO_structure.Mode = GPIO_MODE_INPUT;
    GPIO_structure.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(card->MUX_DATA_1_port, &GPIO_structure);
    HAL_GPIO_WritePin(card->MUX_DATA_1_port, card->MUX_DATA_1_pin, GPIO_PIN_RESET);

    GPIO_structure.Pin = card->MUX_DATA_2_pin;
    HAL_GPIO_Init(card->MUX_DATA_2_port, &GPIO_structure);
    HAL_GPIO_WritePin(card->MUX_DATA_2_port, card->MUX_DATA_2_pin, GPIO_PIN_RESET);

    GPIO_structure.Pin = card->MUX_DATA_3_pin;
    HAL_GPIO_Init(card->MUX_DATA_3_port, &GPIO_structure);
    HAL_GPIO_WritePin(card->MUX_DATA_3_port, card->MUX_DATA_3_pin, GPIO_PIN_RESET);

    return 0;
}
static uint8_t storeMuxReadoutToValue(ETHIXRelayPwrCard* card, GPIO_PinState readout, uint8_t pin_number){
    printf("MUX PIN %d readout: %d\n", pin_number, readout);
    if (readout == GPIO_PIN_SET){
      card->last_MUXes_readout_value |= (1UL << pin_number);
    }
    else{
      card->last_MUXes_readout_value &= ~(1UL << pin_number);
    }
    _2500_ns_delay(DELAY_MULTIPLES_OF_2500_NS*5);
    return 0;
  }

uint8_t readPowrCardMuxValue(ETHIXRelayPwrCard* card){
  //this function store the readout of the MUX pins to card->last_MUXes_readout_value
  //MSB bit corresponds to the D0 pin of first MUX
  //LSB bit corresponds to the D7 pin of the last MUX
  card->last_MUXes_readout_value = 0;
  for (uint8_t i = 0; i < 8; i++) //FOR EACH ADDRESS OF MUX PIN:
  {
    
    //SETUP MUX PINS TO ADDRESS i
    //A0
    if (i & 0b001){
       card-> last_control_register_value |= (1UL << PWR_CARD_CTR_MUX_A0);
    }
    else{
        card-> last_control_register_value &= ~(1UL << PWR_CARD_CTR_MUX_A0);
    }

    //A1
    if (i & 0b010){
       card-> last_control_register_value |= (1UL << PWR_CARD_CTR_MUX_A1);
    }
    else{
        card-> last_control_register_value &= ~(1UL << PWR_CARD_CTR_MUX_A1);
    }


    //A2
    if (i & 0b100){
       card-> last_control_register_value |= (1UL << PWR_CARD_CTR_MUX_A2);
    }
    else{
        card-> last_control_register_value &= ~(1UL << PWR_CARD_CTR_MUX_A2);
    }

    setControlRegisterValue(card, card->last_control_register_value);
    _2500_ns_delay(DELAY_MULTIPLES_OF_2500_NS);

    printf("MUX Address: %d\n", i);
    _2500_ns_delay(DELAY_MULTIPLES_OF_2500_NS*5);
    //Now Read the corresponding input pins and store them into card->last_MUXes_readout_value
    storeMuxReadoutToValue(card, HAL_GPIO_ReadPin(card->MUX_DATA_1_port, card->MUX_DATA_1_pin), 23 - i);
    storeMuxReadoutToValue(card, HAL_GPIO_ReadPin(card->MUX_DATA_2_port, card->MUX_DATA_2_pin), 23-8 - i);
    storeMuxReadoutToValue(card, HAL_GPIO_ReadPin(card->MUX_DATA_3_port, card->MUX_DATA_3_pin), 23-16 - i);
  }
  return 0;
}