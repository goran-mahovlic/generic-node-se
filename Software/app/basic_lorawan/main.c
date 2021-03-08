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
 * @file main.c
 *
 * @copyright Copyright (c) 2021 The Things Industries B.V.
 *
 */

#include "GNSE_bsp.h"
#include "GNSE_lpm.h"
#include "stm32_seq.h"
#include "sys_app.h"
#include "lora_app.h"
#include "app.h"

static void SystemClock_Config(void);
static void Error_Handler(void);

//void led_toggle(uint8_t n_toggles, uint32_t toggle_delay);
//void buzzer_play(uint8_t n_plays, uint32_t play_delay);
//void secure_element_read_info(void);
//void tempreture_sensor_read_data_polling(uint8_t n_reads, uint32_t read_delay);
//void accelerometer_read_data_polling(uint8_t n_reads, uint32_t read_delay);
//void as7341_read_data_polling(uint8_t n_reads, uint32_t read_delay);
//void flash_read_write(void);
//void battery_report(void);

void uart_rxcallback(uint8_t *rxChar, uint16_t size, uint8_t error)
{
  APP_PPRINTF("\r\n DEBUG_USART data received \r\n");
}

void MX_LoRaWAN_Init(void)
{
  SystemApp_Init();
  LoRaWAN_Init();
}

void MX_LoRaWAN_Process(void)
{
  UTIL_SEQ_Run(UTIL_SEQ_DEFAULT);
}

int main(void)
{
  HAL_Init();
  SystemClock_Config();

  //UTIL_ADV_TRACE_Init();
  //UTIL_ADV_TRACE_StartRxProcess(uart_rxcallback);
  //UTIL_ADV_TRACE_SetVerboseLevel(VLEVEL_H);

  MX_LoRaWAN_Init();

//  APP_PPRINTF("\r\n -------------- Starting GNSE basic app -------------- \r\n");

//  APP_PPRINTF("\r\n Testing battery monitoring functionality \r\n");
//  battery_report();

//  APP_PPRINTF("\r\n Testing LED functionality \r\n");
//  led_toggle(NUMBER_LED_TOGGLE, LED_TOGGLE_INTERVAL);

//  APP_PPRINTF("\r\n Testing secure element functionality \r\n");
//  APP_PPRINTF("\r\n 1) Enabling LOAD_SWITCH_SENSORS \r\n");
//  GNSE_BSP_LS_Init(LOAD_SWITCH_SENSORS);
//  GNSE_BSP_LS_On(LOAD_SWITCH_SENSORS);
//  HAL_Delay(100);

//  GNSE_BSP_Ext_Sensor_I2C2_Init();
//  HAL_Delay(100);

  //  APP_PPRINTF("\r\n 2) Attempting to read secure element serial number \r\n");
//  secure_element_read_info();

//  APP_PPRINTF("\r\n Testing on board sensors functionality \r\n");
//  APP_PPRINTF("\r\n Attempting to read sensors data \r\n");
//  tempreture_sensor_read_data_polling(NUMBER_TEMPRETURE_SENSOR_READ, TEMPRETURE_SENSOR_READ_INTERVAL);
//  accelerometer_read_data_polling(NUMBER_ACCLEROMETER_READ, ACCELEROMETER_READ_INTERVAL);
//  as7341_read_data_polling(NUMBER_as7341_READ, as7341_READ_INTERVAL);

//  APP_PPRINTF("\r\n Testing on board external flash functionality \r\n");
//  APP_PPRINTF("\r\n 1) Enabling LOAD_SWITCH_FLASH \r\n");
//  GNSE_BSP_LS_Init(LOAD_SWITCH_FLASH);
//  GNSE_BSP_LS_On(LOAD_SWITCH_FLASH);
//  HAL_Delay(100);

//  APP_PPRINTF("\r\n 2) Attempting to read & write to external flash \r\n");
//  flash_read_write();

//  APP_PPRINTF("\r\n Testing Buzzer functionality \r\n");
//  buzzer_play(NUMBER_BUZZER_PLAY, BUZZER_PLAY_INTERVAL);

  HAL_I2C_MspInit(&GNSE_BSP_ext_sensor_i2c2);
  GNSE_BSP_Ext_Sensor_I2C2_Init();
  uint8_t msg [24];
  AS7341_read(msg);

  //APP_PPRINTF("\r\n Starting LoRa \r\n");

  while (1)
  {
    MX_LoRaWAN_Process();
  }
}

/**
  * @brief System Clock Configuration
  * @return None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure LSE Drive Capability
  */
  __HAL_RCC_LSEDRIVE_CONFIG(RCC_LSEDRIVE_LOW);
  /** Configure the main internal regulator output voltage
  */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
  /** Initializes the CPU, AHB and APB busses clocks
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_LSE|RCC_OSCILLATORTYPE_MSI;
  RCC_OscInitStruct.LSEState = RCC_LSE_ON;
  RCC_OscInitStruct.MSIState = RCC_MSI_ON;
  RCC_OscInitStruct.MSICalibrationValue = RCC_MSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.MSIClockRange = RCC_MSIRANGE_11;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Configure the SYSCLKSource, HCLK, PCLK1 and PCLK2 clocks dividers
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK3|RCC_CLOCKTYPE_HCLK
                              |RCC_CLOCKTYPE_SYSCLK|RCC_CLOCKTYPE_PCLK1
                              |RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_MSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.AHBCLK3Divider = RCC_SYSCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @return None
  */
void Error_Handler(void)
{
  /* User can add his own implementation to report the HAL error return state
  * A basic Implementation below
  * TODO: Improve with system wide error handler, see https://github.com/TheThingsIndustries/generic-node-se/issues/57
  */
  GNSE_BSP_LED_Init(LED_RED);
  GNSE_BSP_LED_On(LED_RED);
  GNSE_LPM_EnterStopMode();
  while (1)
  {
  }
}
