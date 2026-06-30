/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "cmsis_os.h"
#include "lwip.h"
#include "usb_device.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "usbd_cdc_if.h"
#include "stdio.h"
#include "task.h"
#include "semphr.h"
#include "tcpserver.h"
#include "globals.h"
#include "UsbMessageHanler.h"
#include "commandParser.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
HAL_StatusTypeDef status;
uint32_t Reg_Val;
/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
I2C_HandleTypeDef hi2c2;

IWDG_HandleTypeDef hiwdg;

TIM_HandleTypeDef htim7;
TIM_HandleTypeDef htim11;

osThreadId defaultTaskHandle;
uint32_t defaultTaskBuffer[ 256 ];
osStaticThreadDef_t defaultTaskControlBlock;
osThreadId blinkTaskHandle;
uint32_t blinkTaskBuffer[ 128 ];
osStaticThreadDef_t blinkTaskControlBlock;
osThreadId watchdogTaskHandle;
uint32_t watchdogTaskBuffer[ 128 ];
osStaticThreadDef_t watchdogTaskControlBlock;
osThreadId TcpServerTaskHandle;
uint32_t TcpServerTaskBuffer[ 256 ];
osStaticThreadDef_t TcpServerTaskControlBlock;
osThreadId UsbTaskHandle;
uint32_t UsbTaskBuffer[ 256 ];
osStaticThreadDef_t UsbTaskControlBlock;
osThreadId CommParserTaskHandle;
uint32_t CommParserTaskBuffer[ 700 ];
osStaticThreadDef_t CommParserTaskControlBlock;
osSemaphoreId InitDoneSemaphoreHandle;
osStaticSemaphoreDef_t InitDoneSemaphoreControlBlock;
/* USER CODE BEGIN PV */
osMutexId take_measurement_mutex;
osMutexId isBusyWithSendingResponse_mutex;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_I2C2_Init(void);
static void MX_TIM11_Init(void);
static void MX_IWDG_Init(void);
static void MX_TIM7_Init(void);
void StartDefaultTask(void const * argument);
void blinkTaskEntry(void const * argument);
void watchDogReset(void const * argument);
void TcpServerTaskStart(void const * argument);
void UsbTaskStart(void const * argument);
void CommParserTaskStart(void const * argument);

/* USER CODE BEGIN PFP */
//Redriect printf to USB CDC
int _write(int file, char *ptr, int len)
{
  CDC_Transmit_FS((uint8_t*)ptr, len);
  return len;
}

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
volatile const char *g_fault_reason = NULL;
volatile const char *g_fault_task_name = NULL;
volatile uint32_t g_fault_stack_r0 = 0;
volatile uint32_t g_fault_stack_r1 = 0;
volatile uint32_t g_fault_stack_r2 = 0;
volatile uint32_t g_fault_stack_r3 = 0;
volatile uint32_t g_fault_stack_r12 = 0;
volatile uint32_t g_fault_stack_lr = 0;
volatile uint32_t g_fault_stack_pc = 0;
volatile uint32_t g_fault_stack_psr = 0;
volatile uint32_t g_fault_sp = 0;
volatile uint32_t g_fault_exc_return = 0;
volatile uint32_t g_fault_cfsr = 0;
volatile uint32_t g_fault_hfsr = 0;
volatile uint32_t g_fault_dfsr = 0;
volatile uint32_t g_fault_afsr = 0;
volatile uint32_t g_fault_bfar = 0;
volatile uint32_t g_fault_mmfar = 0;

static void DebugTrap(const char *reason, const char *task_name)
{
  g_fault_reason = reason;
  g_fault_task_name = task_name;
  __disable_irq();
  __BKPT(0);
  while (1)
  {
  }
}

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */
  __enable_irq();


  SCB->SHCSR |= SCB_SHCSR_MEMFAULTENA_Msk |
                SCB_SHCSR_BUSFAULTENA_Msk |
                SCB_SHCSR_USGFAULTENA_Msk;
  SCB->CCR |= SCB_CCR_DIV_0_TRP_Msk;

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_I2C2_Init();
  MX_TIM11_Init();
  MX_IWDG_Init();
  MX_TIM7_Init();
  /* USER CODE BEGIN 2 */
  HAL_GPIO_WritePin(STATUS_LED_GREEN_GPIO_Port,STATUS_LED_GREEN_Pin, GPIO_PIN_SET);
  HAL_TIM_Base_Start_IT(&htim11);
  HAL_TIM_Base_Start_IT(&htim7);
  HAL_GPIO_WritePin(LAN_nRST_GPIO_Port,LAN_nRST_Pin, GPIO_PIN_SET);
  HAL_Delay(100);


  /* USER CODE END 2 */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  commandTemplateMutex = xSemaphoreCreateMutex();
  if (commandTemplateMutex == NULL)
  {
    Error_Handler();
  }

  /* USER CODE END RTOS_MUTEX */

  /* Create the semaphores(s) */
  /* definition and creation of InitDoneSemaphore */
  osSemaphoreStaticDef(InitDoneSemaphore, &InitDoneSemaphoreControlBlock);
  InitDoneSemaphoreHandle = osSemaphoreCreate(osSemaphore(InitDoneSemaphore), 1);

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  osMutexDef(take_measurement_mutex);
  take_measurement_mutex = osMutexCreate(osMutex(take_measurement_mutex));

  osMutexDef(isBusyWithSendingResponse_mutex);
  isBusyWithSendingResponse_mutex = osMutexCreate(osMutex(isBusyWithSendingResponse_mutex));
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  init_command_queue();
  if (commandQueueHandle == NULL)
  {
    Error_Handler();
  }
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* definition and creation of defaultTask */
  osThreadStaticDef(defaultTask, StartDefaultTask, osPriorityNormal, 0, 256, defaultTaskBuffer, &defaultTaskControlBlock);
  defaultTaskHandle = osThreadCreate(osThread(defaultTask), NULL);

  /* definition and creation of blinkTask */
  osThreadStaticDef(blinkTask, blinkTaskEntry, osPriorityBelowNormal, 0, 128, blinkTaskBuffer, &blinkTaskControlBlock);
  blinkTaskHandle = osThreadCreate(osThread(blinkTask), NULL);

  /* definition and creation of watchdogTask */
  osThreadStaticDef(watchdogTask, watchDogReset, osPriorityBelowNormal, 0, 128, watchdogTaskBuffer, &watchdogTaskControlBlock);
  watchdogTaskHandle = osThreadCreate(osThread(watchdogTask), NULL);

  /* definition and creation of TcpServerTask */
  osThreadStaticDef(TcpServerTask, TcpServerTaskStart, osPriorityNormal, 0, 256, TcpServerTaskBuffer, &TcpServerTaskControlBlock);
  TcpServerTaskHandle = osThreadCreate(osThread(TcpServerTask), NULL);

  /* definition and creation of UsbTask */
  osThreadStaticDef(UsbTask, UsbTaskStart, osPriorityNormal, 0, 256, UsbTaskBuffer, &UsbTaskControlBlock);
  UsbTaskHandle = osThreadCreate(osThread(UsbTask), NULL);

  /* definition and creation of CommParserTask */
  osThreadStaticDef(CommParserTask, CommParserTaskStart, osPriorityNormal, 0, 700, CommParserTaskBuffer, &CommParserTaskControlBlock);
  CommParserTaskHandle = osThreadCreate(osThread(CommParserTask), NULL);

  /* USER CODE BEGIN RTOS_THREADS */
  /* USER CODE END RTOS_THREADS */

  /* Start scheduler */
  osKernelStart();

  /* We should never get here as control is now taken by the scheduler */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_LSI|RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.LSIState = RCC_LSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 4;
  RCC_OscInitStruct.PLL.PLLN = 168;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 7;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief I2C2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C2_Init(void)
{

  /* USER CODE BEGIN I2C2_Init 0 */

  /* USER CODE END I2C2_Init 0 */

  /* USER CODE BEGIN I2C2_Init 1 */

  /* USER CODE END I2C2_Init 1 */
  hi2c2.Instance = I2C2;
  hi2c2.Init.ClockSpeed = 100000;
  hi2c2.Init.DutyCycle = I2C_DUTYCYCLE_2;
  hi2c2.Init.OwnAddress1 = 0;
  hi2c2.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c2.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c2.Init.OwnAddress2 = 0;
  hi2c2.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c2.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C2_Init 2 */

  /* USER CODE END I2C2_Init 2 */

}

/**
  * @brief IWDG Initialization Function
  * @param None
  * @retval None
  */
static void MX_IWDG_Init(void)
{

  /* USER CODE BEGIN IWDG_Init 0 */

  /* USER CODE END IWDG_Init 0 */

  /* USER CODE BEGIN IWDG_Init 1 */

  /* USER CODE END IWDG_Init 1 */
  hiwdg.Instance = IWDG;
  hiwdg.Init.Prescaler = IWDG_PRESCALER_128;
  hiwdg.Init.Reload = 2500;
  if (HAL_IWDG_Init(&hiwdg) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN IWDG_Init 2 */

  /* USER CODE END IWDG_Init 2 */

}

/**
  * @brief TIM7 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM7_Init(void)
{

  /* USER CODE BEGIN TIM7_Init 0 */

  /* USER CODE END TIM7_Init 0 */

  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM7_Init 1 */

  /* USER CODE END TIM7_Init 1 */
  htim7.Instance = TIM7;
  htim7.Init.Prescaler = 0;
  htim7.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim7.Init.Period = 168-1;
  htim7.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
  if (HAL_TIM_Base_Init(&htim7) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim7, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM7_Init 2 */

  /* USER CODE END TIM7_Init 2 */

}

/**
  * @brief TIM11 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM11_Init(void)
{

  /* USER CODE BEGIN TIM11_Init 0 */

  /* USER CODE END TIM11_Init 0 */

  /* USER CODE BEGIN TIM11_Init 1 */

  /* USER CODE END TIM11_Init 1 */
  htim11.Instance = TIM11;
  htim11.Init.Prescaler = 0;
  htim11.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim11.Init.Period = 420-1;
  htim11.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim11.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
  if (HAL_TIM_Base_Init(&htim11) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM11_Init 2 */

  /* USER CODE END TIM11_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  /* USER CODE BEGIN MX_GPIO_Init_1 */
  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOE_CLK_ENABLE();
  __HAL_RCC_GPIOF_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(EXT_I2C_nRST_GPIO_Port, EXT_I2C_nRST_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(EEPROM_WP_GPIO_Port, EEPROM_WP_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOD, LAN_nRST_Pin|STATUS_LED_GREEN_Pin|STATUS_LED_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(LIN_EN_GPIO_Port, LIN_EN_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : EXT_I2C_nRST_Pin */
  GPIO_InitStruct.Pin = EXT_I2C_nRST_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(EXT_I2C_nRST_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : EEPROM_WP_Pin */
  GPIO_InitStruct.Pin = EEPROM_WP_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(EEPROM_WP_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : LAN_nRST_Pin STATUS_LED_GREEN_Pin STATUS_LED_Pin */
  GPIO_InitStruct.Pin = LAN_nRST_Pin|STATUS_LED_GREEN_Pin|STATUS_LED_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

  /*Configure GPIO pin : LIN_EN_Pin */
  GPIO_InitStruct.Pin = LIN_EN_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LIN_EN_GPIO_Port, &GPIO_InitStruct);

  /* USER CODE BEGIN MX_GPIO_Init_2 */
  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName)
{
  (void)xTask;
  DebugTrap("FreeRTOS stack overflow", pcTaskName);
}

void vApplicationMallocFailedHook(void)
{
  DebugTrap("FreeRTOS malloc failed", pcTaskGetName(NULL));
}

/* USER CODE END 4 */

/* USER CODE BEGIN Header_StartDefaultTask */
/**
  * @brief  Function implementing the defaultTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void const * argument)
{
  /* init code for USB_DEVICE */
  MX_USB_DEVICE_Init();

  /* init code for LWIP */
  MX_LWIP_Init();
  /* USER CODE BEGIN 5 */
  HAL_Delay(1000);
  xSemaphoreGive(InitDoneSemaphoreHandle);
  
  //Try to do it using bit banging method

  for(;;)
  {  
    osDelay(1);
  }
  /* USER CODE END 5 */
}

/* USER CODE BEGIN Header_blinkTaskEntry */
/**
* @brief Function implementing the blinkTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_blinkTaskEntry */
void blinkTaskEntry(void const * argument)
{
  /* USER CODE BEGIN blinkTaskEntry */
  if (xSemaphoreTake(InitDoneSemaphoreHandle, portMAX_DELAY) != pdTRUE)
  {
    Error_Handler();
  }

  //return it
  xSemaphoreGive(InitDoneSemaphoreHandle);

  /* Infinite loop */
  HAL_GPIO_WritePin(STATUS_LED_GREEN_GPIO_Port, STATUS_LED_Pin, GPIO_PIN_SET);
  for(;;)
  {
    HAL_GPIO_TogglePin(STATUS_LED_GREEN_GPIO_Port, STATUS_LED_GREEN_Pin);
    osDelay(500);
  }
  /* USER CODE END blinkTaskEntry */
}

/* USER CODE BEGIN Header_watchDogReset */
/**
* @brief Function implementing the watchdogTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_watchDogReset */
void watchDogReset(void const * argument)
{
  /* USER CODE BEGIN watchDogReset */
  /* Infinite loop */
  for(;;)
  {
    HAL_IWDG_Refresh(&hiwdg);
    osDelay(50);
  }
  /* USER CODE END watchDogReset */
}

/* USER CODE BEGIN Header_TcpServerTaskStart */
/**
* @brief Function implementing the TcpServerTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_TcpServerTaskStart */
void TcpServerTaskStart(void const * argument)
{
  /* USER CODE BEGIN TcpServerTaskStart */
    if (xSemaphoreTake(InitDoneSemaphoreHandle, portMAX_DELAY) != pdTRUE)
    {
      Error_Handler();
    }

    //return it
    xSemaphoreGive(InitDoneSemaphoreHandle);

    tcp_thread(NULL);  
  /* USER CODE END TcpServerTaskStart */
}

/* USER CODE BEGIN Header_UsbTaskStart */
/**
* @brief Function implementing the UsbTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_UsbTaskStart */
void UsbTaskStart(void const * argument)
{
  /* USER CODE BEGIN UsbTaskStart */
  if (xSemaphoreTake(InitDoneSemaphoreHandle, portMAX_DELAY) != pdTRUE)
  {
    Error_Handler();
  }

  //return it
  xSemaphoreGive(InitDoneSemaphoreHandle);
  /* Infinite loop */
  for(;;)
  {
    parse_usb_message();
    osDelay(1);
  }
  /* USER CODE END UsbTaskStart */
}

/* USER CODE BEGIN Header_CommParserTaskStart */
/**
* @brief Function implementing the CommParserTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_CommParserTaskStart */
void CommParserTaskStart(void const * argument)
{
  /* USER CODE BEGIN CommParserTaskStart */
  /* Infinite loop */
  for(;;)
  {

    //Read message queue and handle only valid dequeued commands.
    commandTemplate current_command;

    if(xQueueReceive(commandQueueHandle, &current_command, 0) == pdPASS){
      runCommand(&current_command);

      //print back response based on command sender
      if (current_command.source == USB_HANDLER) {
        printf("%s", current_command.response);
      }

      else if (current_command.source == RS232_HANDLER) {
        //send response via RS232
      }

      else if (current_command.source == TCP_THREAD) {
        //send response via TCP
        size_t total_len = strlen(current_command.response);
        size_t bytes_written = 0;

        while (bytes_written < total_len)
        {
          size_t chunk_written = 0;
          err_t write_err = netconn_write_partly(current_command.TCP_client_netconn,
                                                  current_command.response + bytes_written,
                                                  total_len - bytes_written,
                                                  NETCONN_COPY,
                                                  &chunk_written);

          bytes_written += chunk_written;

          if (write_err == ERR_OK)
          {
            continue;
          }

          if (write_err == ERR_WOULDBLOCK && chunk_written == 0)
          {
            osDelay(1);
            continue;
        }

          break;
        }

      }
    }
    osDelay(1);
  }
  /* USER CODE END CommParserTaskStart */
}

/**
  * @brief  Period elapsed callback in non blocking mode
  * @note   This function is called  when TIM14 interrupt took place, inside
  * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
  * a global variable "uwTick" used as application time base.
  * @param  htim : TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  /* USER CODE BEGIN Callback 0 */
  if (htim->Instance == TIM11) {
    _us_tick++;
  }

  if (htim->Instance == TIM7) {
    TIM_7_tick_1_us++;
  }

  /* USER CODE END Callback 0 */
  if (htim->Instance == TIM14)
  {
    HAL_IncTick();
  }
  /* USER CODE BEGIN Callback 1 */

  /* USER CODE END Callback 1 */
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
