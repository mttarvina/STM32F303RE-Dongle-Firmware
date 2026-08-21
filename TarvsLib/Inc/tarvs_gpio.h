/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __TARVS_GPIO_H
#define __TARVS_GPIO_H

#include "stm32f3xx.h"
#include <stdbool.h>
#include <stdint.h>
#include <sys/types.h>
#ifdef __cplusplus
extern "C" {
#endif

/* Exported macros -----------------------------------------------------------*/
#define GPIO_MAX_NUM 15
#define GPIO_ERROR_NULL_PTR (0x1 << 0)
#define GPIO_ERROR_INVALID_PORT (0x1 << 1)
#define GPIO_ERROR_INVALID_PIN (0x1 << 2)
#define GPIO_ERROR_INVALID_MODE (0x1 << 3)
#define GPIO_ERROR_INVALID_SPEED (0x1 << 4)
#define GPIO_ERROR_INVALID_OUTPUT_TYPE (0x1 << 5)
#define GPIO_ERROR_INVALID_PULL_MODE (0x1 << 6)
#define GPIO_ERROR_INVALID_LEVEL (0x1 << 7)
#define GPIO_ERROR_LL_INIT (0x1 << 8)

/* Exported types ------------------------------------------------------------*/
typedef struct {
  uint8_t mode;
  uint8_t speed;
  uint8_t output_type;
  uint8_t pull_mode;
} GPIO_PinStateDef;

/* Exported variables --------------------------------------------------------*/

/* Exported functions prototypes ---------------------------------------------*/
/**
 * @brief   Get the current error flag value of the GPIO module
 *
 * @param   None
 * @retval  Error State (uint32_t)
 */
uint32_t GPIO_GetErrorState(void);

/**
 * @brief   Reset the error flag value of the GPIO module
 *
 * @param   None
 * @retval  None
 */
void GPIO_ResetErrorState(void);

/**
 * @brief   Configure the GPIO pins of a specific port
 *
 * @param   port : index representing the GPIO port; '0' = GPIOA, '1' = GPIOB ...
 * @param   pin_mask : one or multiple GPIO pins
 * @param   mode : '0' = INPUT, '1' = OUTPUT, '2' = ALTERNATE, '3' = ANALOG
 * @param   speed : '0' = LOW, '1' = MEDIUM, '2' = HIGH
 * @param   output_type : '0' = PUSH-PULL, '1' = OPEN-DRAIN
 * @param   pull_mode : '0' = NO-PULL, '1' = PULL-UP, '2' = PULL-DOWN
 * @retval  SUCCESS | ERROR
 */
ErrorStatus GPIO_Config(uint8_t port, uint32_t pin_mask, uint8_t mode, uint8_t speed, uint8_t output_type,
                        uint8_t pull_mode);

/**
 * @brief   Extract state information of a specific GPIO pin
 *
 * @param   port : index representing the GPIO port; '0' = GPIOA, '1' = GPIOB ...
 * @param   pin : pin mask for specific GPIO pin - similar value to LL_GPIO_PIN_X definition
 * @param   pin_state : pointer to a variable of type GPIO_PinStateDef
 * @retval  SUCCESS | ERROR
 */
ErrorStatus GPIO_GetPinState(uint8_t port, uint32_t pin, GPIO_PinStateDef *pin_state);

/**
 * @brief   Set the output state of one or multiple GPIO pins
 *
 * @param   port : index representing the GPIO port; '0' = GPIOA, '1' = GPIOB ...
 * @param   pin_mask : one or multiple GPIO pins
 * @param   level : 0 (LOW) or 1 (HIGH)
 * @retval  SUCCESS | ERROR
 */
ErrorStatus GPIO_Write(uint8_t port, uint32_t pin_mask, uint8_t level);

/**
 * @brief   Toggle the output state of one or multiple GPIO pins
 *
 * @param   port : index representing the GPIO port; '0' = GPIOA, '1' = GPIOB ...
 * @param   pin_mask : one or multiple GPIO pins
 * @retval  SUCCESS | ERROR
 */
ErrorStatus GPIO_Toggle(uint8_t port, uint32_t pin_mask);

/**
 * @brief   Read input state of one or multiple GPIO pins
 *
 * @param   port : index representing the GPIO port; '0' = GPIOA, '1' = GPIOB ...
 * @param   pin_mask : one or multiple GPIO pins
 * @param   level : pointer to 16-bit unsigned integer that will hold the state of each pin of a GPIO port
 * @retval  SUCCESS | ERROR
 */
ErrorStatus GPIO_Read(uint8_t port, uint32_t pin_mask, uint16_t *level);

#ifdef __cplusplus
}
#endif

#endif /* __TARVS_GPIO_H */
