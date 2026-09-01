#include "spi.h"
#include "stm32f303xe.h"
#include "stm32f3xx.h"
#include "stm32f3xx_ll_dma.h"
#include "stm32f3xx_ll_gpio.h"
#include "stm32f3xx_ll_spi.h"
#include "stm32f3xx_ll_tim.h"
#include "tarvs_buffer.h"
#include "tarvs_spi.h"
#include "tim.h"
#include <stdbool.h>
#include <stdint.h>
#include <sys/types.h>

static uint32_t _spi1_error_state = 0;
static SPI1_ConfigDef _spi1_cfg = {.data_width = LL_SPI_DATAWIDTH_16BIT,
                                   .first_bit = LL_SPI_MSB_FIRST,
                                   .clk_prescale = LL_SPI_BAUDRATEPRESCALER_DIV4,
                                   .clk_polarity = LL_SPI_POLARITY_LOW,
                                   .clk_phase = LL_SPI_PHASE_1EDGE,
                                   .sample_period = 7199, // default = 100us
                                   .transmit_delay = 1,   // default = 13.8889ns
                                   .csn_pulse_width = 79, // default = 1.1us
                                   .csn_polarity = LL_TIM_OCPOLARITY_LOW};

static void _ConfigCSNtoGPIOFloat(void) {
  LL_GPIO_InitTypeDef csn_pin = {
      .Pin = LL_GPIO_PIN_6,
      .Mode = LL_GPIO_MODE_ANALOG,
      .Pull = LL_GPIO_PULL_NO,
  };
  LL_GPIO_Init(GPIOB, &csn_pin);
}

static void _ConfigCSNtoPWMOutput(void) {
  LL_GPIO_InitTypeDef csn_pin = {
      .Pin = LL_GPIO_PIN_6,
      .Mode = LL_GPIO_MODE_ALTERNATE,
      .Speed = LL_GPIO_SPEED_FREQ_HIGH,
      .OutputType = LL_GPIO_OUTPUT_PUSHPULL,
      .Pull = LL_GPIO_PULL_NO,
      .Alternate = LL_GPIO_AF_2,
  };
  LL_GPIO_Init(GPIOB, &csn_pin);
}

static uint16_t _FreqToPeriodCount(uint16_t freq_khz) {
  uint16_t tim4_period = (72000 / freq_khz) - 1;
  return tim4_period;
}

static void _TimerStart(void) {
  LL_TIM_DisableCounter(TIM4);
  LL_TIM_SetCounter(TIM4, 0); // Reset timer count to zero
  LL_TIM_CC_EnableChannel(TIM4, LL_TIM_CHANNEL_CH1);
  LL_TIM_CC_EnableChannel(TIM4, LL_TIM_CHANNEL_CH3);
  LL_TIM_EnableAllOutputs(TIM4);
  LL_TIM_EnableCounter(TIM4);
}

static void _TimerStop(void) {
  LL_TIM_DisableCounter(TIM4);
  LL_TIM_DisableAllOutputs(TIM4);
  LL_TIM_CC_DisableChannel(TIM4, LL_TIM_CHANNEL_CH1);
  LL_TIM_CC_DisableChannel(TIM4, LL_TIM_CHANNEL_CH3);
  LL_TIM_SetCounter(TIM4, 0); // Reset timer count to zero
}

static void _ClearSPIDMAFlags(void) {
  LL_DMA_ClearFlag_GI2(DMA1);
  LL_DMA_ClearFlag_TC2(DMA1);
  LL_DMA_ClearFlag_HT2(DMA1);
  LL_DMA_ClearFlag_TE2(DMA1);
  LL_DMA_ClearFlag_GI5(DMA1);
  LL_DMA_ClearFlag_TC5(DMA1);
  LL_DMA_ClearFlag_HT5(DMA1);
  LL_DMA_ClearFlag_TE5(DMA1);
  LL_TIM_ClearFlag_CC3(TIM4);
  LL_SPI_ClearFlag_OVR(SPI1);
}

uint32_t SPI1_GetErrorState(void) { return _spi1_error_state; }

void SPI1_ResetErrorState(void) { _spi1_error_state = 0; }

ErrorStatus SPI1_SetSampleRate(uint16_t sample_rate_khz) {
  // check for invalid value
  if ((sample_rate_khz < SPI1_MIN_SAMPLE_RATE) || (sample_rate_khz > SPI1_MAX_SAMPLE_RATE)) {
    _spi1_error_state |= SPI1_ERROR_INVALID_SAMPLE_PERIOD;
    return ERROR;
  }
  _spi1_cfg.sample_period = _FreqToPeriodCount(sample_rate_khz);
  return SUCCESS;
}

ErrorStatus SPI1_Config(uint8_t data_width, uint8_t first_bit, uint8_t clk_prescale, uint8_t clk_polarity,
                        uint8_t clk_phase, uint16_t transmit_delay) {
  if (data_width < 3 || data_width > 15) {
    _spi1_error_state |= SPI1_ERROR_INVALID_DATA_WIDTH;
    return ERROR;
  }

  if (first_bit > 1) {
    _spi1_error_state |= SPI1_ERROR_INVALID_FIRST_BIT;
    return ERROR;
  }

  if (clk_prescale > 6) {
    _spi1_error_state |= SPI1_ERROR_INVALID_PRESCALER;
    return ERROR;
  }

  if (clk_polarity > 1) {
    _spi1_error_state |= SPI1_ERROR_INVALID_CLK_POLARITY;
    return ERROR;
  }

  if (clk_phase < 0 || clk_phase > 2) {
    _spi1_error_state |= SPI1_ERROR_INVALID_CLK_PHASE;
    return ERROR;
  }

  if (transmit_delay > 0xFFF) {
    _spi1_error_state |= SPI1_ERROR_INVALID_TRANSMIT_DELAY;
    return ERROR;
  }

  switch (data_width) {
  case 3:
    _spi1_cfg.data_width = LL_SPI_DATAWIDTH_4BIT;
    break;
  case 4:
    _spi1_cfg.data_width = LL_SPI_DATAWIDTH_5BIT;
    break;
  case 5:
    _spi1_cfg.data_width = LL_SPI_DATAWIDTH_6BIT;
    break;
  case 6:
    _spi1_cfg.data_width = LL_SPI_DATAWIDTH_7BIT;
    break;
  case 7:
    _spi1_cfg.data_width = LL_SPI_DATAWIDTH_8BIT;
    break;
  case 8:
    _spi1_cfg.data_width = LL_SPI_DATAWIDTH_9BIT;
    break;
  case 9:
    _spi1_cfg.data_width = LL_SPI_DATAWIDTH_10BIT;
    break;
  case 10:
    _spi1_cfg.data_width = LL_SPI_DATAWIDTH_11BIT;
    break;
  case 11:
    _spi1_cfg.data_width = LL_SPI_DATAWIDTH_12BIT;
    break;
  case 12:
    _spi1_cfg.data_width = LL_SPI_DATAWIDTH_13BIT;
    break;
  case 13:
    _spi1_cfg.data_width = LL_SPI_DATAWIDTH_14BIT;
    break;
  case 14:
    _spi1_cfg.data_width = LL_SPI_DATAWIDTH_15BIT;
    break;
  case 15:
    _spi1_cfg.data_width = LL_SPI_DATAWIDTH_16BIT;
    break;
  default:
    _spi1_cfg.data_width = LL_SPI_DATAWIDTH_16BIT;
  }

  if (first_bit == 0) {
    _spi1_cfg.first_bit = LL_SPI_MSB_FIRST;
  } else {
    _spi1_cfg.first_bit = LL_SPI_LSB_FIRST;
  }

  switch (clk_prescale) {
  case 0:
    _spi1_cfg.clk_prescale = LL_SPI_BAUDRATEPRESCALER_DIV4;
    break;
  case 1:
    _spi1_cfg.clk_prescale = LL_SPI_BAUDRATEPRESCALER_DIV8;
    break;
  case 2:
    _spi1_cfg.clk_prescale = LL_SPI_BAUDRATEPRESCALER_DIV16;
    break;
  case 3:
    _spi1_cfg.clk_prescale = LL_SPI_BAUDRATEPRESCALER_DIV32;
    break;
  case 4:
    _spi1_cfg.clk_prescale = LL_SPI_BAUDRATEPRESCALER_DIV64;
    break;
  case 5:
    _spi1_cfg.clk_prescale = LL_SPI_BAUDRATEPRESCALER_DIV128;
    break;
  case 6:
    _spi1_cfg.clk_prescale = LL_SPI_BAUDRATEPRESCALER_DIV256;
    break;
  }

  if (clk_polarity == 0) {
    _spi1_cfg.clk_polarity = LL_SPI_POLARITY_LOW;
  } else {
    _spi1_cfg.clk_polarity = LL_SPI_POLARITY_HIGH;
  }

  if (clk_phase == 1) {
    _spi1_cfg.clk_phase = LL_SPI_PHASE_1EDGE;
  } else {
    _spi1_cfg.clk_phase = LL_SPI_PHASE_2EDGE;
  }

  _spi1_cfg.transmit_delay = transmit_delay + 1;

  return SUCCESS;
}

ErrorStatus SPI1_CSN_Config(uint16_t pulse_width, uint8_t polarity) {
  if (pulse_width < 1 || pulse_width >= _spi1_cfg.sample_period) {
    _spi1_error_state |= SPI1_ERROR_INVALID_CSN_PULSE_WIDTH;
    return ERROR;
  }

  if (polarity > 1) {
    _spi1_error_state |= SPI1_ERROR_INVALID_CSN_POLARITY;
    return ERROR;
  }

  _spi1_cfg.csn_pulse_width = pulse_width - 1;

  if (polarity == 0) {
    _spi1_cfg.csn_polarity = LL_TIM_OCPOLARITY_LOW;
  } else {
    _spi1_cfg.csn_polarity = LL_TIM_OCPOLARITY_HIGH;
  }
  return SUCCESS;
}

void SPI1_Init(void) {
  MX_TIM4_Init();
  MX_SPI1_Init();

  LL_TIM_SetAutoReload(TIM4, _spi1_cfg.sample_period);
  LL_TIM_OC_SetCompareCH1(TIM4, _spi1_cfg.csn_pulse_width);
  LL_TIM_OC_SetPolarity(TIM4, LL_TIM_CHANNEL_CH1, _spi1_cfg.csn_polarity);
  LL_TIM_OC_SetCompareCH3(TIM4, _spi1_cfg.transmit_delay);

  LL_SPI_SetDataWidth(SPI1, _spi1_cfg.data_width);
  LL_SPI_SetTransferBitOrder(SPI1, _spi1_cfg.first_bit);
  LL_SPI_SetBaudRatePrescaler(SPI1, _spi1_cfg.clk_prescale);
  LL_SPI_SetClockPolarity(SPI1, _spi1_cfg.clk_polarity);
  LL_SPI_SetClockPhase(SPI1, _spi1_cfg.clk_phase);
}

ErrorStatus SPI1_DMABurstTransaction(uint16_t frame_size, bool increment_tx) {
  if ((frame_size < 1) || (frame_size > RX_BUF_SIZE)) {
    _spi1_error_state |= SPI1_ERROR_INVALID_FRAME_SIZE;
    return ERROR;
  }

  // --- Disable everything first
  _TimerStop();
  LL_TIM_DisableDMAReq_CC3(TIM4);
  LL_SPI_DisableDMAReq_RX(SPI1);
  LL_SPI_Disable(SPI1);
  LL_DMA_DisableChannel(DMA1, LL_DMA_CHANNEL_5); // TX (TIM4_CH3)
  LL_DMA_DisableChannel(DMA1, LL_DMA_CHANNEL_2); // RX (SPI1_RX)

  // --- Clear DMA Flags
  _ClearSPIDMAFlags();

  // --- Flush SPI data register
  while (LL_SPI_IsActiveFlag_RXNE(SPI1)) {
    (void)LL_SPI_ReceiveData16(SPI1);
  }

  // --- Configure TIM4_CH3/SPI_TX DMA (DMA1 Channel 5)
  LL_DMA_SetMemoryIncMode(DMA1, LL_DMA_CHANNEL_5, increment_tx ? LL_DMA_MEMORY_INCREMENT : LL_DMA_MEMORY_NOINCREMENT);
  LL_DMA_SetMemoryAddress(DMA1, LL_DMA_CHANNEL_5, (uint32_t)BUFFER_GetTXDataAddr());
  LL_DMA_SetPeriphAddress(DMA1, LL_DMA_CHANNEL_5, (uint32_t)&SPI1->DR);
  LL_DMA_SetDataLength(DMA1, LL_DMA_CHANNEL_5, frame_size);

  // --- Configure SPI1_RX DMA (DMA1 Channel 2)
  LL_DMA_SetMemoryAddress(DMA1, LL_DMA_CHANNEL_2, (uint32_t)BUFFER_GetRXDataAddr());
  LL_DMA_SetPeriphAddress(DMA1, LL_DMA_CHANNEL_2, (uint32_t)&SPI1->DR);
  LL_DMA_SetDataLength(DMA1, LL_DMA_CHANNEL_2, frame_size);

  // --- Enable DMA request channels
  LL_SPI_Enable(SPI1);
  LL_DMA_EnableChannel(DMA1, LL_DMA_CHANNEL_2); // Enable RX DMA First
  LL_SPI_EnableDMAReq_RX(SPI1);
  LL_DMA_EnableChannel(DMA1, LL_DMA_CHANNEL_5); // Enable Timer/TX Second
  LL_TIM_EnableDMAReq_CC3(TIM4);

  // --- Start TIM4 to initiate SPI DMA burst transaction
  _ConfigCSNtoPWMOutput();
  _TimerStart();
  while (!LL_DMA_IsActiveFlag_TC2(DMA1)) {
  };

  // --- CLEANUP
  _TimerStop();
  LL_TIM_DisableDMAReq_CC3(TIM4);
  LL_SPI_DisableDMAReq_RX(SPI1);
  LL_SPI_Disable(SPI1);
  LL_DMA_DisableChannel(DMA1, LL_DMA_CHANNEL_5);
  LL_DMA_DisableChannel(DMA1, LL_DMA_CHANNEL_2);
  _ClearSPIDMAFlags();

  volatile uint32_t tmp;
  tmp = SPI1->DR;
  tmp = SPI1->SR;
  (void)tmp;

  _ConfigCSNtoGPIOFloat();

  return SUCCESS;
}
