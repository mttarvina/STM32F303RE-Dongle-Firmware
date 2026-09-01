#include "app.h"
#include "stm32f3xx_ll_gpio.h"
#include "stm32f3xx_ll_utils.h"
#include "tarvs_buffer.h"
#include "tarvs_cmd.h"
#include "tarvs_gpio.h"
#include "tarvs_spi.h"
#include "tarvs_usart2.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

// --- Application Variables
static char usart2_tx_msg[USART2_SERIAL_BUF_SIZE] = {0};
static char usart2_rx_msg[USART2_SERIAL_BUF_SIZE] = {0};
static uint16_t usart2_tx_bytes = 0;
static uint16_t usart2_rx_bytes = 0;

static CMD_StructDef serial_cmd;

/**
 * @brief   Set the initial application state, all GPIOs should be configured as Analog input
 *
 * @param   None
 * @retval  None
 */
void APP_Init(void) {
  // Re-initialize SPI CSN pin as analog input
  // CubeMX will initialize this to alternate due to TIM4_CH1 (PWM Generation) config
  LL_GPIO_InitTypeDef spi_csn = {
      .Pin = LL_GPIO_PIN_6,
      .Mode = LL_GPIO_MODE_ANALOG,
      .Pull = LL_GPIO_PULL_NO,
  };
  LL_GPIO_Init(GPIOB, &spi_csn);

  // Re-initialize SPI pins as analog input
  // CubeMX will initialize this to alternate function due to SPI1 config
  LL_GPIO_InitTypeDef spi_pins = {
      .Pin = LL_GPIO_PIN_5 | LL_GPIO_PIN_6 | LL_GPIO_PIN_7,
      .Mode = LL_GPIO_MODE_ANALOG,
      .Pull = LL_GPIO_PULL_NO,
  };
  LL_GPIO_Init(GPIOA, &spi_pins);
  USART2_ResetRXBuffer();
}

/**
 * @brief   Send a formatted error message containing the 32-bit error code of a specific module
 *
 * @param   module : Peripheral module. Refer to app.h (MODULE_GPIO, MODULE_SPI, etc...)
 * @retval  None
 */
void APP_SendErrorMessage(uint8_t module) {
  uint32_t err;
  switch (module) {
  case MODULE_CMD:
    err = CMD_GetErrorState();
    usart2_tx_bytes = snprintf(usart2_tx_msg, USART2_SERIAL_BUF_SIZE, "CMD=?;ERR=%lu\r\n", err);
    USART2_SendMessage((const uint8_t *)usart2_tx_msg, usart2_tx_bytes);
    CMD_ResetErrorState();
    break;
  case MODULE_GPIO:
    err = GPIO_GetErrorState();
    usart2_tx_bytes = snprintf(usart2_tx_msg, USART2_SERIAL_BUF_SIZE, "GPIO=?;ERR=%lu\r\n", err);
    USART2_SendMessage((const uint8_t *)usart2_tx_msg, usart2_tx_bytes);
    GPIO_ResetErrorState();
    break;
  case MODULE_SPI:
    err = SPI1_GetErrorState();
    usart2_tx_bytes = snprintf(usart2_tx_msg, USART2_SERIAL_BUF_SIZE, "SPI=?;ERR=%lu\r\n", err);
    USART2_SendMessage((const uint8_t *)usart2_tx_msg, usart2_tx_bytes);
    SPI1_ResetErrorState();
    break;
  default:
    break;
  }
}

/**
 * @brief   Helper function that checks if the returned value of function is SUCCESS or ERROR.
 *          Calls APP_SendErrorMessage if the function returns ERROR
 *
 * @param   ret : Return value of a function of type - ErrorStatus
 * @param   module : Peripheral module. Refer to app.h (MODULE_GPIO, MODULE_SPI, etc...)
 * @retval
 */
void APP_ErrorCheck(ErrorStatus ret, uint8_t module) {
  if (ret == ERROR) {
    APP_SendErrorMessage(module);
  }
}

/**
 * @brief   Debug function used for early testing of specific features
 *
 * @param   None
 * @retval  None
 */
void APP_Test(void) {
  // SPI1_SetSampleRate(800);
  // SPI1_CSN_Config(80, 0);
  // SPI1_Init();
  // BUFFER_ResetTXData();
  // SPI1_AppendTXData(0x1111);
  // SPI1_AppendTXData(0x2222);
  // SPI1_AppendTXData(0x3333);
  // SPI1_AppendTXData(0x4444);
  // SPI1_AppendTXData(0x5555);
  // SPI1_AppendTXData(0x6666);
  // SPI1_DMABurstTransaction(6, true);

  while (1) {
    SPI1_SetSampleRate(300);
    SPI1_CSN_Config(80, 0);
    SPI1_Init();
    BUFFER_ResetTXData();
    BUFFER_AppendTXData(0x1111);
    BUFFER_AppendTXData(0x2222);
    SPI1_DMABurstTransaction(2, true);
    usart2_tx_bytes = snprintf(usart2_tx_msg, USART2_SERIAL_BUF_SIZE, "A\r\n");
    USART2_SendMessage((const uint8_t *)usart2_tx_msg, usart2_tx_bytes);
    LL_mDelay(1000);

    SPI1_SetSampleRate(200);
    SPI1_CSN_Config(80, 0);
    SPI1_Init();
    BUFFER_ResetTXData();
    BUFFER_AppendTXData(0xA0A0);
    BUFFER_AppendTXData(0xFDCB);
    SPI1_DMABurstTransaction(2, true);
    usart2_tx_bytes = snprintf(usart2_tx_msg, USART2_SERIAL_BUF_SIZE, "B\r\n");
    USART2_SendMessage((const uint8_t *)usart2_tx_msg, usart2_tx_bytes);
    LL_mDelay(1000);
  }
}

void APP_Main(void) {
  APP_Init();

  // Start the test routine
  // APP_Test();

  while (1) {
    if (USART2_IsCommandAvailable()) {
      usart2_rx_bytes = USART2_ReadMessage(usart2_rx_msg, USART2_SERIAL_BUF_SIZE);
      // #ifdef ENABLE_DEBUG
      //       usart2_tx_bytes = snprintf(usart2_tx_msg, USART2_SERIAL_BUF_SIZE, "CMD=%s\r\n", usart2_rx_msg);
      //       USART2_SendMessage((const uint8_t*)usart2_tx_msg, usart2_tx_bytes);
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
            USART2_SendMessage((const uint8_t *)usart2_tx_msg, usart2_tx_bytes);
          } else if (serial_cmd.subject == CMD_BUFFER) {
            // Format: [READ, BUFFER, '0', <frame_size[4]>, '0', '0', '0', '0', '!'] --> RX Buffer
            // Format: [READ, BUFFER, '1', <frame_size[4]>, '0', '0', '0', '0', '!'] --> TX Buffer
            uint8_t buf_type = 0;
            uint16_t frame_size = 0;
            APP_ErrorCheck(CMD_CharToInt(serial_cmd.param, &buf_type), MODULE_CMD);
            APP_ErrorCheck(CMD_ArgToInt(serial_cmd.argA, &frame_size), MODULE_CMD);
            if (buf_type == 0) {
              APP_ErrorCheck(BUFFER_TransmitRXData(frame_size), MODULE_BUFFER);
            } else if (buf_type == 1) {
              APP_ErrorCheck(BUFFER_TransmitTXData(frame_size), MODULE_BUFFER);
            }
          } else {
            CMD_RaiseInvalidSubjectError();
            APP_SendErrorMessage(MODULE_CMD);
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
          } else if (serial_cmd.subject == CMD_SPI) {
            // Format: [WRITE, SPI, '0', <frame_size[4]>, <increment_tx[4]>, '!']
            uint16_t frame_size = 0;
            uint16_t increment_tx = 0;
            APP_ErrorCheck(CMD_ArgToInt(serial_cmd.argA, &frame_size), MODULE_CMD);
            APP_ErrorCheck(CMD_ArgToInt(serial_cmd.argB, &increment_tx), MODULE_CMD);
            if (increment_tx == 1) {
              APP_ErrorCheck(SPI1_DMABurstTransaction(frame_size, true), MODULE_SPI);
            } else if (increment_tx == 0) {
              APP_ErrorCheck(SPI1_DMABurstTransaction(frame_size, false), MODULE_SPI);
            }
          } else {
            CMD_RaiseInvalidSubjectError();
            APP_SendErrorMessage(MODULE_CMD);
          }
          break;
        case CMD_APPEND:
          if (serial_cmd.subject == CMD_BUFFER) {
            // Format: [APPEND, BUFFER, '0', <data[4]>, '0', '0', '0', '0', '!'] --> RX Buffer
            // Format: [APPEND, BUFFER, '1', <data[4]>, '0', '0', '0', '0', '!'] --> TX Buffer
            uint8_t buf_type = 0;
            uint16_t tx_data = 0;
            APP_ErrorCheck(CMD_CharToInt(serial_cmd.param, &buf_type), MODULE_CMD);
            APP_ErrorCheck(CMD_ArgToInt(serial_cmd.argA, &tx_data), MODULE_SPI);
            if (buf_type == 0) {
              APP_ErrorCheck(BUFFER_AppendRXData(tx_data), MODULE_BUFFER);
            } else if (buf_type == 1) {
              APP_ErrorCheck(BUFFER_AppendTXData(tx_data), MODULE_BUFFER);
            }
          } else {
            CMD_RaiseInvalidSubjectError();
            APP_SendErrorMessage(MODULE_CMD);
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
          } else {
            CMD_RaiseInvalidSubjectError();
            APP_SendErrorMessage(MODULE_CMD);
          }
          break;
        case CMD_START:
          CMD_RaiseNotSupportedError();
          APP_SendErrorMessage(MODULE_CMD);
          break;
        case CMD_STOP:
          CMD_RaiseNotSupportedError();
          APP_SendErrorMessage(MODULE_CMD);
          break;
        case CMD_CAPTURE:
          CMD_RaiseNotSupportedError();
          APP_SendErrorMessage(MODULE_CMD);
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
          } else if (serial_cmd.subject == CMD_SPI) {
            // Format: [CONFIG, SPI, '0', <sample_rate_khz[4]>, '0', '0', '0', '0', '!'] --> Configure SPI Sample Rate
            // Format: [CONFIG, SPI, '1', <data_width>, <first_bit>, <prescaler>, <clock_polarity>, <clock_phase>,
            // <transmit_delay[3]>, '!'] --> Configure SPI
            uint8_t param = 0;
            APP_ErrorCheck(CMD_CharToInt(serial_cmd.param, &param), MODULE_CMD);
            if (param == 0) { // Sample Rate
              uint16_t argA_int = 0;
              APP_ErrorCheck(CMD_ArgToInt(serial_cmd.argA, &argA_int), MODULE_CMD);
              APP_ErrorCheck(SPI1_SetSampleRate(argA_int), MODULE_SPI);
            } else if (param == 1) {
              uint8_t data_width, first_bit, prescale, clk_polarity, clk_phase = 0;
              uint16_t transmit_delay = 0;
              APP_ErrorCheck(CMD_CharToInt(serial_cmd.argA[0], &data_width), MODULE_CMD);
              APP_ErrorCheck(CMD_CharToInt(serial_cmd.argA[1], &first_bit), MODULE_CMD);
              APP_ErrorCheck(CMD_CharToInt(serial_cmd.argA[2], &prescale), MODULE_CMD);
              APP_ErrorCheck(CMD_CharToInt(serial_cmd.argA[3], &clk_polarity), MODULE_CMD);
              APP_ErrorCheck(CMD_CharToInt(serial_cmd.argB[0], &clk_phase), MODULE_CMD);
              serial_cmd.argB[0] = '0';
              APP_ErrorCheck(CMD_ArgToInt(serial_cmd.argB, &transmit_delay), MODULE_CMD);
              APP_ErrorCheck(SPI1_Config(data_width, first_bit, prescale, clk_polarity, clk_phase, transmit_delay),
                             MODULE_CMD);
            }
          } else if (serial_cmd.subject == CMD_SPI_CSN) {
            // Format: [CONFIG, SPI_CSN, '0', <pulse_width[4]>, '0', '0', '0', <polarity>, '!'] --> Configure SPI CSN
            uint8_t polarity = 0;
            uint16_t pulse_width = 0;
            APP_ErrorCheck(CMD_CharToInt(serial_cmd.argB[3], &polarity), MODULE_CMD);
            APP_ErrorCheck(CMD_ArgToInt(serial_cmd.argA, &pulse_width), MODULE_CMD);
            APP_ErrorCheck(SPI1_CSN_Config(pulse_width, polarity), MODULE_SPI);
          } else {
            CMD_RaiseInvalidSubjectError();
            APP_SendErrorMessage(MODULE_CMD);
          }
          break;
        case CMD_INIT:
          if (serial_cmd.subject == CMD_SPI) {
            // Format: [INIT, SPI, '0', '0', '0', '0', '0', '0', '0', '0', '0', '!']
            SPI1_Init();
          } else {
            CMD_RaiseInvalidSubjectError();
            APP_SendErrorMessage(MODULE_CMD);
          }
          break;
        case CMD_RESET:
          if (serial_cmd.subject == CMD_BUFFER) {
            // Format: [RESET, BUFFER, '0', '0', '0', '0', '0', '0', '0', '0', '0', '!'] --> RX Buffer
            // Format: [RESET, BUFFER, '1', '0', '0', '0', '0', '0', '0', '0', '0', '!'] --> TX Buffer
            uint8_t buf_type = 0;
            APP_ErrorCheck(CMD_CharToInt(serial_cmd.param, &buf_type), MODULE_CMD);
            if (buf_type == 0) {
              BUFFER_ResetRXData();
            } else if (buf_type == 1) {
              BUFFER_ResetTXData();
            }
          } else {
            CMD_RaiseInvalidSubjectError();
            APP_SendErrorMessage(MODULE_CMD);
          }
          break;
        default:
          CMD_RaiseInvalidActionError();
          APP_SendErrorMessage(MODULE_CMD);
          break;
        }
      } else {
        CMD_RaiseUnknownError();
        APP_SendErrorMessage(MODULE_CMD);
      }
    }
  }
}
