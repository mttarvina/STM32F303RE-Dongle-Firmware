/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __TARVS_CMD_H
#define __TARVS_CMD_H

#include "stm32f3xx.h"
#include <stdbool.h>
#include <stdint.h>
#include <sys/types.h>
#ifdef __cplusplus
extern "C" {
#endif

/* Exported macros -----------------------------------------------------------*/
#define CMD_VALID_SIZE 12
#define CMD_VALID_ARG_SIZE 4
#define CMD_ERROR_NULL_PTR (0x1 << 0)
#define CMD_ERROR_INVALID_SIZE (0x1 << 1)
#define CMD_ERROR_INVALID_FORMAT (0x1 << 2)
#define CMD_ERROR_INVALID_ARGUMENT (0x1 << 3)

/* Exported types ------------------------------------------------------------*/
typedef enum {
  CMD_READ = 48,
  CMD_WRITE = 49,
  CMD_TOGGLE = 50,
  CMD_START = 51,
  CMD_STOP = 52,
  CMD_CAPTURE = 53,
  CMD_CONFIG = 54,
} CMD_Action;

typedef enum {
  CMD_GPIO = 48,
  CMD_ADC = 49,
  CMD_I2C = 50,
  CMD_SPI = 51,
  CMD_TIMER = 52,
  CMD_PWM = 53,
} CMD_Subject;

typedef struct {
  CMD_Action action;
  CMD_Subject subject;
  uint8_t param;
  uint8_t argA[CMD_VALID_ARG_SIZE];
  uint8_t argB[CMD_VALID_ARG_SIZE];
} CMD_StructDef;

/* Exported variables --------------------------------------------------------*/

/* Exported functions prototypes ---------------------------------------------*/
/**
 * @brief   Get the current error flag value of the CMD module
 *
 * @param   None
 * @retval  Error State (uint32_t)
 */
uint32_t CMD_GetErrorState(void);

/**
 * @brief   Reset the error flag value of the CMD module
 *
 * @param   None
 * @retval  None
 */
void CMD_ResetErrorState(void);

/**
 * @brief   Parse an array of bytes/characters and convert them into a structured command format
 *
 * @param   src : pointer to the source array
 * @param   cmd : pointer to an instance of CMD_StructDef
 * @param   len : length of the source array
 * @retval  SUCCESS if parsed command is valid, ERROR otherwise
 */
ErrorStatus CMD_Parse(const char *src, CMD_StructDef *cmd, uint16_t len);

/**
 * @brief   Convert a 4-byte/character array into a 16-bit unsigned integer
 *
 * @param   arg : 4-byte/character array
 * @param   dest: pointer to destination variable (uint16_t)
 * @retval  SUCCESS | ERROR
 */
ErrorStatus CMD_ArgToInt(const uint8_t *src, uint16_t *dest);

/**
 * @brief   Convert a byte/character into an 8-bit unsigned integer
 *
 * @param   src : 1-byte/character parameter
 * @param   dest: pointer to destination variable (uint8_t)
 * @retval  SUCCESS | ERROR
 */
ErrorStatus CMD_CharToInt(uint8_t src, uint8_t *dest);

#ifdef __cplusplus
}
#endif

#endif /* __TARVS_CMD_H */
