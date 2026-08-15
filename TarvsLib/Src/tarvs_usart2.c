#include "tarvs_usart2.h"
#include "stm32f3xx_ll_usart.h"

void USART2_SendMessage(const char *str, uint16_t num_bytes) {
  for (uint16_t i = 0; i < num_bytes; i++) {
    while (!LL_USART_IsActiveFlag_TXE(USART2))
      ; // Wait until TX data register empty
    LL_USART_TransmitData8(USART2, (uint8_t)str[i]);
  }
  while (!LL_USART_IsActiveFlag_TC(USART2))
    ; // Wait for Transmission Complete
}
