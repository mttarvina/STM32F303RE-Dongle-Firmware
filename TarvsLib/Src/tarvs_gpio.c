#include "stm32f303xe.h"
#include "stm32f3xx.h"
#include "stm32f3xx_ll_bus.h"
#include "stm32f3xx_ll_gpio.h"
#include "tarvs_gpio.h"
#include <stdint.h>

#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))

static uint32_t _gpio_error_state = 0;
static GPIO_TypeDef *const _gpio_ports[] = {GPIOA, GPIOB, GPIOC, GPIOD, GPIOE, GPIOF};
static const uint32_t _gpio_rcc_clocks[] = {LL_AHB1_GRP1_PERIPH_GPIOA, LL_AHB1_GRP1_PERIPH_GPIOB,
                                            LL_AHB1_GRP1_PERIPH_GPIOC, LL_AHB1_GRP1_PERIPH_GPIOD,
                                            LL_AHB1_GRP1_PERIPH_GPIOE, LL_AHB1_GRP1_PERIPH_GPIOF};
static const uint32_t _gpio_modes[] = {LL_GPIO_MODE_INPUT, LL_GPIO_MODE_OUTPUT, LL_GPIO_MODE_ALTERNATE,
                                       LL_GPIO_MODE_ANALOG};
static const uint32_t _gpio_speeds[] = {LL_GPIO_SPEED_FREQ_LOW, LL_GPIO_SPEED_FREQ_MEDIUM, LL_GPIO_SPEED_FREQ_HIGH};
static const uint32_t _gpio_output_types[] = {LL_GPIO_OUTPUT_PUSHPULL, LL_GPIO_OUTPUT_OPENDRAIN};
static const uint32_t _gpio_pull_modes[] = {LL_GPIO_PULL_NO, LL_GPIO_PULL_UP, LL_GPIO_PULL_DOWN};

uint32_t GPIO_GetErrorState(void) { return _gpio_error_state; }

void GPIO_ResetErrorState(void) { _gpio_error_state = 0; }

ErrorStatus GPIO_Config(uint8_t port, uint32_t pin_mask, uint8_t mode, uint8_t speed, uint8_t output_type,
                        uint8_t pull_mode) {
  // check for invalid port
  if (port >= ARRAY_SIZE(_gpio_ports)) {
    _gpio_error_state |= GPIO_ERROR_INVALID_PORT;
    return ERROR;
  }

  // check for invalid pin mask
  if ((pin_mask == 0) || (pin_mask > 0xFFFF)) {
    _gpio_error_state |= GPIO_ERROR_INVALID_PIN;
    return ERROR;
  }

  // check for invalid mode
  if (mode >= ARRAY_SIZE(_gpio_modes)) {
    _gpio_error_state |= GPIO_ERROR_INVALID_MODE;
    return ERROR;
  }

  // check for invalid speed
  if (speed >= ARRAY_SIZE(_gpio_speeds)) {
    _gpio_error_state |= GPIO_ERROR_INVALID_SPEED;
    return ERROR;
  }

  // check for invalid output mode
  if (output_type >= ARRAY_SIZE(_gpio_output_types)) {
    _gpio_error_state |= GPIO_ERROR_INVALID_OUTPUT_TYPE;
    return ERROR;
  }

  // check for invalid pull mode
  if (pull_mode >= ARRAY_SIZE(_gpio_pull_modes)) {
    _gpio_error_state |= GPIO_ERROR_INVALID_PULL_MODE;
    return ERROR;
  }

  // ensure AHB clock is enabled
  LL_AHB1_GRP1_EnableClock(_gpio_rcc_clocks[port]);

  LL_GPIO_InitTypeDef _cfg = {.Pin = pin_mask,
                              .Mode = _gpio_modes[mode],
                              .Speed = _gpio_speeds[speed],
                              .OutputType = _gpio_output_types[output_type],
                              .Pull = _gpio_pull_modes[pull_mode]};

  if (LL_GPIO_Init(_gpio_ports[port], &_cfg) != SUCCESS) {
    _gpio_error_state |= GPIO_ERROR_LL_INIT;
    return ERROR;
  }
  return SUCCESS;
}

ErrorStatus GPIO_GetPinState(uint8_t port, uint32_t pin, GPIO_PinStateDef *pin_state) {
  // check for null pointer
  if (pin_state == NULL) {
    _gpio_error_state |= GPIO_ERROR_NULL_PTR;
    return ERROR;
  }

  // check for invalid port
  if (port >= ARRAY_SIZE(_gpio_ports)) {
    _gpio_error_state |= GPIO_ERROR_INVALID_PORT;
    return ERROR;
  }

  // check if pin mask contains exactly ONE bit set
  if ((pin == 0) || (pin & (pin - 1)) != 0 || pin > (1 << GPIO_MAX_NUM)) {
    _gpio_error_state |= GPIO_ERROR_INVALID_PIN;
    return ERROR;
  }

  // find bit position using __builtin_ctz (Count Trailing Zeros)
  uint32_t _pnum = (uint32_t)__builtin_ctz(pin);

  uint32_t _shift_2bit = _pnum * 2;
  uint32_t _mask_2bit = 0x3U << _shift_2bit;

  pin_state->mode = (uint8_t)((READ_REG(_gpio_ports[port]->MODER) & _mask_2bit) >> _shift_2bit);
  pin_state->output_type = (uint8_t)((READ_REG(_gpio_ports[port]->OTYPER) >> _pnum) & 0x1U);
  pin_state->speed = (uint8_t)((READ_REG(_gpio_ports[port]->OSPEEDR) & _mask_2bit) >> _shift_2bit);
  pin_state->pull_mode = (uint8_t)((READ_REG(_gpio_ports[port]->PUPDR) & _mask_2bit) >> _shift_2bit);

  return SUCCESS;
}

ErrorStatus GPIO_Write(uint8_t port, uint32_t pin_mask, uint8_t level) {
  // check for invalid port
  if (port >= ARRAY_SIZE(_gpio_ports)) {
    _gpio_error_state |= GPIO_ERROR_INVALID_PORT;
    return ERROR;
  }

  // check for invalid pin mask
  if ((pin_mask == 0) || (pin_mask > 0xFFFF)) {
    _gpio_error_state |= GPIO_ERROR_INVALID_PIN;
    return ERROR;
  }

  // check for invalid level
  if (level > 1) {
    _gpio_error_state |= GPIO_ERROR_INVALID_LEVEL;
    return ERROR;
  }

  // Lower 16 bits set pins, upper 16 bits reset pins atomically
  _gpio_ports[port]->BSRR = level ? pin_mask : (pin_mask << 16);
  return SUCCESS;
}

ErrorStatus GPIO_Toggle(uint8_t port, uint32_t pin_mask) {
  // check for invalid port
  if (port >= ARRAY_SIZE(_gpio_ports)) {
    _gpio_error_state |= GPIO_ERROR_INVALID_PORT;
    return ERROR;
  }

  // check for invalid pin mask
  if ((pin_mask == 0) || (pin_mask > 0xFFFF)) {
    _gpio_error_state |= GPIO_ERROR_INVALID_PIN;
    return ERROR;
  }

  // toggle using ODR register
  _gpio_ports[port]->ODR ^= pin_mask;
  return SUCCESS;
}

ErrorStatus GPIO_Read(uint8_t port, uint32_t pin_mask, uint16_t *pin_states) {
  // check for null pointer
  if (pin_states == NULL) {
    _gpio_error_state |= GPIO_ERROR_NULL_PTR;
    return ERROR;
  }

  // check for invalid port
  if (port >= ARRAY_SIZE(_gpio_ports)) {
    _gpio_error_state |= GPIO_ERROR_INVALID_PORT;
    return ERROR;
  }

  // check for invalid pin mask
  if ((pin_mask == 0) || (pin_mask > 0xFFFF)) {
    _gpio_error_state |= GPIO_ERROR_INVALID_PIN;
    return ERROR;
  }

  *pin_states = (uint16_t)(_gpio_ports[port]->IDR & pin_mask);
  return SUCCESS;
}
