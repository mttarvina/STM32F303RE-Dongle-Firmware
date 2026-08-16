/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __TARVS_CMD_H
#define __TARVS_CMD_H

#include <stdint.h>
#include <sys/types.h>
#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>
#include <stdbool.h>

/* Exported macros -----------------------------------------------------------*/
#define CMD_VALID_SIZE 12
#define CMD_ARGUMENT_SIZE 4
/* Exported types ------------------------------------------------------------*/
typedef enum {
  CMD_GET = 48,
  CMD_SET,
  CMD_TOGGLE,
  CMD_START,
  CMD_STOP,
  CMD_CAPTURE,
} CMD_Action;

typedef enum {
  CMD_GPIO = 48,
  CMD_ADC,
  CMD_I2C,
  CMD_SPI,
  CMD_TIMER,
  CMD_PWM,
  CMD_SEQ,
} CMD_Subject;

typedef struct {
  CMD_Action action;
  CMD_Subject subject;
  uint8_t param;
  uint8_t argA[CMD_ARGUMENT_SIZE];
  uint8_t argB[CMD_ARGUMENT_SIZE];
} CMD_StructDef;

/* Exported variables --------------------------------------------------------*/

/* Exported functions prototypes ---------------------------------------------*/
/**
 * @brief   Parse an array of bytes/characters and convert them into a structured command format
 *
 * @param   src : pointer to the source array
 * @param   cmd : pointer to an instance of CMD_StructDef
 * @param   len : length of the source array
 * @retval  true if parsed command is valid, false otherwise
 */
bool CMD_Parse(char *src, CMD_StructDef *cmd, uint16_t len);

/**
 * @brief   Convert a 4-byte array argument into a 16-bit unsigned integer
 *
 * @param   arg : 4-byte array
 * @retval  16-bit unsigned integer equivalent
 */
uint16_t CMD_ArgToInt(uint8_t *arg);

#ifdef __cplusplus
}
#endif

#endif /* __TARVS_CMD_H */
