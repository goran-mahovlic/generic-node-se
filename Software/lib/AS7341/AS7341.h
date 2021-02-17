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

/**
 * Read out the serial number
 *
 * @param serial    the address for the result of the serial number
 * @return          0 if the command was successful, else an error code.
 */
int8_t AS7341_read_ID(uint8_t *ID);

#ifdef __cplusplus
}
#endif

#endif /* AS7341_H */
