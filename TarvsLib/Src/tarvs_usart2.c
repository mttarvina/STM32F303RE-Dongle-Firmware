#include "stm32f3xx_ll_usart.h"
#include "tarvs_cmd.h"
#include "tarvs_usart2.h"
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

static RingBuf_TypeDef _rx_buf = {
    .buffer = {0},
    .head = 0,
    .tail = 0,
};

void USART2_ResetRXBuffer(void) {
  memset(_rx_buf.buffer, 0, USART2_SERIAL_BUF_SIZE);
  _rx_buf.head = 0;
  _rx_buf.tail = 0;
}

void USART2_AppendToRXBuffer(uint8_t data) {
  uint16_t next_head = (_rx_buf.head + 1) & USART2_SERIAL_BUF_MASK;

  // If buffer is full, drop oldest data by incrementing tail safely
  if (next_head == _rx_buf.tail) {
    _rx_buf.tail = (_rx_buf.tail + 1) & USART2_SERIAL_BUF_MASK;
  }

  // append the new character/byte
  _rx_buf.buffer[_rx_buf.head] = data;
  _rx_buf.head = next_head;
}

void USART2_SendMessage(const uint8_t *data, uint16_t len) {
  for (uint16_t i = 0; i < len; i++) {
    while (!LL_USART_IsActiveFlag_TXE(USART2))
      ; // Wait until TX data register is empty
    LL_USART_TransmitData8(USART2, (uint16_t)data[i]);
  }
  while (!LL_USART_IsActiveFlag_TC(USART2))
    ; // Wait for Transmission Complete
}

bool USART2_IsDataAvailable(void) { return (_rx_buf.head != _rx_buf.tail); }

bool USART2_IsCommandAvailable(void) {
  uint16_t available_bytes = 0;

  if (_rx_buf.head >= _rx_buf.tail) {
    available_bytes = _rx_buf.head - _rx_buf.tail;
  } else {
    available_bytes = USART2_SERIAL_BUF_SIZE - _rx_buf.tail + _rx_buf.head;
  }
  // Minimum valid frame is 12 bytes + 2 delimiter (\r\n) = 14 bytes
  if (available_bytes < (CMD_VALID_SIZE + 2U)) {
    return false;
  }
  return true;
}

uint16_t USART2_ReadMessage(char *dest, uint16_t len) {
  uint16_t i = 0;

  // Grab a snapshot of head in case the ISR modifies it mid-execution
  uint16_t current_head = _rx_buf.head;

  if ((len == 0) || (current_head == _rx_buf.tail)) {
    if (len > 0) {
      dest[0] = '\0';
    }
    return 0;
  }

  // Read until dest is full (leaving room for \0) or ring buffer is empty, or
  // until line terminator is encountered
  while ((i < (len - 1)) && (current_head != _rx_buf.tail)) {
    char next_char = _rx_buf.buffer[_rx_buf.tail];

    // If we hit any line terminator, consume it and break
    if (next_char == '\r' || next_char == '\n') {
      _rx_buf.tail = (_rx_buf.tail + 1) & USART2_SERIAL_BUF_MASK; // Consume it

      // Handle CRLF (\r\n) pairs if they arrive together
      current_head = _rx_buf.head; // update head snapshot
      if (current_head != _rx_buf.tail) {
        char check_lf = _rx_buf.buffer[_rx_buf.tail];
        if ((next_char == '\r' && check_lf == '\n') || (next_char == '\n' && check_lf == '\r')) {
          _rx_buf.tail = (_rx_buf.tail + 1) & USART2_SERIAL_BUF_MASK; // Consume the second half
        }
      }
      break;
    }

    // Otherwise, copy the character normally
    dest[i] = next_char;
    _rx_buf.tail = (_rx_buf.tail + 1) & USART2_SERIAL_BUF_MASK;
    i++;

    // Refresh head snapshot for the next loop iteration
    current_head = _rx_buf.head;
  }

  dest[i] = '\0'; // Cleanly null-terminate
  return i;
}
