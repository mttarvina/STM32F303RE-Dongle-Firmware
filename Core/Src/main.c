/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
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
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "gpio.h"
#include "stm32f3xx_ll_gpio.h"
#include "stm32f3xx_ll_utils.h"
#include "usart.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "tarvs_usart2.h"
#include "tarvs_cmd.h"
#include <stdint.h>
#include <stdio.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
 * @brief  The application entry point.
 * @retval int
 */
int main(void) {

  /* USER CODE BEGIN 1 */
  char usart2_tx_msg[USART2_SERIAL_BUF_SIZE] = {0};
  char usart2_rx_msg[USART2_SERIAL_BUF_SIZE] = {0};
  uint16_t usart2_tx_bytes = 0;
  uint16_t usart2_rx_bytes = 0;

  CMD_StructDef serial_cmd;
  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick.
   */
  LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_SYSCFG);
  LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_PWR);

  /* System interrupt init*/
  NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_4);

  /* SysTick_IRQn interrupt configuration */
  NVIC_SetPriority(SysTick_IRQn,
                   NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 15, 0));

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_USART2_UART_Init();

  /* USER CODE BEGIN 2 */
  LL_GPIO_InitTypeDef user_led2 = {.Pin = LL_GPIO_PIN_5,
                                   .Mode = LL_GPIO_MODE_OUTPUT,
                                   .OutputType = LL_GPIO_OUTPUT_PUSHPULL,
                                   .Speed = LL_GPIO_SPEED_FREQ_MEDIUM};

  LL_GPIO_Init(GPIOA, &user_led2);
  USART2_ResetRXBuffer();
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1) {
    if (USART2_IsDataAvailable()) {
      usart2_rx_bytes = USART2_ReadMessage(usart2_rx_msg, USART2_SERIAL_BUF_SIZE);

      #ifdef ENABLE_DEBUG
        usart2_tx_bytes = snprintf(usart2_tx_msg, USART2_SERIAL_BUF_SIZE, "String=%s\r\n", usart2_rx_msg);
        USART2_SendMessage(usart2_tx_msg, usart2_tx_bytes);
      #endif

      bool valid_cmd = CMD_Parse(usart2_rx_msg, &serial_cmd, usart2_rx_bytes);
      if (valid_cmd) {
        uint16_t argA = CMD_ArgToInt(serial_cmd.argA);
        uint16_t argB = CMD_ArgToInt(serial_cmd.argB);

        #ifdef ENABLE_DEBUG
          usart2_tx_bytes = snprintf(usart2_tx_msg, USART2_SERIAL_BUF_SIZE, ":CMD=%d;SUBJ=%d;PARAM=%d;ArgA=%d;ArgB=%d\r\n", serial_cmd.action, serial_cmd.subject, serial_cmd.param, argA, argB);
          USART2_SendMessage(usart2_tx_msg, usart2_tx_bytes);
        #endif

      }
      else {
        usart2_tx_bytes = snprintf(usart2_tx_msg, USART2_SERIAL_BUF_SIZE, ":CMD=?\r\n");
        USART2_SendMessage(usart2_tx_msg, usart2_tx_bytes);
      }
    }
    LL_mDelay(10); // Important: Prevents incomplete capture of longer commands
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
 * @brief System Clock Configuration
 * @retval None
 */
void SystemClock_Config(void) {
  LL_FLASH_SetLatency(LL_FLASH_LATENCY_2);
  while (LL_FLASH_GetLatency() != LL_FLASH_LATENCY_2) {
  }
  LL_RCC_HSI_Enable();

  /* Wait till HSI is ready */
  while (LL_RCC_HSI_IsReady() != 1) {
  }
  LL_RCC_HSI_SetCalibTrimming(16);
  LL_RCC_PLL_ConfigDomain_SYS(LL_RCC_PLLSOURCE_HSI, LL_RCC_PLL_MUL_9,
                              LL_RCC_PREDIV_DIV_1);
  LL_RCC_PLL_Enable();

  /* Wait till PLL is ready */
  while (LL_RCC_PLL_IsReady() != 1) {
  }
  LL_RCC_SetAHBPrescaler(LL_RCC_SYSCLK_DIV_1);
  LL_RCC_SetAPB1Prescaler(LL_RCC_APB1_DIV_2);
  LL_RCC_SetAPB2Prescaler(LL_RCC_APB2_DIV_1);
  LL_RCC_SetSysClkSource(LL_RCC_SYS_CLKSOURCE_PLL);

  /* Wait till System clock is ready */
  while (LL_RCC_GetSysClkSource() != LL_RCC_SYS_CLKSOURCE_STATUS_PLL) {
  }
  LL_Init1msTick(72000000);
  LL_SetSystemCoreClock(72000000);
  LL_RCC_SetUSARTClockSource(LL_RCC_USART2_CLKSOURCE_SYSCLK);
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
 * @brief  This function is executed in case of error occurrence.
 * @retval None
 */
void Error_Handler(void) {
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1) {
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
void assert_failed(uint8_t *file, uint32_t line) {
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line
     number, ex: printf("Wrong parameters value: file %s on line %d\r\n", file,
     line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
