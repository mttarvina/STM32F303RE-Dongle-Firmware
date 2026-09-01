/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __TARVS_SPI_H
#define __TARVS_SPI_H

#include "stm32f3xx.h"
#include <stdbool.h>
#include <stdint.h>
#include <sys/types.h>
#ifdef __cplusplus
extern "C" {
#endif

/* Exported macros -----------------------------------------------------------*/
#define SPI1_MIN_SAMPLE_RATE 2   // in kHz
#define SPI1_MAX_SAMPLE_RATE 800 // in kHz

#define SPI1_ERROR_NULL_PTR (0x1 << 0)
#define SPI1_ERROR_INVALID_SAMPLE_PERIOD (0x1 << 1)
#define SPI1_ERROR_INVALID_TRANSMIT_DELAY (0x1 << 2)
#define SPI1_ERROR_INVALID_CSN_PULSE_WIDTH (0x1 << 3)
#define SPI1_ERROR_INVALID_CSN_POLARITY (0x1 << 4)
#define SPI1_ERROR_TX_BUFFER_FULL (0x1 << 5)
#define SPI1_ERROR_INVALID_FRAME_SIZE (0x1 << 6)
#define SPI1_ERROR_INVALID_DATA_WIDTH (0x1 << 7)
#define SPI1_ERROR_INVALID_FIRST_BIT (0x1 << 8)
#define SPI1_ERROR_INVALID_PRESCALER (0x1 << 9)
#define SPI1_ERROR_INVALID_CLK_POLARITY (0x1 << 10)
#define SPI1_ERROR_INVALID_CLK_PHASE (0x1 << 11)

/* Exported types ------------------------------------------------------------*/
typedef struct {
  uint32_t data_width;
  uint32_t first_bit;
  uint32_t clk_prescale;
  uint32_t clk_polarity;
  uint32_t clk_phase;
  uint16_t sample_period;
  uint16_t transmit_delay;  // ~13.8889 ns resolution
  uint16_t csn_pulse_width; // ~13.8889 ns resolution
  uint8_t csn_polarity;
} SPI1_ConfigDef;
/* Exported variables --------------------------------------------------------*/

/* Exported functions prototypes ---------------------------------------------*/
uint32_t SPI1_GetErrorState(void);
void SPI1_ResetErrorState(void);
ErrorStatus SPI1_SetSampleRate(uint16_t sample_rate_khz);
ErrorStatus SPI1_Config(uint8_t data_width, uint8_t first_bit, uint8_t clk_prescale, uint8_t clk_polarity,
                        uint8_t clk_phase, uint16_t transmit_delay);
ErrorStatus SPI1_CSN_Config(uint16_t pulse_width, uint8_t polarity);
void SPI1_Init(void);
ErrorStatus SPI1_DMABurstTransaction(uint16_t frame_size, bool increment_tx);

#ifdef __cplusplus
}
#endif

#endif /* __TARVS_SPI_H */
