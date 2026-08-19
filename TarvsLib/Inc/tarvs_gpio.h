/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __TARVS_GPIO_H
#define __TARVS_GPIO_H

#include <stdbool.h>
#include <stdint.h>
#include <sys/types.h>
#include "stm32f3xx.h"
#ifdef __cplusplus
extern "C" {
#endif

/* Exported macros -----------------------------------------------------------*/
#define GPIO_MAX_NUM  15
#define GPIO_ERROR_INVALID_PORT         (0x1 << 0)
#define GPIO_ERROR_INVALID_PIN          (0x1 << 1)
#define GPIO_ERROR_INVALID_MODE         (0x1 << 2)
#define GPIO_ERROR_INVALID_SPEED        (0x1 << 3)
#define GPIO_ERROR_INVALID_OUTPUT_MODE  (0x1 << 4)
#define GPIO_ERROR_INVALID_PULL_MODE    (0x1 << 5)
#define GPIO_ERROR_LL_INIT              (0x1 << 6)
#define GPIO_ERROR_NULL_PTR             (0x1 << 7)

/* Exported types ------------------------------------------------------------*/
typedef struct {
  uint8_t mode;
  uint8_t speed;
  uint8_t output_mode;
  uint8_t pull_mode;
} GPIO_PinStateDef;

/* Exported variables --------------------------------------------------------*/

/* Exported functions prototypes ---------------------------------------------*/
uint32_t GPIO_GetErrorState(void);
void GPIO_ClearErrorState(void);
ErrorStatus GPIO_Config(uint8_t port, uint32_t pin_mask, uint8_t mode, uint8_t speed, uint8_t output_mode, uint8_t pull_mode);
ErrorStatus GPIO_GetPinState(uint8_t port, uint32_t pin, GPIO_PinStateDef *pin_state);

#ifdef __cplusplus
}
#endif

#endif /* __TARVS_GPIO_H */
