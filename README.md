# STM32F303RE-Dongle-Firmware
Firmware to access and control the STM32F303RE peripherals through Python/CLI

## Revision History
[v0.0.1]:
- (Clock) Fixed setting, configured for max HCLK frequency of 72MHz
- (USART2) Uses interrupt based ring buffer for receiving commands from the PC

## Overview
This project aims to develop a custom dongle firmware for STM32F303RE micrcontroller and expose a standard API through USB serial communication to allow access and control of the MCU peripherals such as GPIO, ADC, I2C, or SPI, to high level software applications running in a PC.

This project aims to showcase my capability in developing custom bare metal embedded C firmware on an STM32 microcontroller.

My setup in developing this project is composed of:
- STM32CubeMX - for initial MCU configuration and code generation using HAL/LL libraries
- SMT32CubeCLT + CMake - for compiling, debugging, and uploading the firmware binary to the MCU
- Zed Code Editor - firmware code development

## Hardware Description
This firmware was tested and developed primarily for the Nucleo-F303RE development board.
Board Revision: MB1136 RevC-04

Main References:
- User Manual: https://www.st.com/resource/en/user_manual/um1724-stm32-nucleo64-boards-mb1136-stmicroelectronics.pdf
- Board Schematic: https://www.st.com/resource/en/schematic_pack/mb1136-default-c04_schematic.pdf

Additional Notes:
- The external clock oscillator (HSE), designated as X3, is not populated on the actual board
- USART2 (PA2/PA3 of STM32F303RE) is the assigned UART interface connected to the ST-LINK MCU and is not exposed to both the Arduino UNO and any of the ST Morpho headers

## MCU Overview

## Preripheral Configuration
This section describes how each MCU peripheral is configured and initialized either through STM32CubeMX or through dedicated custom libraries.

- Target MCU: STM32F303RE
- Target Platform/Board: Nucleo-F303RE
- STM32CubeMX Config File: STM32F303RE-Dongle-Firmware.ioc

### Clock
This MCU is configured to run at its maximum rated HCLK frequency of 72MHz. This configuration is currently fixed and mainly done through STM32CubeMX.

STM32CubeMX Config Settings:
- Uses PLL as input to System Clock
- HSI (internal 8MHz oscillator) as PLL clock source
- PLL PreDiv = 1
- PLL Multiplier = 9
- SYSCLK = 72MHz
- AHB Prescaler = 1
- HCLK = 72MHz
- Cortex System Timer = 72MHz
- AHB bus,core,memory, and DMA = 72MHz
- FCLK = 72MHz
- APB1 Prescaler = 2
- APB1 Peripheral Clock (PCLK1) = 36MHz
- APB1 Timer Clock = 72MHz
- APB2 Prescaler = 1
- APB2 Peripheral Clock (PCLK2) = 72MHz
- APB2 Timer Clock = 72MHz
- USART2 Clock = SYSCLK (72MHz)

### USART2
By default, USART2 is connected to the ST-LINK MCU to allow USB serial communication through the virtual COM port feature.
For this project, USART2 will be the main communication interface between the PC and the MCU.

Relevant pins:
- PA2 --> USART2_TX
- PA3 --> USART2_RX

STM32CubeMX Config Settings:
- Clock Source = SYSCLK (72MHz)
- Mode = Asynchronous
- Hardware Flow Control (RS232) = Disabled
- Hardware Flow Control (RS485) = Disabled
- Baud Rate = 230400
- Word Length = 8bits
- Parity = None
- Stop Bits = 1
- Data Direction = RX+TX
- Oversampling = 8 samples
- USART2 Global Interrupt (NVIC) = Enabled

## STM32CubeMX General Project Settings
- STM32CubeMX Config File: STM32F303RE-Dongle-Firmware.ioc
- Toolchain IDE: CMake
- Default Compiler/Linker: GCC
- Generate peripheral initialization as pair of '.c/.h' files
- Keep user code when re-generating
- Every peripheral code uses LL libraries as much as possible

## Build, Compile, Flash
This project uses the following tasks configured for Zed editor located in ".zed/tasks.json"
```json
[
  {
    "label": "CMake: Configure",
    "command": "cmake -B build -G Ninja -DCMAKE_TOOLCHAIN_FILE=./cmake/gcc-arm-none-eabi.cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=ON",
    "use_new_terminal": false,
    "allow_concurrent_runs": false
  },
  {
    "label": "Link: Compile Commands",
    "command": "ln -sf build/compile_commands.json compile_commands.json",
    "use_new_terminal": false,
  },
  {
    "label": "Build",
    "command": "cmake --build build",
    "use_new_terminal": false,
    "allow_concurrent_runs": false
  },
  {
    "label": "Clean",
    "command": "rm -rf build",
    "use_new_terminal": false
  },
  {
    "label": "Flash MCU",
    "command": "STM32_Programmer_CLI -c port=SWD mode=UR -w build/*.elf -v -rst",
    "use_new_terminal": false,
    "allow_concurrent_runs": false
  }
]
```
To run/trigger a specific task, we only need to click "CTRL+SHIFT+P --> task: spawn" in Zed editor.
