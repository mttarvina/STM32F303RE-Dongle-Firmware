/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __TARVS_USART2_H
#define __TARVS_USART2_H

#include <stdbool.h>
#include <stdint.h>
#include <sys/types.h>
#ifdef __cplusplus
extern "C" {
#endif

/* Exported macros -----------------------------------------------------------*/
#define USART2_SERIAL_BUF_SIZE 256
#define USART2_SERIAL_BUF_MASK (USART2_SERIAL_BUF_SIZE - 1)

/* Exported types ------------------------------------------------------------*/
typedef struct {
  uint8_t buffer[USART2_SERIAL_BUF_SIZE];
  volatile uint16_t head;
  volatile uint16_t tail;
} RingBuf_TypeDef;

/* Exported variables --------------------------------------------------------*/

/* Exported functions prototypes ---------------------------------------------*/
/**
 * @brief   Reset the contents of RX ring buffer to 0
 *
 * @param   None
 * @retval  None
 */
void USART2_ResetRXBuffer(void);

/**
 * @brief   Append new character/byte to the RX ring buffer
 *
 * @param   data - character/byte
 * @retval  None
 */
void USART2_AppendToRXBuffer(uint8_t data);

/**
 * @brief   Send each character of the array to USART2 TX one by one
 *
 * @param   data - pointer to char array
 * @param   len - number of bytes/characters to be sent
 * @retval  None
 */
void USART2_SendMessage(const uint8_t *data, uint16_t len);

/**
 * @brief   Check if unread data is currently available in the RX buffer
 *
 * @param   None
 * @retval  true if data is available, false otherwise
 */
bool USART2_IsDataAvailable(void);

/**
 * @brief   Checks if there are at least 14-bytes to read in the RX buffer
 *
 * @param   None
 * @retval  None
 */
bool USART2_IsCommandAvailable(void);

/**
 * @brief   Read all available data from the RX buffer until a line terminator
 * is encountered
 *
 * @param   dest - pointer to char array
 * @param   len - number of bytes/characters to be read, should be less than or
 * equal to max size of dest
 * @retval  number of characters/bytes read
 */
uint16_t USART2_ReadMessage(char *dest, uint16_t len);

#ifdef __cplusplus
}
#endif

#endif /* __TARVS_USART2_H */
