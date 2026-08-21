#include "app.h"
#include "stm32f3xx_ll_gpio.h"
#include "tarvs_cmd.h"
#include "tarvs_gpio.h"
#include "tarvs_usart2.h"
#include <stdint.h>
#include <stdio.h>

// Global variables for the main state machine loop
static LL_GPIO_InitTypeDef user_led = {0};
static char usart2_tx_msg[USART2_SERIAL_BUF_SIZE] = {0};
static char usart2_rx_msg[USART2_SERIAL_BUF_SIZE] = {0};
static uint16_t usart2_tx_bytes = 0;
static uint16_t usart2_rx_bytes = 0;

static CMD_StructDef serial_cmd;

/**
 * @brief
 *
 * @param
 * @retval
 */
void APP_Init(void) {
  user_led.Pin = LL_GPIO_PIN_5;
  user_led.Mode = LL_GPIO_MODE_OUTPUT;
  user_led.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  user_led.Speed = LL_GPIO_SPEED_FREQ_HIGH;
  LL_GPIO_Init(GPIOA, &user_led);
  USART2_ResetRXBuffer();
}

void APP_SendErrorMessage(uint8_t module) {
  uint32_t err;
  switch (module) {
  case MODULE_CMD:
    err = CMD_GetErrorState();
    usart2_tx_bytes = snprintf(usart2_tx_msg, USART2_SERIAL_BUF_SIZE, "CMD=?;ERR=%lu\r\n", err);
    USART2_SendMessage(usart2_tx_msg, usart2_tx_bytes);
    CMD_ResetErrorState();
    break;
  case MODULE_GPIO:
    err = GPIO_GetErrorState();
    usart2_tx_bytes = snprintf(usart2_tx_msg, USART2_SERIAL_BUF_SIZE, "GPIO=?;ERR=%lu\r\n", err);
    USART2_SendMessage(usart2_tx_msg, usart2_tx_bytes);
    GPIO_ResetErrorState();
    break;
  default:
    break;
  }
}

void APP_ErrorCheck(ErrorStatus ret, uint8_t module) {
  if (ret == ERROR) {
    APP_SendErrorMessage(module);
  }
}

void APP_Main(void) {
  APP_Init();

  while (1) {
    if (USART2_IsCommandAvailable()) {
      usart2_rx_bytes = USART2_ReadMessage(usart2_rx_msg, USART2_SERIAL_BUF_SIZE);
      // #ifdef ENABLE_DEBUG
      //       usart2_tx_bytes = snprintf(usart2_tx_msg, USART2_SERIAL_BUF_SIZE, "String=%s\r\n", usart2_rx_msg);
      //       USART2_SendMessage(usart2_tx_msg, usart2_tx_bytes);
      // #endif
      if (CMD_Parse(usart2_rx_msg, &serial_cmd, usart2_rx_bytes) == SUCCESS) {
        switch (serial_cmd.action) {
        case CMD_READ:
          if (serial_cmd.subject == CMD_GPIO) {
            // Format: [READ, GPIO, <port>, <pin_mask[4]>, '0', '0', '0', '0', '!']
            // Example: READ from GPIO, port GPIO, pin #6 --> "00000400000!\r\n"
            uint8_t port;
            uint16_t pins, pin_states;
            APP_ErrorCheck(CMD_CharToInt(serial_cmd.param, &port), MODULE_CMD);
            APP_ErrorCheck(CMD_ArgToInt(serial_cmd.argA, &pins), MODULE_CMD);
            APP_ErrorCheck(GPIO_Read(port, pins, &pin_states), MODULE_GPIO);
            usart2_tx_bytes = snprintf(usart2_tx_msg, USART2_SERIAL_BUF_SIZE, "%d\r\n", pin_states);
            USART2_SendMessage(usart2_tx_msg, usart2_tx_bytes);
          }
          break;
        case CMD_WRITE:
          if (serial_cmd.subject == CMD_GPIO) {
            // Format: [WRITE, GPIO, <port>, <pin_mask[4]>, <level>, '0', '0', '0', '!']
            // Example: WRITE to GPIO, port GPIOA, pin #6, to HIGH --> "10000401000!\r\n"
            uint8_t port, level;
            uint16_t pins;
            APP_ErrorCheck(CMD_CharToInt(serial_cmd.param, &port), MODULE_CMD);
            APP_ErrorCheck(CMD_ArgToInt(serial_cmd.argA, &pins), MODULE_CMD);
            APP_ErrorCheck(CMD_CharToInt(serial_cmd.argB[0], &level), MODULE_CMD);
            APP_ErrorCheck(GPIO_Write(port, pins, level), MODULE_GPIO);
          }
          break;
        case CMD_TOGGLE:
          if (serial_cmd.subject == CMD_GPIO) {
            // Format: [TOGGLE, GPIO, <port>, <pin_mask[4]>, '0', '0', '0', '0', '!']
            // Example: Toggle GPIO, port GPIOA, pin #6 --> "20000400000!\r\n"
            uint8_t port;
            uint16_t pins;
            APP_ErrorCheck(CMD_CharToInt(serial_cmd.param, &port), MODULE_CMD);
            APP_ErrorCheck(CMD_ArgToInt(serial_cmd.argA, &pins), MODULE_CMD);
            APP_ErrorCheck(GPIO_Toggle(port, pins), MODULE_GPIO);
          }
          break;
        case CMD_START:
          break;
        case CMD_STOP:
          break;
        case CMD_CAPTURE:
          break;
        case CMD_CONFIG:
          if (serial_cmd.subject == CMD_GPIO) {
            // Format: [CONFIG, GPIO, <port>, <pin_mask>, <mode>, <speed>, <output_type>, <pull>, '!']
            // Example: Configure GPIO, port GPIOA, pin #6, as OUTPUT, FAST, PUSH-PULL, PULL-NO --> "60000401200!\r\n"
            uint8_t port, mode, speed, output_mode, pull_mode;
            uint16_t pins;
            APP_ErrorCheck(CMD_CharToInt(serial_cmd.param, &port), MODULE_CMD);
            APP_ErrorCheck(CMD_ArgToInt(serial_cmd.argA, &pins), MODULE_CMD);
            APP_ErrorCheck(CMD_CharToInt(serial_cmd.argB[0], &mode), MODULE_CMD);
            APP_ErrorCheck(CMD_CharToInt(serial_cmd.argB[1], &speed), MODULE_CMD);
            APP_ErrorCheck(CMD_CharToInt(serial_cmd.argB[2], &output_mode), MODULE_CMD);
            APP_ErrorCheck(CMD_CharToInt(serial_cmd.argB[3], &pull_mode), MODULE_CMD);
            APP_ErrorCheck(GPIO_Config(port, pins, mode, speed, output_mode, pull_mode), MODULE_GPIO);
          }
          break;
        default:
          APP_SendErrorMessage(MODULE_CMD);
          break;
        }
      } else {
        APP_SendErrorMessage(MODULE_CMD);
      }
    }
  }
}
