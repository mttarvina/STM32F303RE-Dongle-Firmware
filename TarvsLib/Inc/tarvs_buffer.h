/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __TARVS_BUFFER_H
#define __TARVS_BUFFER_H

#include "stm32f3xx.h"
#ifdef __cplusplus
extern "C" {
#endif

/* Exported macros -----------------------------------------------------------*/
#define RX_BUF_SIZE 8192   // number of 16-bit/2-byte samples, allocate a larger value to RX
#define TX_BUF_SIZE 512    // number of 16-bit/2-byte samples, allocate a smaller value to TX
#define RX_CHUNK_SIZE 1024 // this is in bytes

#define BUF_ERROR_RX_BUFFER_FULL (0x1 << 0)
#define BUF_ERROR_TX_BUFFER_FULL (0x1 << 1)
#define BUF_ERROR_INVALID_FRAME_SIZE (0x1 << 2)

/* Exported types ------------------------------------------------------------*/

/* Exported variables --------------------------------------------------------*/

/* Exported functions prototypes ---------------------------------------------*/
uint32_t BUFFER_GetErrorState(void);
void BUFFER_ResetErrorState(void);
void BUFFER_ResetRXData(void);
void BUFFER_ResetTXData(void);
uint16_t *BUFFER_GetRXDataAddr(void);
uint16_t *BUFFER_GetTXDataAddr(void);
ErrorStatus BUFFER_AppendRXData(uint16_t data);
ErrorStatus BUFFER_AppendTXData(uint16_t data);
ErrorStatus BUFFER_TransmitRXData(uint16_t frame_size);
ErrorStatus BUFFER_TransmitTXData(uint16_t frame_size);

#ifdef __cplusplus
}
#endif

#endif /* __TARVS_BUFFER_H */
