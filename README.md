# STM32F303RE-Dongle-Firmware
Firmware to access and control the STM32F303RE peripherals through Python/CLI

## Revision History
[v0.0.1]:
- CLOCK -> Fixed setting, configured for max HCLK frequency of 72MHz
- USART2 -> Dedicated interface for PC communication. Uses interrupt based ring buffer for receiving commands.
- GPIO -> Functional and supports the following features:
  - GPIO Config - configure one or multiple GPIO pins
  - GPIO Write - set the output level/state of one or multiple GPIO pins configured as OUTPUT
  - GPIO Toggle - toggle the output level/state of one or multiple GPIO pins configure as OUTPUT
  - GPIO Read - read the level/state of one or multiple GPIO pins configured as INPUT

## Overview
This project aims to develop a custom dongle firmware for STM32F303RE microcontroller and expose a custom API through USB serial communication to allow access and control of the MCU peripherals such as GPIO, ADC, I2C, or SPI, to high level software applications running in a PC.

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

#### STM32CubeMX Config Settings

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

#### Dedicated Pins

- PA2 --> USART2_TX
- PA3 --> USART2_RX

#### STM32CubeMX Config Settings

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

#### Supported Pins

The GPIO command API supports all GPIOs exposed by the Nucleo-F303RE board.

**Arduino Connectors**

| Arduino Compatible Pin Name | Board Location | MCU GPIO Pin |
| --------------------------- | -------------- | ------------ |
| A0                          | CN8-Pin1       | PA0          |
| A1                          | CN8-Pin2       | PA1          |
| A2                          | CN8-Pin3       | PA4          |
| A3                          | CN8-Pin4       | PB0          |
| A4                          | CN8-Pin5       | PC1          |
| A5                          | CN8-Pin6       | PC0          |
| D0                          | CN9-Pin1       | PA3[*1]      |
| D1                          | CN9-Pin2       | PA2[*1]      |
| D2                          | CN9-Pin3       | PA10         |
| D3                          | CN9-Pin4       | PB3          |
| D4                          | CN9-Pin5       | PB5          |
| D5                          | CN9-Pin6       | PB4          |
| D6                          | CN9-Pin7       | PB10         |
| D7                          | CN9-Pin8       | PA8          |
| D8                          | CN5-Pin1       | PA9          |
| D9                          | CN5-Pin2       | PC7          |
| D10                         | CN5-Pin3       | PB6          |
| D11                         | CN5-Pin4       | PA7          |
| D12                         | CN5-Pin5       | PA6          |
| D13                         | CN5-Pin6       | PA5          |
| D14/SDA                     | CN5-Pin9       | PB9          |
| D15/SCL                     | CN5-Pin10      | PB8          |

[*1] PA2 and PA3 are dedicated pins for USART2_TX/RX so it is recommended to avoid reconfiguring these pins

**ST Morpho Connectors**

| CN7 Pins | MCU GPIO Pin | CN10 Pins | MCU GPIO Pin |
| -------- | ------------ | --------- | ------------ |
| 1        | PC10         | 1         | PC9          |
| 2        | PC11         | 2         | PC8          |
| 3        | PC12         | 3         | PB8          |
| 4        | PD2          | 4         | PC6          |
| 5        |              | 5         | PB9          |
| 6        |              | 6         | PC5          |
| 7        |              | 7         |              |
| 8        |              | 8         |              |
| 9        | PF6          | 9         |              |
| 10       |              | 10        | PD8          |
| 11       | PF7          | 11        | PA5          |
| 12       |              | 12        | PA12         |
| 13       | PA13 [*1]    | 13        | PA6          |
| 14       |              | 14        | PA11         |
| 15       | PA14 [*1]    | 15        | PA7          |
| 16       |              | 16        | PB12         |
| 17       | PA15         | 17        | PB6          |
| 18       |              | 18        | PB11         |
| 19       |              | 19        | PC7          |
| 20       |              | 20        |              |
| 21       | PB7          | 21        | PA9          |
| 22       |              | 22        | PB2          |
| 23       | PC13         | 23        | PA8          |
| 24       |              | 24        | PB1          |
| 25       |              | 25        | PB10         |
| 26       |              | 26        | PB15         |
| 27       |              | 27        | PB4          |
| 28       | PA0          | 28        | PB14         |
| 29       | PF0          | 29        | PB5          |
| 30       | PA1          | 30        | PB13         |
| 31       | PF1          | 31        | PB3          |
| 32       | PA4          | 32        |              |
| 33       |              | 33        | PA10         |
| 34       | PB0          | 34        | PC4          |
| 35       | PC2          | 35        | PA2 [*2]     |
| 36       | PC1          | 36        | PF5          |
| 37       | PC3          | 37        | PA3 [*2]     |
| 38       | PC0          | 38        | PF4          |

[*1] PA13 and PA14 are debug/programming pins connected to the ST-LINK so it is recommended to avoid reconfiguring these pins.

[*2] PA2 and PA3 are dedicated pins for USART2_TX/RX so it is recommended to avoid reconfiguring these pins

### SPI

#### Supported Pins

| SPI Signal | MCU GPIO | Arduino Connector | ST Morpho Connector |
| ---------- | -------- | ----------------- | ------------------- |
| SP1_SCK    | PA5      | CN5-Pin6 (D13)    | CN10-Pin11          |
| SPI1_MISO  | PA6      | CN5-Pin5 (D12)    | CN10-Pin13          |
| SPI1_MOSI  | PA7      | CN5-Pin4 (D11)    | CN10-Pin15          |
| CS         | PB6      | CN5-Pin3 (D10)    | CN10-Pin17          |

### Timer4

Timer4 is the dedicated timer for SPI communication which allows for an adjustable sampling rate

#### Supported Pins

| Timer16 Output Signal | MCU GPIO | Arduino Connector | ST Morpho Connector |
| --------------------- | -------- | ----------------- | ------------------- |
| CS                    | PB6      | CN5-Pin3 (D10)    | CN10-Pin17          |

#### STM32CubeMX Config Settings

- Slave Mode = Disabled
- Trigger Source = Disabled
- Clock Source = Internal
- Channel 1 = PWM

## Serial Command API

This section describes the "language" of communication between the MCU and any high level software application connected through the USB interface.

### Command Structure
The MCU accepts a formatted command composed of a fized array of 12 bytes. The format is described as follows:

```cmd
['action', 'subject', 'param/attr', 'argA3', 'argA2', 'argA1', 'argA0', 'argB3', 'argB2', 'argB1', 'argB0', '!']
```

This was designed so that the a command roughly translates to a comprehensible english statement such as "WRITE to GPIO at port GPIOA pins 5 & 6 to HIGH" where:
- WRITE represents the "action"
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
| READ     | '0'                 | 48                 |
| WRITE    | '1'                 | 49                 |
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
| CONFIG | GPIO    | *gpio_port* | *pin_mask*           |*mode,speed,output_type,pull*|

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
- INPUT = '0'
- OUTPUT = '1'
- ALTERNATE = '2'
- ANALOG = '3'

##### GPIO Speed
This represents the 2nd byte (argB2) of Argument B of the command. The valid options are listed below:
- LOW = '0'
- MEDIUM = '1'
- HIGH = '2'

##### GPIO Output Type
This represents the 3rd byte (argB1) of Argument B of the command. The valid options are listed below:
- PUSH-PULL = '0'
- OPEN-DRAIN = '1'

##### GPIO Pull-up/Pull-down Setting
This represents the 4th byte (argB0) of Argument B of the command. The valid options are listed below:
- NONE = '0'
- PULL-UP = '1'
- PULL-DOWN = '2'

##### Example
- Configure PA6 as OUTPUT, PUSH-PULL, no pull-up/pull-down, and GPIO speed = HIGH.
  - [CONFIG, GPIO, GPIOA, 0x0040, OUTPUT, HIGH, PUSH-PULL, NONE, "!"]
  - Command string --> **"60000401200!\r\n"**

#### Writing to one or multiple GPIO pins
Writing to one or multiple GPIO pins is done via the WRITE command and has the following format:
| Action | Subject | Param/Attr  | Argument A (4-bytes) | Argument B (Bit 3)| Argument B (Bits 2-0) |
|--------|---------|-------------|----------------------|-------------------|-----------------------|
| WRITE  | GPIO    | *gpio_port* | *pin_mask*           | *level*           | "000"                 |

##### GPIO Port Options
Please refer to the *Configuring a GPIO pin* section

##### GPIO Pin Mask
Please refer to the *Configuring a GPIO pin* section

##### GPIO Level
This represents intended output state of one or multiple GPIO pins. The valid options are as listed below:
- LOW = '0'
- HIGH = '1'

##### Example
- Set PA6 as output HIGH.
  - [WRITE, GPIO, GPIOA, 0x0040, HIGH, "000", "!"]
  - Command string --> **"20000401000!\r\n"**

#### Toggle the output of one or multiple GPIO pins
Toggling the output state of one or multiple GPIO pins is done via the TOGGLE command and has the following format:
| Action | Subject | Param/Attr  | Argument A (4-bytes) | Argument B (4-bytes)|
|--------|---------|-------------|----------------------|---------------------|
| TOGGLE | GPIO    | *gpio_port* | *pin_mask*           | "0000"              |

##### GPIO Port Options
Please refer to the *Configuring a GPIO pin* section

##### GPIO Pin Mask
Please refer to the *Configuring a GPIO pin* section

##### Example
- Toggle PA6 of SMT32F303RE.
  - [TOGGLE, GPIO, GPIOA, 0x0040, "0000", "!"]
  - Command string --> **"20000400000!\r\n"**

#### Read input state of one or multiple GPIO pins
Reading the input state of one or multiple GPIO pins is done via the READ command and has the following format:
| Action | Subject | Param/Attr  | Argument A (4-bytes) | Argument B (4-bytes)|
|--------|---------|-------------|----------------------|---------------------|
| READ   | GPIO    | *gpio_port* | *pin_mask*           | "0000"              |

##### GPIO Port Options
Please refer to the *Configuring a GPIO pin* section

##### GPIO Pin Mask
Please refer to the *Configuring a GPIO pin* section

##### MCU Response
The response string represents a 16-bit unsigned integer where each bit corresponds to the input level/state of the corresponding pin.
- Example: a response of "64\r\n" means that a HIGH level signal is present on pin #6 of the GPIO port, and LOW for the other pins.
- Example: a response of "65r\n" means that a HIGH level signal is present on both pin #6 and pin #0 of the GPIO port, and LOW for the other pins.

##### Example
- Read input pin state of PA6.
  - [READ, GPIO, GPIOA, 0x0040, "0000", "!"]
  - Command string --> **"00000400000!\r\n"**

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
- Command execution delay largely depends on the serial baud rate and the amount of time it takes to transmit the command from PC to the MCU.
- Using two line terminators ("\r\n"), every command transmits 14 bytes --> 14 * (8 bits + 1 start bit + 1 stop bit) = 140 bits
- At 230400 baud rate, the transmission time is: 140 bits / (230400 bits/s) --> ~608us
- Once the MCU receives all 14 bytes, the delay taken by parsing the command and routing the state machine to execute the command is insignificant compared to the transmission time.

## Known Issues
- "CMD=?;ERR=2" (CMD_ERROR_INVALID_SIZE) appears intermittently during READ GPIO command - tested at 200ms and 500ms command interval
