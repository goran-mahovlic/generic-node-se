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
#include "GNSE_lpm.h"

uint8_t _wtime = 0x30;
uint16_t _atime = 29;
uint16_t _astep = 599;
uint8_t _gain = 5;

uint8_t regVal;
HAL_StatusTypeDef res;

HAL_StatusTypeDef writeRegisterByte(uint8_t address, uint8_t data)
{
  // Added for stability
  HAL_Delay(1);
  res = HAL_I2C_Mem_Write(&GNSE_BSP_ext_sensor_i2c2, SPECTR_ADDR, address, 1, &data, 1, AS7341_CMD_DURATION_USEC);
  return res;
}

HAL_StatusTypeDef setBank(uint8_t enable){
	  // SMUX, POWER, SPM ENABLED | WAIT, FLICKER DISABLED
	  regVal = 	HAL_I2C_Mem_Read(&GNSE_BSP_ext_sensor_i2c2, SPECTR_ADDR, CFG0_ADDR, 1, &regVal, 1, AS7341_CMD_DURATION_USEC);

	  if (enable){
		SET_BIT(regVal,4);
		writeRegisterByte(CFG0_ADDR, TURN_ON_LED);

	  }
	  else{
		CLEAR_BIT(regVal,4);
		writeRegisterByte(CFG0_ADDR, 0x00);
	  }
	  return 0;
}

void setup_F1F4_Clear_NIR()
{
	setBank(1);
	/*
	 * ADC0	0b0001
	 * ADC1 0b0010
	 * ADC2 0b0011
	 * ADC3 0b0100
	 * ADC4 0x0101
	 * ADC5 0x0110
	 */
	  writeRegisterByte(0x00, 0x30);  // F3 left set to ADC2
	  writeRegisterByte(0x01, 0x01);  // F1 left set to ADC0
	  writeRegisterByte(0x02, 0x00);  // Reserved or disabled
	  writeRegisterByte(0x03, 0x00);  // F8 left disabled
	  writeRegisterByte(0x04, 0x00);  // F6 left disabled
	  writeRegisterByte(0x05, 0x42);  // F4 left connected to ADC3/f2 left connected to ADC1
	  writeRegisterByte(0x06, 0x00);  // F5 left disabled
	  writeRegisterByte(0x07, 0x00);  // F7 left disabled
	  writeRegisterByte(0x08, 0x50);  // CLEAR connected to ADC4
	  writeRegisterByte(0x09, 0x00);  // F5 right disabled
	  writeRegisterByte(0x0A, 0x00);  // F7 right disabled
	  writeRegisterByte(0x0B, 0x00);  // Reserved or disabled
	  writeRegisterByte(0x0C, 0x20);  // F2 right connected to ADC1
	  writeRegisterByte(0x0D, 0x04);  // F4 right connected to ADC3
	  writeRegisterByte(0x0E, 0x00);  // F6/F8 right disabled
	  writeRegisterByte(0x0F, 0x30);  // F3 right connected to ADC2
	  writeRegisterByte(0x10, 0x01);  // F1 right connected to ADC0
	  writeRegisterByte(0x11, 0x50);  // CLEAR right connected to ADC4
	  writeRegisterByte(0x12, 0x00);  // Reserved or disabled
	  writeRegisterByte(0x13, 0x06);  // NIR connected to ADC5
	  setBank(0);
}
void setup_F5F8_Clear_NIR()
{
	  setBank(1);
	  writeRegisterByte(0x00, 0x00); // F3 left disable
	  writeRegisterByte(0x01, 0x00); // F1 left disable
	  writeRegisterByte(0x02, 0x00); // reserved/disable
	  writeRegisterByte(0x03, 0x40); // F8 left connected to ADC3
	  writeRegisterByte(0x04, 0x02); // F6 left connected to ADC1
	  writeRegisterByte(0x05, 0x00); // F4/ F2 disabled
	  writeRegisterByte(0x06, 0x10); // F5 left connected to ADC0
	  writeRegisterByte(0x07, 0x03); // F7 left connected to ADC2
	  writeRegisterByte(0x08, 0x50); // CLEAR Connected to ADC4
	  writeRegisterByte(0x09, 0x10); // F5 right connected to ADC0
	  writeRegisterByte(0x0A, 0x03); // F7 right connected to ADC2
	  writeRegisterByte(0x0B, 0x00); // Reserved or disabled
	  writeRegisterByte(0x0C, 0x00); // F2 right disabled
	  writeRegisterByte(0x0D, 0x00); // F4 right disabled
	  writeRegisterByte(0x0E, 0x24); // F8 right connected to ADC3/ F6 right connected to ADC1
	  writeRegisterByte(0x0F, 0x00); // F3 right disabled
	  writeRegisterByte(0x10, 0x00); // F1 right disabled
	  writeRegisterByte(0x11, 0x50); // CLEAR right connected to AD4
	  writeRegisterByte(0x12, 0x00); // Reserved or disabled
	  writeRegisterByte(0x13, 0x06); // NIR connected to ADC5
	  setBank(0);
}


HAL_StatusTypeDef getID(uint8_t *data)
{
  return HAL_I2C_Mem_Read(&GNSE_BSP_ext_sensor_i2c2, SPECTR_ADDR, WHOAMI_ADRR, 1, data, 1, AS7341_CMD_DURATION_USEC);
}

HAL_StatusTypeDef setupIntegration(uint8_t ATIME, uint16_t ASTEP)
{
  uint16_t ASTEP_LSB = ASTEP & 0xff;
  uint16_t ASTEP_MSB = (ASTEP >> 8);

  regVal = ATIME;
  res = HAL_I2C_Mem_Write(&GNSE_BSP_ext_sensor_i2c2, SPECTR_ADDR, ATIME_ADDR, 1, &regVal, 1, AS7341_CMD_DURATION_USEC);
  if (res != HAL_OK){
    return res;
  }
  regVal = (uint8_t)ASTEP_LSB;
  res = HAL_I2C_Mem_Write(&GNSE_BSP_ext_sensor_i2c2, SPECTR_ADDR, ASTEP_ADDR_LSB, 1, &regVal, 1, AS7341_CMD_DURATION_USEC);
  if (res != HAL_OK){
    return res;
  }
  regVal = (uint8_t)ASTEP_MSB;
  res = HAL_I2C_Mem_Write(&GNSE_BSP_ext_sensor_i2c2, SPECTR_ADDR, ASTEP_ADDR_MSB, 1, &regVal, 1, AS7341_CMD_DURATION_USEC);
  if (res != HAL_OK){
    return res;
  }
  return HAL_OK;
}

HAL_StatusTypeDef lowPowerEnable(void)
{
  // SMUX, POWER, SPM ENABLED | WAIT, FLICKER DISABLED
	regVal = (uint8_t)PON_VAL;
	return HAL_I2C_Mem_Write(&GNSE_BSP_ext_sensor_i2c2, SPECTR_ADDR, ENABLE_ADDR, 1, &regVal, 1, AS7341_CMD_DURATION_USEC);
}

HAL_StatusTypeDef chipEnable(uint8_t enable)
{
  // SMUX, POWER, SPM ENABLED | WAIT, FLICKER DISABLED
  regVal = 	HAL_I2C_Mem_Read(&GNSE_BSP_ext_sensor_i2c2, SPECTR_ADDR, ENABLE_ADDR, 1, &regVal, 1, AS7341_CMD_DURATION_USEC);

  if (enable){
	SET_BIT(regVal,0);

  }
  else{
	CLEAR_BIT(regVal,PON_SETUP);
  }

  return HAL_I2C_Mem_Write(&GNSE_BSP_ext_sensor_i2c2, SPECTR_ADDR, ENABLE_ADDR, 1, &regVal, 1, AS7341_CMD_DURATION_USEC);
}

HAL_StatusTypeDef ledEnable(uint8_t enable){

    regVal = CFG0_VAL_REG1;
    res = HAL_I2C_Mem_Write(&GNSE_BSP_ext_sensor_i2c2, SPECTR_ADDR, CFG0_ADDR, 1, &regVal, 1, AS7341_CMD_DURATION_USEC);
    if (res != HAL_OK){
      return res;
    }

    HAL_I2C_Mem_Read(&GNSE_BSP_ext_sensor_i2c2, SPECTR_ADDR, CONFIG_ADDR, 1, &regVal, 1, AS7341_CMD_DURATION_USEC);

    if(enable == 0){
    	CLEAR_BIT(regVal,TURN_ON_LED);
    }
    else{
    	SET_BIT(regVal,TURN_ON_LED);
    }

	res = HAL_I2C_Mem_Write(&GNSE_BSP_ext_sensor_i2c2, SPECTR_ADDR, CONFIG_ADDR, 1, &regVal, 1, AS7341_CMD_DURATION_USEC);
	if (res != HAL_OK){
		return res;
	}

	if(enable == 0){
		regVal = 0x00;
	}
	else{
		regVal = LED_VAL;
	}

	res = HAL_I2C_Mem_Write(&GNSE_BSP_ext_sensor_i2c2, SPECTR_ADDR, LED_ADDR, 1, &regVal, 1, AS7341_CMD_DURATION_USEC);

	if (res != HAL_OK){
		return res;
	}

	return HAL_OK;
}

HAL_StatusTypeDef enableSpectralMeasurement(uint8_t enable)
{
  regVal = 	HAL_I2C_Mem_Read(&GNSE_BSP_ext_sensor_i2c2, SPECTR_ADDR, ENABLE_ADDR, 1, &regVal, 1, AS7341_CMD_DURATION_USEC);

  if (enable){
	SET_BIT(regVal,PON_SETUP);
	SET_BIT(regVal,SPM_SETUP);
  }
  else{
	CLEAR_BIT(regVal,SPM_SETUP);
  }
  return HAL_I2C_Mem_Write(&GNSE_BSP_ext_sensor_i2c2, SPECTR_ADDR, ENABLE_ADDR, 1, &regVal, 1, AS7341_CMD_DURATION_USEC);
}

void setSMUXCommand(uint8_t command)
{
  writeRegisterByte(AS7341_CFG6, command);
}

HAL_StatusTypeDef enableSMUX(uint8_t enable)
{
    HAL_I2C_Mem_Read(&GNSE_BSP_ext_sensor_i2c2, SPECTR_ADDR, 0x80, 1, &regVal, 1, AS7341_CMD_DURATION_USEC);

	if(enable){
		SET_BIT(regVal,TURN_ON_LED);
	}
	else{
		CLEAR_BIT(regVal,TURN_ON_LED);
	}
    writeRegisterByte(0x80, regVal);
    return regVal;
}

void setSMUXLowChannels(uint8_t f1_f4)
{
  enableSMUX(0);
  enableSpectralMeasurement(0);

  if (f1_f4 >= 1){
    setup_F1F4_Clear_NIR();
  }
  else{
    setup_F5F8_Clear_NIR();
  }

  setSMUXCommand(AS7341_SMUX_CMD_WRITE);
  enableSMUX(1);
  enableSpectralMeasurement(1);
}

uint8_t isDataReady(void)
{
  res = HAL_I2C_Mem_Read(&GNSE_BSP_ext_sensor_i2c2, SPECTR_ADDR, AS7341_STATUS2, 1, &regVal, 1, AS7341_CMD_DURATION_USEC);
  // Added for stability
  HAL_Delay(10);
  if(res != HAL_OK){NVIC_SystemReset();}
  return (regVal >> 6) & 1;
}

uint8_t waitForData(){
  long i=0;
  uint8_t timeout = 1;
  // Added timeout for safety
  while((!isDataReady()) && timeout){
      i++;
      if (i>20000){
    	  timeout = 0;
      }
  }
  return timeout;
}

void readAllChannel(uint8_t *reading){
	//Temporary buffer for f1f4 readings
	uint8_t read_f1f4[12] = { '0' };
	//Temporary buffer for f5f8 readings
	uint8_t read_f5f8[12] = { '0' };
	//Prepare SMUX for f1f4 channels
	setSMUXLowChannels(1);

	if(waitForData()){
		//Read f1f4 channels
		HAL_I2C_Mem_Read(&GNSE_BSP_ext_sensor_i2c2, SPECTR_ADDR, AS7341_CH0_DATA_L, 1, read_f1f4, 12, AS7341_CMD_DURATION_USEC);
	}
	//Setup SMUX for f5f8 channels
	setSMUXLowChannels(0);
	//Wait for data to be ready
	if(waitForData()){
		//Read f5f8 channels
		HAL_I2C_Mem_Read(&GNSE_BSP_ext_sensor_i2c2, SPECTR_ADDR, AS7341_CH0_DATA_L, 1, read_f5f8, 12, AS7341_CMD_DURATION_USEC);
	}
	for (int i=0;i<12;i++){
		reading[i] = read_f1f4[i];
		reading[i+12] = read_f5f8[i];
	}
	HAL_I2C_Mem_Read(&GNSE_BSP_ext_sensor_i2c2, SPECTR_ADDR, 0xA7, 1, &regVal, 12, AS7341_CMD_DURATION_USEC);
}

//Integration time = (ATIME + 1) x (ASTEP + 1) x 2.78µs
//Set the value of register ATIME, through which the value of Integration time can be calculated. The value represents the time that must be spent during data reading.
//Default ATIME 29
//Set the value of register ASTEP, through which the value of Integration time can be calculated. The value represents the time that must be spent during data reading.
//Default ASTEP 599
void AS7341_setup(){

	setupIntegration(_atime,_astep);
	// Set gain x64
	writeRegisterByte(CFG1_ADDR, _gain);
	// Set read times to two
	writeRegisterByte(REG_AS7341_WTIME, _wtime);
	//
}

void AS7341_lowpower(void){
	writeRegisterByte(LOW_POWER, CFG0_VAL_REG1);  // Set sleep to ON
}

WWDG_HandleTypeDef WwdgHandle;

void WWDG_Reset(void){
    /* Refresh WWDG: update counter value to 127, the refresh window is:
    ~780 * (127-80) = 36.6ms < refresh window < ~780 * 64 = 49.9ms */

    if(HAL_WWDG_Refresh(&WwdgHandle) != HAL_OK)
    {
      //Error_Handler();
    }
}

void checkHAL(void){
	regVal = HAL_I2C_Mem_Read(&GNSE_BSP_ext_sensor_i2c2, SPECTR_ADDR, CONFIG_ADDR, 1, &regVal, 1, AS7341_CMD_DURATION_USEC);
	if (regVal != HAL_OK){
		NVIC_SystemReset();
	}
}

void checkLED(void){
	res = HAL_I2C_Mem_Read(&GNSE_BSP_ext_sensor_i2c2, SPECTR_ADDR, LED_ADDR, 1, &regVal, 1, AS7341_CMD_DURATION_USEC);
	if((regVal==LED_VAL) || (res != HAL_OK)){NVIC_SystemReset();}
	res = HAL_I2C_Mem_Write(&GNSE_BSP_ext_sensor_i2c2, SPECTR_ADDR, CFG0_ADDR, 1, &regVal, 1, AS7341_CMD_DURATION_USEC);
	if((READ_BIT(regVal,TURN_ON_LED)) || (res != HAL_OK)){NVIC_SystemReset();}
}

void AS7341_setATIME(uint8_t atime){
	_atime = atime;
}

void AS7341_setWTIME(uint8_t wtime){
	_wtime = wtime;
}

void AS7341_setASTEP(uint16_t astep){
	_astep = astep;
}

void AS7341_setGAIN(uint8_t gain){
	_gain = gain;
}

void AS7341_writeRegister(uint8_t reg, uint8_t value){
	__HAL_RCC_WWDG_CLK_ENABLE();
	WWDG_Reset();
	GNSE_BSP_Ext_Sensor_I2C2_Init();
	enableSpectralMeasurement(0);
	if (reg<0x74){
		setBank(1);
	}
	else{
		setBank(0);
	}
	writeRegisterByte(reg, value);
	checkLED();
	checkHAL();
	AS7341_lowpower();
	chipEnable(0);
	__HAL_RCC_WWDG_CLK_DISABLE();
}

void AS7341_read(uint8_t *reading, uint8_t useLED) {
	__HAL_RCC_WWDG_CLK_ENABLE();
	WWDG_Reset();
	GNSE_BSP_Ext_Sensor_I2C2_Init();
	//getID(reading);
	chipEnable(1);
	// Clear Sleep-After-Interrupt Active.
	writeRegisterByte(0xFA,0x00);
	AS7341_setup();
	if(useLED){
		ledEnable(1);
	}
	WWDG_Reset();
	readAllChannel(reading);
	if(useLED){
		ledEnable(0);
	}
	checkLED();
	checkHAL();
	AS7341_lowpower();
	chipEnable(0);
	__HAL_RCC_WWDG_CLK_DISABLE();
}
