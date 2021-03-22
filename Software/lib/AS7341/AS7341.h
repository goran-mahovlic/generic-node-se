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
 * @file AS7341.h
 * @brief Common file for GNSE applications
 *
 * @copyright Copyright (c) 2021 The Things Industries B.V.
 *
 */

#ifndef AS7341_H
#define AS7341_H

#include "sensirion_i2c.h"

#ifdef __cplusplus
extern "C" {
#endif

#define AS7341_STATUS_OK 0
#define AS7341_STATUS_ERR_BAD_DATA (-1)
#define AS7341_STATUS_CRC_FAIL (-2)
#define AS7341_STATUS_UNKNOWN_DEVICE (-3)
#define AS7341_MEASUREMENT_DURATION_USEC 14400
static const uint16_t AS7341_CMD_DURATION_USEC = 200;

#define SPECTR_ADDR 0x39 << 1

#define WHOAMI_ADRR     0x92
#define LOW_POWER 		0xA9
#define ENABLE_ADDR     0x80
#define ATIME_ADDR      0x81
#define ASTEP_ADDR_LSB  0xCA
#define ASTEP_ADDR_MSB  0xCB
#define CFG0_ADDR       0xA9
#define CFG1_ADDR       0xAA
#define AS7341_CFG6     0xAF
#define AS7341_STATUS2  0xA3
#define AS7341_CH0_DATA_L	0x95

#define REG_AS7341_WTIME	0X83
#define CONFIG_ADDR     0x70
#define LED_ADDR        0x74

#define CFG0_VAL_REG0         0b00000000  //  REGISTER BANK >= 0x80
#define CFG0_VAL_REG1         0b00010000  //  REGISTER BANK 0x60 - 0x74
#define CFG0_VAL_DEF		  0x40
#define TURN_ON_LED           0b00001000  // TURN LED ON
#define TURN_OFF_LED          0b00000000  // TURN LED OFF
#define LED_VAL               0b10000100  // LED POWER CONTROL
#define PON_VAL				  0b00000001
#define PON_SETUP			  0b00011011
#define POFF_VAL			  0b00000000
#define ENABLE_VAL            0b00000011  // POWER, SPM ENABLED | SMUX WAIT, FLICKER DISABLED
#define SMUX_ENABLE           0b00010011  // POWER, SPM, SMUX ENABLED |  WAIT, FLICKER DISABLED
#define SPM_ON                0b00000011
#define SPM_SETUP             0b00000010
#define SPM_OFF               0b00000001
#define AS7341_SMUX_CMD_WRITE 0b00010000
#define GAIN_X16 			  0x04
#define GAIN_X64 			  0x07

/**
 * Read out the serial number
 *
 * @param serial    the address for the result of the serial number
 * @return          0 if the command was successful, else an error code.
 */
void AS7341_read(uint8_t *reading, uint8_t useLED);
void AS7341_writeRegister(uint8_t reg, uint8_t value);
void AS7341_setATIME(uint8_t atime);
void AS7341_setASTEP(uint16_t astep);
void AS7341_setGAIN(uint8_t gain);
void AS7341_setWTIME(uint8_t wtime);

#ifdef __cplusplus
}
#endif

#endif /* AS7341_H */
