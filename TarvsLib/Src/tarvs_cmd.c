#include "tarvs_cmd.h"
#include <stdint.h>

bool CMD_Parse(char *src, CMD_StructDef *cmd, uint16_t len) {
  cmd->action = 0;
  cmd->subject = 0;
  cmd->param = 0;

  if (len != CMD_VALID_SIZE) {
    return false;
  }
  if (src[CMD_VALID_SIZE - 1] != '!') {
    return false;
  }

  cmd->action = src[0];
  cmd->subject = src[1];
  cmd->param = src[2];

  for (uint8_t i = 0; i < CMD_ARGUMENT_SIZE; i++) {
    cmd->argA[i] = src[i + 3];
  }
  for (uint8_t i = 0; i < CMD_ARGUMENT_SIZE; i++) {
    cmd->argB[i] = src[i + 7];
  }
  return true;
}

uint16_t CMD_ArgToInt(uint8_t *arg){
  uint8_t arg_num[CMD_ARGUMENT_SIZE] = {0};

  for (uint8_t i = 0; i < CMD_ARGUMENT_SIZE; i++){\
    // convert characters from '0' to '9'
    if ((arg[i] >= 48) && (arg[i] <= 57)) {
      arg_num[i] = arg[i] - 48;
    }
    // convert characters from 'A' to 'F'
    else if ((arg[i] >= 65) && (arg[i] <= 70)){
      arg_num[i] = arg[i] - 55;
    }
    // convert characters from 'a' to 'f'
    else if ((arg[i] >= 97) && (arg[i] <= 102)){
      arg_num[i] = arg[i] - 87;
    }
  }

  // First element represents the hex-byte including the MSB,
  // Last element represents the hex-byte including the LSB
  return ((arg_num[0] << 12) | (arg_num[1] << 8) | (arg_num[2] << 4) | arg_num[3]);
}
