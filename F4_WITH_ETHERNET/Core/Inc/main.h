/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

void HAL_TIM_MspPostInit(TIM_HandleTypeDef *htim);

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define EXT_I2C_nRST_Pin GPIO_PIN_6
#define EXT_I2C_nRST_GPIO_Port GPIOE
#define I2C_EEPROM_SDA_Pin GPIO_PIN_0
#define I2C_EEPROM_SDA_GPIO_Port GPIOF
#define I2C_EEPROM_SCL_Pin GPIO_PIN_1
#define I2C_EEPROM_SCL_GPIO_Port GPIOF
#define EXT_PIN_8_Pin GPIO_PIN_2
#define EXT_PIN_8_GPIO_Port GPIOF
#define EXT_PIN_5_Pin GPIO_PIN_3
#define EXT_PIN_5_GPIO_Port GPIOF
#define EXT_PIN_6_Pin GPIO_PIN_4
#define EXT_PIN_6_GPIO_Port GPIOF
#define EXT_PIN_7_Pin GPIO_PIN_5
#define EXT_PIN_7_GPIO_Port GPIOF
#define EXT_OE_Pin GPIO_PIN_6
#define EXT_OE_GPIO_Port GPIOF
#define EXT_PIN_3_Pin GPIO_PIN_7
#define EXT_PIN_3_GPIO_Port GPIOF
#define EEPROM_WP_Pin GPIO_PIN_8
#define EEPROM_WP_GPIO_Port GPIOF
#define EXT_PIN_2_Pin GPIO_PIN_9
#define EXT_PIN_2_GPIO_Port GPIOF
#define EXT_PIN_4_Pin GPIO_PIN_10
#define EXT_PIN_4_GPIO_Port GPIOF
#define EXT_PIN_1_Pin GPIO_PIN_3
#define EXT_PIN_1_GPIO_Port GPIOC
#define ADC_sense_Pin GPIO_PIN_0
#define ADC_sense_GPIO_Port GPIOA
#define OPT_INPUT_6_Pin GPIO_PIN_0
#define OPT_INPUT_6_GPIO_Port GPIOB
#define OPT_INPUT_7_Pin GPIO_PIN_1
#define OPT_INPUT_7_GPIO_Port GPIOB
#define OPT_INPUT_8_Pin GPIO_PIN_11
#define OPT_INPUT_8_GPIO_Port GPIOF
#define OPT_INPUT_9_Pin GPIO_PIN_12
#define OPT_INPUT_9_GPIO_Port GPIOF
#define OPT_INPUT_10_Pin GPIO_PIN_13
#define OPT_INPUT_10_GPIO_Port GPIOF
#define MUX_A2_Pin GPIO_PIN_15
#define MUX_A2_GPIO_Port GPIOF
#define MUX_A1_Pin GPIO_PIN_0
#define MUX_A1_GPIO_Port GPIOG
#define MUX_A0_Pin GPIO_PIN_1
#define MUX_A0_GPIO_Port GPIOG
#define MUXes_Dout_1_Pin GPIO_PIN_7
#define MUXes_Dout_1_GPIO_Port GPIOE
#define MUXes_nEN_Pin GPIO_PIN_8
#define MUXes_nEN_GPIO_Port GPIOE
#define RELAY_DRIVER_RCLK_Pin GPIO_PIN_10
#define RELAY_DRIVER_RCLK_GPIO_Port GPIOE
#define MUXes_Dout_2_Pin GPIO_PIN_11
#define MUXes_Dout_2_GPIO_Port GPIOE
#define MUXes_Dout_3_Pin GPIO_PIN_12
#define MUXes_Dout_3_GPIO_Port GPIOE
#define MUXes_Dout_4_Pin GPIO_PIN_13
#define MUXes_Dout_4_GPIO_Port GPIOE
#define VENTIL_DRIVER_SER_Pin GPIO_PIN_14
#define VENTIL_DRIVER_SER_GPIO_Port GPIOE
#define VENTIL_DRIVER_nOE_Pin GPIO_PIN_15
#define VENTIL_DRIVER_nOE_GPIO_Port GPIOE
#define SPI2_SCK_Pin GPIO_PIN_10
#define SPI2_SCK_GPIO_Port GPIOB
#define SPI2_MISO_Pin GPIO_PIN_14
#define SPI2_MISO_GPIO_Port GPIOB
#define SPI2_MOSI_Pin GPIO_PIN_15
#define SPI2_MOSI_GPIO_Port GPIOB
#define VENTIL_DRIVER_nSRCLR_Pin GPIO_PIN_8
#define VENTIL_DRIVER_nSRCLR_GPIO_Port GPIOD
#define LAN_nRST_Pin GPIO_PIN_9
#define LAN_nRST_GPIO_Port GPIOD
#define STATUS_LED_GREEN_Pin GPIO_PIN_10
#define STATUS_LED_GREEN_GPIO_Port GPIOD
#define STATUS_LED_Pin GPIO_PIN_11
#define STATUS_LED_GPIO_Port GPIOD
#define RELAY_DRIVER_SRCLK_Pin GPIO_PIN_13
#define RELAY_DRIVER_SRCLK_GPIO_Port GPIOD
#define RELAY_DRIVER_nSRCLR_Pin GPIO_PIN_14
#define RELAY_DRIVER_nSRCLR_GPIO_Port GPIOD
#define RELAY_DRIVER_nOE_Pin GPIO_PIN_5
#define RELAY_DRIVER_nOE_GPIO_Port GPIOG
#define RELAY_DRIVER_SER_Pin GPIO_PIN_6
#define RELAY_DRIVER_SER_GPIO_Port GPIOG
#define VENTIL_DRIVER_SRCLK_Pin GPIO_PIN_7
#define VENTIL_DRIVER_SRCLK_GPIO_Port GPIOG
#define VENTIL_DRIVER_RCLK_Pin GPIO_PIN_8
#define VENTIL_DRIVER_RCLK_GPIO_Port GPIOG
#define LIN_EN_Pin GPIO_PIN_8
#define LIN_EN_GPIO_Port GPIOC
#define I2C_EXPANDER_SDA_Pin GPIO_PIN_9
#define I2C_EXPANDER_SDA_GPIO_Port GPIOC
#define I2C_EXPANDER_SCL_Pin GPIO_PIN_8
#define I2C_EXPANDER_SCL_GPIO_Port GPIOA
#define FDCAN_SHDN_Pin GPIO_PIN_3
#define FDCAN_SHDN_GPIO_Port GPIOD
#define FDCAN1_S_Pin GPIO_PIN_4
#define FDCAN1_S_GPIO_Port GPIOD
#define IO_PD7_Pin GPIO_PIN_7
#define IO_PD7_GPIO_Port GPIOD
#define IO_PG9_Pin GPIO_PIN_9
#define IO_PG9_GPIO_Port GPIOG
#define IO_PG10_Pin GPIO_PIN_10
#define IO_PG10_GPIO_Port GPIOG
#define IO_PG11_Pin GPIO_PIN_11
#define IO_PG11_GPIO_Port GPIOG
#define IO_PG12_Pin GPIO_PIN_12
#define IO_PG12_GPIO_Port GPIOG
#define IO_PG13_Pin GPIO_PIN_13
#define IO_PG13_GPIO_Port GPIOG
#define IO_PG14_Pin GPIO_PIN_14
#define IO_PG14_GPIO_Port GPIOG
#define CTRL_15_Pin GPIO_PIN_15
#define CTRL_15_GPIO_Port GPIOG

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
