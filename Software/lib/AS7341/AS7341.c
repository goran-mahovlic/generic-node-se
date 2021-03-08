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
#define LOW_POWER 		0xA9
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
#define PON_VAL				  0b00000001
#define POFF_VAL			  0b00000000
#define ENABLE_VAL            0b00000011  // POWER, SPM ENABLED | SMUX WAIT, FLICKER DISABLED
#define SMUX_ENABLE           0b00010011  // POWER, SPM, SMUX ENABLED |  WAIT, FLICKER DISABLED
#define SPM_ON                0b00000011
#define SPM_OFF               0b00000001
#define AS7341_SMUX_CMD_WRITE 0b00000010

static const uint16_t AS7341_CMD_DURATION_USEC = 1000;
uint8_t d1[1] = {"\x92"};
uint8_t buffor[1];
uint8_t startup = 0;
HAL_StatusTypeDef res;

HAL_StatusTypeDef writeRegisterByte(I2C_HandleTypeDef *hi2c, uint8_t address, uint8_t value)
{
  buffor[0] = value;
  res = HAL_I2C_Mem_Write(hi2c, SPECTR_ADDR, address, 1, buffor, 1, AS7341_CMD_DURATION_USEC);
  //if (res != HAL_OK)
    // HAL_GPIO_WritePin(LD5_GPIO_Port, LD5_Pin, GPIO_PIN_SET);
  return res;
}
void setup_F1F4_Clear_NIR(I2C_HandleTypeDef *hi2c)
{
  writeRegisterByte(hi2c, 0x00, 0x30);  // F3 left set to ADC2
  writeRegisterByte(hi2c, 0x01, 0x01);  // F1 left set to ADC0
  writeRegisterByte(hi2c, 0x02, 0x00);  // Reserved or disabled
  writeRegisterByte(hi2c, 0x03, 0x00);  // F8 left disabled
  writeRegisterByte(hi2c, 0x04, 0x00);  // F6 left disabled
  writeRegisterByte(hi2c, 0x05, 0x42);  // F4 left connected to ADC3/f2 left connected to ADC1
  writeRegisterByte(hi2c, 0x06, 0x00);  // F5 left disabled
  writeRegisterByte(hi2c, 0x07, 0x00);  // F7 left disabled
  writeRegisterByte(hi2c, 0x08, 0x50);  // CLEAR connected to ADC4
  writeRegisterByte(hi2c, 0x09, 0x00);  // F5 right disabled
  writeRegisterByte(hi2c, 0x0A, 0x00);  // F7 right disabled
  writeRegisterByte(hi2c, 0x0B, 0x00);  // Reserved or disabled
  writeRegisterByte(hi2c, 0x0C, 0x20);  // F2 right connected to ADC1
  writeRegisterByte(hi2c, 0x0D, 0x04);  // F4 right connected to ADC3
  writeRegisterByte(hi2c, 0x0E, 0x00);  // F6/F8 right disabled
  writeRegisterByte(hi2c, 0x0F, 0x30);  // F3 right connected to ADC2
  writeRegisterByte(hi2c, 0x10, 0x01);  // F1 right connected to ADC0
  writeRegisterByte(hi2c, 0x11, 0x50);  // CLEAR right connected to ADC4
  writeRegisterByte(hi2c, 0x12, 0x00);  // Reserved or disabled
  writeRegisterByte(hi2c, 0x13, 0x06);  // NIR connected to ADC5
}
void setup_F5F8_Clear_NIR(I2C_HandleTypeDef *hi2c)
{
  writeRegisterByte(hi2c, 0x00, 0x00); // F3 left disable
  writeRegisterByte(hi2c, 0x01, 0x00); // F1 left disable
  writeRegisterByte(hi2c, 0x02, 0x00); // reserved/disable
  writeRegisterByte(hi2c, 0x03, 0x40); // F8 left connected to ADC3
  writeRegisterByte(hi2c, 0x04, 0x02); // F6 left connected to ADC1
  writeRegisterByte(hi2c, 0x05, 0x00); // F4/ F2 disabled
  writeRegisterByte(hi2c, 0x06, 0x10); // F5 left connected to ADC0
  writeRegisterByte(hi2c, 0x07, 0x03); // F7 left connected to ADC2
  writeRegisterByte(hi2c, 0x08, 0x50); // CLEAR Connected to ADC4
  writeRegisterByte(hi2c, 0x09, 0x10); // F5 right connected to ADC0
  writeRegisterByte(hi2c, 0x0A, 0x03); // F7 right connected to ADC2
  writeRegisterByte(hi2c, 0x0B, 0x00); // Reserved or disabled
  writeRegisterByte(hi2c, 0x0C, 0x00); // F2 right disabled
  writeRegisterByte(hi2c, 0x0D, 0x00); // F4 right disabled
  writeRegisterByte(hi2c, 0x0E, 0x24); // F8 right connected to ADC2/ F6 right connected to ADC1
  writeRegisterByte(hi2c, 0x0F, 0x00); // F3 right disabled
  writeRegisterByte(hi2c, 0x10, 0x00); // F1 right disabled
  writeRegisterByte(hi2c, 0x11, 0x50); // CLEAR right connected to AD4
  writeRegisterByte(hi2c, 0x12, 0x00); // Reserved or disabled
  writeRegisterByte(hi2c, 0x13, 0x06); // NIR connected to ADC5
}
HAL_StatusTypeDef getID(I2C_HandleTypeDef *hi2c, uint8_t *data)
{
  return HAL_I2C_Mem_Read(hi2c, SPECTR_ADDR, WHOAMI_ADRR, 1, data, 1, AS7341_CMD_DURATION_USEC);
       //HAL_I2C_Mem_Read(&GNSE_BSP_ext_sensor_i2c2, SPECTR_ADDR, WHOAMI_ADRR, 1, id, 1, AS7341_CMD_DURATION_USEC);

}
HAL_StatusTypeDef setupIntegration(I2C_HandleTypeDef *hi2c, uint8_t ATIME, uint16_t ASTEP)
{
  uint16_t ASTEP_LSB = ASTEP & 0xff;
  uint16_t ASTEP_MSB = (ASTEP >> 8);

  buffor[0] = ATIME;
  res = HAL_I2C_Mem_Write(hi2c, SPECTR_ADDR, ATIME_ADDR, 1, buffor, 1, AS7341_CMD_DURATION_USEC);
  if (res != HAL_OK)
    return res;

  buffor[0] = (uint8_t)ASTEP_LSB;
  res = HAL_I2C_Mem_Write(hi2c, SPECTR_ADDR, ASTEP_ADDR_LSB, 1, buffor, 1, AS7341_CMD_DURATION_USEC);
  if (res != HAL_OK)
    return res;

  buffor[0] = (uint8_t)ASTEP_MSB;
  res = HAL_I2C_Mem_Write(hi2c, SPECTR_ADDR, ASTEP_ADDR_MSB, 1, buffor, 1, AS7341_CMD_DURATION_USEC);
  if (res != HAL_OK)
    return res;

  return HAL_OK;
}

HAL_StatusTypeDef lowPowerEnable(I2C_HandleTypeDef *hi2c)
{
  // SMUX, POWER, SPM ENABLED | WAIT, FLICKER DISABLED
  buffor[0] = (uint8_t)PON_VAL;
  return HAL_I2C_Mem_Write(hi2c, SPECTR_ADDR, ENABLE_ADDR, 1, buffor, 1, AS7341_CMD_DURATION_USEC);
}

HAL_StatusTypeDef chipEnable(I2C_HandleTypeDef *hi2c)
{
  // SMUX, POWER, SPM ENABLED | WAIT, FLICKER DISABLED
  buffor[0] = (uint8_t)PON_VAL;
  return HAL_I2C_Mem_Write(hi2c, SPECTR_ADDR, ENABLE_ADDR, 1, buffor, 1, AS7341_CMD_DURATION_USEC);
}

HAL_StatusTypeDef SP_Enable(I2C_HandleTypeDef *hi2c)
{
  // SMUX, POWER, SPM ENABLED | WAIT, FLICKER DISABLED
  buffor[0] = (uint8_t)PON_VAL;
  return HAL_I2C_Mem_Write(hi2c, SPECTR_ADDR, ENABLE_ADDR, 1, buffor, 1, AS7341_CMD_DURATION_USEC);
}

HAL_StatusTypeDef chipDisable(I2C_HandleTypeDef *hi2c)
{
  // SMUX, POWER, SPM ENABLED | WAIT, FLICKER DISABLED
  buffor[0] = (uint8_t)POFF_VAL;
  return HAL_I2C_Mem_Write(hi2c, SPECTR_ADDR, ENABLE_ADDR, 1, buffor, 1, AS7341_CMD_DURATION_USEC);
}

HAL_StatusTypeDef turnOnLED(I2C_HandleTypeDef *hi2c)
{
  buffor[0] = CFG0_VAL_REG1;
  res = HAL_I2C_Mem_Write(hi2c, SPECTR_ADDR, CFG0_ADDR, 1, buffor, 1, AS7341_CMD_DURATION_USEC);
  if (res != HAL_OK)
    return res;

  buffor[0] = TURN_ON_LED;
  res = HAL_I2C_Mem_Write(hi2c, SPECTR_ADDR, CONFIG_ADDR, 1, buffor, 1, AS7341_CMD_DURATION_USEC);
  if (res != HAL_OK)
    return res;

  buffor[0] = LED_VAL;
  res = HAL_I2C_Mem_Write(hi2c, SPECTR_ADDR, LED_ADDR, 1, buffor, 1, AS7341_CMD_DURATION_USEC);
  if (res != HAL_OK)
    return res;

  return HAL_OK;
}
HAL_StatusTypeDef turnOffLED(I2C_HandleTypeDef *hi2c)
{
  buffor[0] = CFG0_VAL_REG1;
  res = HAL_I2C_Mem_Write(hi2c, SPECTR_ADDR, CFG0_ADDR, 1, buffor, 1, AS7341_CMD_DURATION_USEC);
  if (res != HAL_OK)
    return res;

  buffor[0] = TURN_OFF_LED;
  res = HAL_I2C_Mem_Write(hi2c, SPECTR_ADDR, CONFIG_ADDR, 1, buffor, 1, AS7341_CMD_DURATION_USEC);
  if (res != HAL_OK)
    return res;

  buffor[0] = 0;
  res = HAL_I2C_Mem_Write(hi2c, SPECTR_ADDR, LED_ADDR, 1, buffor, 1, AS7341_CMD_DURATION_USEC);
  if (res != HAL_OK)
    return res;

  return HAL_OK;
}
HAL_StatusTypeDef enableSpectralMeasurement(I2C_HandleTypeDef *hi2c, uint8_t enable)
{
  if (enable)
    buffor[0] = SPM_ON;
  else
    buffor[0] = SPM_OFF;
  return HAL_I2C_Mem_Write(hi2c, SPECTR_ADDR, ENABLE_ADDR, 1, buffor, 1, AS7341_CMD_DURATION_USEC);
}
void setSMUXCommand(I2C_HandleTypeDef *hi2c, uint8_t command)
{
  writeRegisterByte(hi2c, AS7341_CFG6, command);
}
HAL_StatusTypeDef enableSMUX(I2C_HandleTypeDef *hi2c)
{
    return writeRegisterByte(hi2c, ENABLE_ADDR, SMUX_ENABLE);
}
void setSMUXLowChannels(I2C_HandleTypeDef *hi2c, uint8_t f1_f4)
{
  enableSpectralMeasurement(hi2c, 0);
 // setSMUXLowChannels(hi2c, AS7341_SMUX_CMD_WRITE);
  if (f1_f4)
  {
    setup_F1F4_Clear_NIR(hi2c);
  }
  else
  {
    setup_F5F8_Clear_NIR(hi2c);
  }
  enableSMUX(hi2c);
  //if (enableSMUX(hi2c) != HAL_OK)
    // HAL_GPIO_WritePin(LD5_GPIO_Port, LD5_Pin, GPIO_PIN_SET);
}
uint8_t isDataReady(I2C_HandleTypeDef *hi2c)
{
  HAL_I2C_Mem_Read(hi2c, SPECTR_ADDR, AS7341_STATUS2, 1, buffor, 1, AS7341_CMD_DURATION_USEC);
    //HAL_GPIO_WritePin(LD5_GPIO_Port, LD5_Pin, GPIO_PIN_SET);

  return (buffor[0] >> 6) & 1;
}
void waitForData(I2C_HandleTypeDef *hi2c)
{
  //HAL_GPIO_WritePin(LD6_GPIO_Port, LD6_Pin, GPIO_PIN_SET);

  // Needs timeout for safety
  while(! isDataReady(hi2c))
  {}
  //HAL_GPIO_WritePin(LD6_GPIO_Port, LD6_Pin, GPIO_PIN_RESET);
}
void readAllChannel(I2C_HandleTypeDef *hi2c, uint8_t *reading)
{
  setSMUXLowChannels(hi2c, 1);
  enableSpectralMeasurement(hi2c, 1);
  waitForData(hi2c);
  HAL_I2C_Mem_Read(hi2c, SPECTR_ADDR, AS7341_CH0_DATA_L, 1, reading, 12, AS7341_CMD_DURATION_USEC);
}

void AS7341_read(uint8_t *reading) {
	GNSE_BSP_Ext_Sensor_I2C2_Init();
	getID(&GNSE_BSP_ext_sensor_i2c2, d1);
	chipEnable(&GNSE_BSP_ext_sensor_i2c2);
	turnOnLED(&GNSE_BSP_ext_sensor_i2c2);
	readAllChannel(&GNSE_BSP_ext_sensor_i2c2, reading);
	turnOffLED(&GNSE_BSP_ext_sensor_i2c2);
	writeRegisterByte(&GNSE_BSP_ext_sensor_i2c2, LOW_POWER, CFG0_VAL_REG1);  // Set sleep to ON
	chipDisable(&GNSE_BSP_ext_sensor_i2c2);
}
