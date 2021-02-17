/** Copyright © 2021 The Things Industries B.V.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/**
 * @file AS7341.c
 *
 * @copyright Copyright (c) 2021 The Things Industries B.V.
 *
 */

#include "AS7341.h"
#include "sensirion_i2c.h"
#include "GNSE_bsp_serial.h"

#define SPECTR_ADDR 0x39 << 1

#define WHOAMI_ADRR     0x92
#define ENABLE_ADDR     0x80
#define CONFIG_ADDR     0x70
#define ATIME_ADDR      0x81
#define ASTEP_ADDR_LSB  0xCA
#define ASTEP_ADDR_MSB  0xCB
#define CFG0_ADDR       0xA9
#define LED_ADDR        0x74
#define AS7341_CFG6     0xAF
#define AS7341_STATUS2  0xA3
#define AS7341_CH0_DATA_L     0x95 

#define CFG0_VAL_REG0         0b00000000  //  REGISTER BANK >= 0x80
#define CFG0_VAL_REG1         0b00010000  //  REGISTER BANK 0x60 - 0x74   
#define TURN_ON_LED           0b00001000  // TURN LED ON
#define TURN_OFF_LED          0b00000000  // TURN LED ON
#define LED_VAL               0b10000100  // LED POWER CONTROL
#define ENABLE_VAL            0b00000011  // POWER, SPM ENABLED | SMUX WAIT, FLICKER DISABLED
#define SMUX_ENABLE           0b00010011  // POWER, SPM, SMUX ENABLED |  WAIT, FLICKER DISABLED
#define SPM_ON                0b00000011
#define SPM_OFF               0b00000001
#define AS7341_SMUX_CMD_WRITE 0b00000010

static const uint16_t AS7341_CMD_DURATION_USEC = 1000;

static const uint16_t AS7341_CMD_SLEEP = 0xB098;
static const uint16_t AS7341_CMD_WAKEUP = 0x3517;

int8_t AS7341_read_ID(uint8_t *ID) {
    int8_t ret;

    HAL_I2C_Mem_Read(&GNSE_BSP_sensor_i2c1, SPECTR_ADDR, WHOAMI_ADRR, 1, ret, 1, AS7341_CMD_DURATION_USEC);
    return ret;
}
