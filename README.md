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

### GPIO
By default, all GPIO pins are configured as analog inputs. Each GPIO can be configured and controlled via the serial command API.

## Serial Command API
This section describes the "language" of communication between the MCU and any high level software application connected through the USB interface.

### Command Structure
The MCU accepts a formatted command composed of a fized array of 12 bytes. The format is described as follows:

```cmd
['action', 'subject', 'param/attr', 'argA3', 'argA2', 'argA1', 'argA0', 'argB3', 'argB2', 'argB1', 'argB0', '!']
```

This was designed so that the a command roughly translates to a comprehensible english statement such as "SET the GPIO at port GPIOA pins 5 & 6 to HIGH" where:
- SET represents the "action"
- GPIO represents the "subject"
- Port GPIOA represents the "param/attr"
- Pins 5 & 6 represents the value for "argA"
- State HIGH represent the value for "argB"

The two arguments: "argA" and "argB" depends on the combination of the first three command bytes and are documented in the later sections for each of the supported peripherals. Each argument has a 4-byte allocation and can sometimes represent a 16-bit integer depending on the intended command.

The last byte "!" signals the end of the command and is required for the MCU to recognize the command as valid. 

#### Command Actions
The first byte of the valid command represents the intended "action" and has the following options:
| Action   | Char Representation | Integer Equivalent |
|----------|---------------------|--------------------|
| GET      | '0'                 | 48                 |
| SET      | '1'                 | 49                 |
| TOGGLE   | '2'                 | 50                 |
| START    | '3'                 | 51                 |
| STOP     | '4'                 | 52                 |
| CAPTURE  | '5'                 | 53                 |
| CONFIG   | '6'                 | 54                 |

#### Command Subject
The second byte represents the peripheral acting as the "subject" in which the "action" is intended for. It has the following options:
| Subject  | Char Representation | Integer Equivalent |
|----------|---------------------|--------------------|
| GPIO     | '0'                 | 48                 |
| ADC      | '1'                 | 49                 |
| I2C      | '2'                 | 50                 |
| SPI      | '3'                 | 51                 |
| TIMER    | '4'                 | 52                 |
| PWM      | '5'                 | 53                 |

### GPIO Commands
The next sections outline the list of supported commands for controlling and configuring any GPIO pins that are present in the actual Nucleo-F303RE board.

#### Configuring a GPIO pin
Configuring a GPIO is done via the CONFIG command and has the following format:
| Action | Subject | Param/Attr  | Argument A (4-bytes) | Argument B (4-bytes)        |
|--------|---------|-------------|----------------------|-----------------------------|
| CONFIG | GPIO    | *gpio_port* | *gpio_pin_mask*      | mode|speed|output_type|pull |

##### GPIO Port Options
This represents the 'param/attr' byte of the command. The valid options are listed below:
- GPIOA = '0'
- GPIOB = '1'
- GPIOC = '2'
- GPIOD = '3'
- GPIOE = '4'
- GPIOF = '5'

##### GPIO Pin Mask
This is a 4-byte integer that represents the locations of the target GPIO pins and represents the Argument A of the command. This allows for multiple pins to be configured in one command. For example, configuring GPIO pins 0 and 1 will require setting bit 0 and 1 of this 4-byte integer to 1. The resulting 4-byte integer would be: 0x0003

##### GPIO Mode
This represents the 1st byte (argB3) of Argument B of the command. The valid options are listed below:
- MODE_ANALOG = '0'
- MODE_OUTPUT = '1'
- MODE_INPUT = '2'
- MODE_ALTERNATE = '3'

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
    "allow_concurrent_runs": true,
    "reveal": "always"
  },
  {
    "label": "Link: Compile Commands",
    "command": "ln -sf build/compile_commands.json compile_commands.json",
    "use_new_terminal": false,
    "allow_concurrent_runs": true,
    "reveal": "always"
  },
  {
    "label": "Build",
    "command": "cmake --build build",
    "use_new_terminal": false,
    "allow_concurrent_runs": true,
    "reveal": "always"
  },
  {
    "label": "Clean",
    "command": "rm -rf build",
    "use_new_terminal": false,
    "allow_concurrent_runs": true,
    "reveal": "always"
  },
  {
    "label": "Flash MCU",
    "command": "STM32_Programmer_CLI -c port=SWD mode=UR -w build/*.elf -v -rst",
    "use_new_terminal": false,
    "allow_concurrent_runs": true,
    "reveal": "always"
  }
]
```
To run/trigger a specific task, we only need to click "CTRL+SHIFT+P --> task: spawn" in Zed editor.

## Notes
- Command execution delay largely depends on the serial baud rate and the amount of time it takes to transmit the command from PC to the MCU. A 14-byte transmission at 230400 baud rate takes about 608us. Once the MCU receives all 14 bytes, the delay taken by parsing the command and routing the state machine to call the appropriate function is insignificant compared to the transmission time.
