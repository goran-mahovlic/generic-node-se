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
#include "stm32_timer.h"

WWDG_HandleTypeDef WwdgHandle;
IWDG_HandleTypeDef IwdgHandle;
static UTIL_TIMER_Object_t IwdgTimer;

static void SystemClock_Config(void);
static void Error_Handler(void);
static void IWDG_OnTimer(void *context);
void IWDG_StartTimer(void);

//void temperature_sensor_read_data_polling(uint8_t n_reads, uint32_t read_delay);

void WWDG_Start(void){
	  /*##-1- Check if the system has resumed from WWDG reset ####################*/
	  if(__HAL_RCC_GET_FLAG(RCC_FLAG_WWDGRST) != RESET)
	  {
	    /* Clear reset flags */
	    __HAL_RCC_CLEAR_RESET_FLAGS();
	  }
	  /*##-2- Configure the WWDG peripheral ######################################*/
	  /* WWDG clock counter = (PCLK1 (42MHz)/4096)/8) = 1281 Hz (~780 us)
	     WWDG Window value = 80 means that the WWDG counter should be refreshed only
	     when the counter is below 80 (and greater than 64) otherwise a reset will
	     be generated.
	     WWDG Counter value = 127, WWDG timeout = ~780 us * 64 = 49.9 ms */
	  WwdgHandle.Instance = WWDG;
	  WwdgHandle.Init.Prescaler = WWDG_PRESCALER_128;
	  WwdgHandle.Init.Window    = 120;
	  WwdgHandle.Init.Counter   = 127;

	  if(HAL_WWDG_Init(&WwdgHandle) != HAL_OK)
	  {
	    /* Initialization Error */
	    //Error_Handler();
	  }

}

void IWDG_Start(void){
	HAL_IWDG_Init(&IwdgHandle);
	HAL_IWDG_Refresh(&IwdgHandle);
}

static void IWDG_OnTimer(void *context){
	HAL_IWDG_Refresh(&IwdgHandle);
	IWDG_StartTimer();
}

void IWDG_StartTimer(void){
	UTIL_TIMER_Create(&IwdgTimer, 0xFFFFFFFFU, UTIL_TIMER_ONESHOT, IWDG_OnTimer, NULL);
	UTIL_TIMER_SetPeriod(&IwdgTimer, 20000);
	UTIL_TIMER_Start(&IwdgTimer);
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
  WWDG_Start();
  HAL_Init();
  SystemClock_Config();
  __HAL_RCC_WWDG_CLK_DISABLE();

  MX_LoRaWAN_Init();
  //IWDG_Start();
  //IWDG_StartTimer();
  //temperature_sensor_read_data_polling(NUMBER_TEMPERATURE_SENSOR_READ, TEMPERATURE_SENSOR_READ_INTERVAL);
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
  __HAL_RCC_LSEDRIVE_CONFIG(RCC_LSEDRIVE_HIGH);
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
  //GNSE_BSP_LED_Init(LED_RED);
  //GNSE_BSP_LED_On(LED_RED);
  NVIC_SystemReset();
 // GNSE_LPM_EnterStopMode();
 // while (1)
 // {
 // }
}
