#include "stm32f3xx.h"
#include "stm32f3xx_ll_utils.h"
#include "tarvs_buffer.h"
#include "tarvs_usart2.h"
#include <stdint.h>
#include <string.h>
#include <sys/types.h>

static uint32_t _buf_error_state = 0;
static uint16_t _buf_rx_data[RX_BUF_SIZE] = {0};
static uint16_t _buf_tx_data[TX_BUF_SIZE] = {0};
static uint16_t _buf_rx_index = 0;
static uint16_t _buf_tx_index = 0;

uint32_t BUFFER_GetErrorState(void) { return _buf_error_state; }

void BUFFER_ResetErrorState(void) { _buf_error_state = 0; }

void BUFFER_ResetRXData(void) {
  memset(_buf_rx_data, 0, (2 * RX_BUF_SIZE));
  _buf_rx_index = 0;
}

void BUFFER_ResetTXData(void) {
  memset(_buf_tx_data, 0, (2 * TX_BUF_SIZE));
  _buf_tx_index = 0;
}

uint16_t *BUFFER_GetRXDataAddr(void) { return _buf_rx_data; }

uint16_t *BUFFER_GetTXDataAddr(void) { return _buf_tx_data; }

ErrorStatus BUFFER_AppendRXData(uint16_t data) {
  if (_buf_tx_index >= RX_BUF_SIZE) {
    _buf_error_state |= BUF_ERROR_RX_BUFFER_FULL;
    return ERROR;
  }
  _buf_rx_data[_buf_rx_index] = data;
  _buf_rx_index = _buf_rx_index + 1;
  return SUCCESS;
}

ErrorStatus BUFFER_AppendTXData(uint16_t data) {
  if (_buf_tx_index >= TX_BUF_SIZE) {
    _buf_error_state |= BUF_ERROR_TX_BUFFER_FULL;
    return ERROR;
  }
  _buf_tx_data[_buf_tx_index] = data;
  _buf_tx_index = _buf_tx_index + 1;
  return SUCCESS;
}

ErrorStatus BUFFER_TransmitRXData(uint16_t frame_size) {
  if ((frame_size < 1) || (frame_size > RX_BUF_SIZE)) {
    _buf_error_state |= BUF_ERROR_INVALID_FRAME_SIZE;
    return ERROR;
  }
  uint16_t num_bytes = frame_size * 2;
  USART2_SendMessage((const uint8_t *)_buf_rx_data, num_bytes);
  return SUCCESS;
}

ErrorStatus BUFFER_TransmitTXData(uint16_t frame_size) {
  if ((frame_size < 1) || (frame_size > TX_BUF_SIZE)) {
    _buf_error_state |= BUF_ERROR_INVALID_FRAME_SIZE;
    return ERROR;
  }
  uint16_t num_bytes = frame_size * 2;
  USART2_SendMessage((const uint8_t *)_buf_tx_data, num_bytes);
  return SUCCESS;
}
