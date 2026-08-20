#include "tarvs_cmd.h"
#include <stdint.h>
#include <time.h>

static uint32_t _cmd_error_state = 0;

uint32_t CMD_GetErrorState(void) { return _cmd_error_state; }

void CMD_ResetErrorState(void) { _cmd_error_state = 0; }

ErrorStatus CMD_Parse(const char *src, CMD_StructDef *cmd, uint16_t len) {
  if (src == NULL || cmd == NULL) {
    _cmd_error_state |= CMD_ERROR_NULL_PTR;
    return ERROR;
  }
  if (len != CMD_VALID_SIZE) {
    _cmd_error_state |= CMD_ERROR_INVALID_SIZE;
    return ERROR;
  }
  if (src[CMD_VALID_SIZE - 1] != '!') {
    _cmd_error_state |= CMD_ERROR_INVALID_FORMAT;
    return ERROR;
  }

  cmd->action = src[0];
  cmd->subject = src[1];
  cmd->param = src[2];

  for (uint8_t i = 0; i < CMD_VALID_ARG_SIZE; i++) {
    cmd->argA[i] = src[i + 3];
  }
  for (uint8_t i = 0; i < CMD_VALID_ARG_SIZE; i++) {
    cmd->argB[i] = src[i + 7];
  }

  return SUCCESS;
}

ErrorStatus CMD_ArgToInt(const uint8_t *src, uint16_t *dest) {
  if (src == NULL || dest == NULL) {
    _cmd_error_state |= CMD_ERROR_NULL_PTR;
    return ERROR;
  }

  uint16_t _val = 0;
  for (uint8_t i = 0; i < CMD_VALID_ARG_SIZE; i++) {
    uint8_t _c = src[i];
    uint8_t _digit;

    if (_c >= '0' && _c <= '9') {
      _digit = _c - '0';
    } else if (_c >= 'A' && _c <= 'F') {
      _digit = _c - 'A' + 10;
    } else if (_c >= 'a' && _c <= 'f') {
      _digit = _c - 'a' + 10;
    } else {
      _cmd_error_state |= CMD_ERROR_INVALID_ARGUMENT;
      *dest = 0; // assign a safe value in case dest is uninitialized
      return ERROR;
    }
    _val = (_val << 4) | _digit;
  }

  *dest = _val;
  return SUCCESS;
}

ErrorStatus CMD_CharToInt(uint8_t src, uint8_t *dest) {
  if (dest == NULL) {
    _cmd_error_state |= CMD_ERROR_NULL_PTR;
    return ERROR;
  }

  if (src >= '0' && src <= '9') {
    *dest = src - '0';
  } else if (src >= 'A' && src <= 'F') {
    *dest = src - 'A' + 10;
  } else if (src >= 'a' && src <= 'f') {
    *dest = src - 'a' + 10;
  } else {
    _cmd_error_state |= CMD_ERROR_INVALID_ARGUMENT;
    *dest = 0xFF; // assign an invalid value in case dest is uninitialized, so other error checks can catch it later
    return ERROR;
  }
  return SUCCESS;
}
