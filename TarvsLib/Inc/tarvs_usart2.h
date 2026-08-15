/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __TARVS_USART2_H
#define __TARVS_USART2_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>

/* Private includes ----------------------------------------------------------*/


/* Exported types ------------------------------------------------------------*/


/* Exported constants --------------------------------------------------------*/


/* Exported macro ------------------------------------------------------------*/
#define USART2_SERIAL_BUF_SIZE  512

/* Exported functions prototypes ---------------------------------------------*/
/**
 * @brief   Send each character of the array to USART2 TX one by one
 *
 * @param   str - pointer to char array
 * @param   num_bytes - number of bytes/characters to be sent
 * @retval  None
 */
void USART2_SendMessage(const char *str, uint16_t num_bytes);


/* Private defines -----------------------------------------------------------*/


#ifdef __cplusplus
}
#endif

#endif /* __TARVS_USART2_H */
