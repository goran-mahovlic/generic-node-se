/**
  ******************************************************************************
  * @file    lora_app.c
  * @author  MCD Application Team
  * @brief   Application of the LRWAN Middleware
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
  *
  ******************************************************************************
  */

#include "app.h"
#include "Region.h" /* Needed for LORAWAN_DEFAULT_DATA_RATE */
#include "stm32_timer.h"
#include "sys_app.h"
#include "lora_app.h"
#include "stm32_seq.h"
#include "stm32_lpm.h"
#include "LmHandler.h"
#include "lora_info.h"
#include "AS7341.h"
#include "GNSE_flash.h"

// For time change
// TYPE MINUTES
// 0x01 0x02  // After next downlink node will send every two minutes
// For LED change
// TYPE SET
// 0x02 0x01  // After next downlink LED on sensor will be used
#define SET_TIME 0x01     // 1 byte time in minutes
#define SET_LED 0x02      // 1 byte 1(ON)/0(OFF)
#define SET_REGISTER 0x03 // 2 bytes 1. register 2. data
#define SET_ATIME 0x04    // 1 byte  ATIME
#define SET_ASTEP 0x05    // 2 bytes 1. ASTEP HIGH 2. ASTEP LOW
#define SET_GAIN 0x06     // 1 byte GAIN
#define SET_WTIME 0x07    // 1 byte WTIME
#define RESET_NODE 0x08   // reset
#define GET_REGISTER 0x09 // not implemented


uint8_t as7341_RX[40] = {'\0'};
uint8_t firstStart = 1;
uint8_t startup = 1;
uint8_t downlinkData = 0;
volatile static uint8_t atime = 29;
volatile static uint16_t astep = 599;
volatile static uint8_t again = 5;
volatile static uint8_t wtime = 0x30;
volatile static uint16_t minutes = 15;
volatile static uint8_t ledStatus = 0;
volatile uint8_t time_multiply = 6; // Needs to be 6 if we want to set time in minutes
static uint8_t eeprom_data[20] = { '\0' };

/**
  * @brief LoRa State Machine states
  */
typedef enum TxEventType_e
{
  /**
    * @brief Application data transmission issue based on timer every TxDutyCycleTime
    */
  TX_ON_TIMER,
  /**
    * @brief AppdataTransmition external event plugged on OnSendEvent( )
    */
  TX_ON_EVENT
} TxEventType_t;

/**
  * @brief  LoRa endNode send request
  * @param  none
  * @return none
  */
static void SendTxData(void);

/**
  * @brief  TX timer callback function
  * @param  timer context
  * @return none
  */
static void OnTxTimerEvent(void *context);

/**
  * @brief  LED timer callback function
  * @param  LED context
  * @return none
  */
static void OnTimerLedEvent(void *context);

/**
  * @brief  join event callback function
  * @param  params
  * @return none
  */
static void OnJoinRequest(LmHandlerJoinParams_t *joinParams);

/**
  * @brief  tx event callback function
  * @param  params
  * @return none
  */
static void OnTxData(LmHandlerTxParams_t *params);

/**
  * @brief callback when LoRa endNode has received a frame
  * @param appData
  * @param params
  * @return None
  */
static void OnRxData(LmHandlerAppData_t *appData, LmHandlerRxParams_t *params);

/*!
 * Will be called each time a Radio IRQ is handled by the MAC layer
 *
 */
static void OnMacProcessNotify(void);

/**
  * @brief User application buffer
  */
static uint8_t AppDataBuffer[LORAWAN_APP_DATA_BUFFER_MAX_SIZE];

/**
  * @brief User application data structure
  */
static LmHandlerAppData_t AppData = {0, 0, AppDataBuffer};

static ActivationType_t ActivationType = LORAWAN_DEFAULT_ACTIVATION_TYPE;

/**
  * @brief LoRaWAN handler Callbacks
  */
static LmHandlerCallbacks_t LmHandlerCallbacks =
    {
        .GetBatteryLevel = GetBatteryLevel,
        .GetTemperature = GetTemperatureLevel,
        .OnMacProcess = OnMacProcessNotify,
        .OnJoinRequest = OnJoinRequest,
        .OnTxData = OnTxData,
        .OnRxData = OnRxData
        };

/**
  * @brief LoRaWAN handler parameters
  */
static LmHandlerParams_t LmHandlerParams =
    {
        .ActiveRegion = ACTIVE_REGION,
        .DefaultClass = LORAWAN_DEFAULT_CLASS,
        .AdrEnable = LORAWAN_ADR_STATE,
        .TxDatarate = LORAWAN_DEFAULT_DATA_RATE,
        .PingPeriodicity = LORAWAN_DEFAULT_PING_SLOT_PERIODICITY};

/**
  * @brief Type of Event to generate application Tx
  */
static TxEventType_t EventType = TX_ON_TIMER;

/**
  * @brief Timer to handle the application Tx
  */
static UTIL_TIMER_Object_t TxTimer;

/**
  * @brief Timer to handle the application Tx Led to toggle
  */
static UTIL_TIMER_Object_t TxLedTimer;

static void save_eeprom_data(void){
	  eeprom_data[0] = 0x01;
	  eeprom_data[1] = minutes >> 8;
	  eeprom_data[2] = minutes;
	  eeprom_data[3] = ledStatus;
	  eeprom_data[4] = atime;
	  eeprom_data[5] = astep >> 8;
	  eeprom_data[6] = astep;
	  eeprom_data[7] = again;
	  eeprom_data[8] = wtime;
}

void LoRaWAN_Init(void)
{
  // User can add any indication here (LED manipulation or Buzzer)

  UTIL_SEQ_RegTask((1 << CFG_SEQ_Task_LmHandlerProcess), UTIL_SEQ_RFU, LmHandlerProcess);
  UTIL_SEQ_RegTask((1 << CFG_SEQ_Task_LoRaSendOnTxTimerOrButtonEvent), UTIL_SEQ_RFU, SendTxData);

  /* Init Info table used by LmHandler*/
  LoraInfo_Init();

  /* Init the Lora Stack*/
  LmHandlerInit(&LmHandlerCallbacks);

  LmHandlerConfigure(&LmHandlerParams);

  LmHandlerJoin(ActivationType);

  if(firstStart){
	  firstStart = 0;
		GNSE_Flash_Init();
		GNSE_Flash_Read(0, 9, eeprom_data);
		// Read first byte and check if device is used for first time
		// If data on first byte in eeprom is not 0x05 - write defaults in EEPROM
		if(eeprom_data[0] != 0x01){
		  // Write to eeprom
			save_eeprom_data();
			GNSE_Flash_BlockErase(0,3);
			GNSE_Flash_Write(0, 9, eeprom_data);
		}
		else{
		  minutes = eeprom_data[1] << 8 | eeprom_data[2];
		  ledStatus = eeprom_data[3];
		  atime = eeprom_data[4];
		  astep = eeprom_data[5] << 8 | eeprom_data[6];
		  again = eeprom_data[7];
		  wtime = eeprom_data[8];
		}
		GNSE_Flash_DeInit();
  }

  if (EventType == TX_ON_TIMER)
  {
    /* send every time timer elapses */
    UTIL_TIMER_Create(&TxTimer, 0xFFFFFFFFU, UTIL_TIMER_ONESHOT, OnTxTimerEvent, NULL);
    UTIL_TIMER_SetPeriod(&TxTimer, APP_TX_DUTYCYCLE * time_multiply * minutes);
    UTIL_TIMER_Start(&TxTimer);
  }
  else
  {
    GNSE_BSP_PB_Init(BUTTON_SW1, BUTTON_MODE_EXTI);
  }

}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
  if (GPIO_Pin == BUTTON_SW1_PIN)
  {
    /* Note: when "EventType == TX_ON_TIMER" this GPIO is not initialised */
    UTIL_SEQ_SetTask((1 << CFG_SEQ_Task_LoRaSendOnTxTimerOrButtonEvent), CFG_SEQ_Prio_0);
  }
}

static void OnRxData(LmHandlerAppData_t *appData, LmHandlerRxParams_t *params)
{
  if ((appData != NULL) && (params != NULL))
  {
    static const char *slotStrings[] = {"1", "2", "C", "C Multicast", "B Ping-Slot", "B Multicast Ping-Slot"};

    APP_LOG(ADV_TRACER_TS_OFF, ADV_TRACER_VLEVEL_M, "\r\n ###### ========== MCPS-Indication ==========\r\n");
    APP_LOG(ADV_TRACER_TS_OFF, ADV_TRACER_VLEVEL_M, "\r\n ###### D/L FRAME:%04d | SLOT:%s | PORT:%d | DR:%d | RSSI:%d | SNR:%d\r\n",
            params->DownlinkCounter, slotStrings[params->RxSlot], appData->Port, params->Datarate, params->Rssi, params->Snr);
    switch (appData->Port)
    {
    case LRAWAN_APP_SWITCH_CLASS_PORT:
      /*this port switches the class*/
      if (appData->BufferSize == 1)
      {
        switch (appData->Buffer[0])
        {
        case LRAWAN_APP_SWITCH_CLASS_A:
        {
          LmHandlerRequestClass(CLASS_A);
          break;
        }
        case LRAWAN_APP_SWITCH_CLASS_B:
        {
#if defined(LORAMAC_CLASSB_ENABLED) && (LORAMAC_CLASSB_ENABLED == 1)
          LmHandlerRequestClass(CLASS_B);
#else
          APP_LOG(ADV_TRACER_TS_OFF, ADV_TRACER_VLEVEL_M, "\r\n Configure LORAMAC_CLASSB_ENABLED to be able to switch to this class \r\n");
#endif
          break;
        }
        case LRAWAN_APP_SWITCH_CLASS_C:
        {
          LmHandlerRequestClass(CLASS_C);
          break;
        }
        default:
          break;
        }
      }
      break;
    case LORAWAN_APP_PORT:
      APP_LOG(TS_OFF, VLEVEL_M, "\r\n Recieved %d bytes on LORAWAN_APP_PORT: %d \r\n", appData->BufferSize, LORAWAN_APP_PORT);
      downlinkData = 1;
      switch(appData->Buffer[0]){
      	  case SET_TIME:
      	      minutes = appData->Buffer[1] << 8 | appData->Buffer[2];
      	      if (minutes == 0){minutes=1;}
      	      //UTIL_TIMER_Create(&TxTimer, 0xFFFFFFFFU, UTIL_TIMER_ONESHOT, OnTxTimerEvent, NULL);
      	      UTIL_TIMER_SetPeriod(&TxTimer, APP_TX_DUTYCYCLE * time_multiply * minutes);
      	      UTIL_TIMER_Start(&TxTimer);
      	      //UTIL_TIMER_Create(&TxLedTimer, 0xFFFFFFFFU, UTIL_TIMER_ONESHOT, OnTimerLedEvent, NULL);
      	      UTIL_TIMER_SetPeriod(&TxLedTimer, (APP_TX_DUTYCYCLE * time_multiply * minutes) - 5000);
      	      UTIL_TIMER_Start(&TxLedTimer);
    	  break;
      	  case SET_LED:
      	      ledStatus = appData->Buffer[1];
      	      if (ledStatus>1){ledStatus=1;}
    	  break;
      	  case SET_REGISTER:
      		AS7341_writeRegister(appData->Buffer[1], appData->Buffer[2]);
    	  break;
      	  case SET_ATIME:
      		atime = appData->Buffer[1];
      		AS7341_setATIME(atime);
    	  break;
      	  case SET_ASTEP:
      		astep = appData->Buffer[1] << 8 | appData->Buffer[2];
      		AS7341_setASTEP(astep);
    	  break;
      	  case SET_GAIN:
      		if (appData->Buffer[1] > 10){ appData->Buffer[1] = 10;}
      		again = appData->Buffer[1];
      		AS7341_setGAIN(again);
    	  break;
      	  case SET_WTIME:
      		wtime = appData->Buffer[1];
      		AS7341_setWTIME(wtime);
    	  break;
      	  case GET_REGISTER:
    	  break;
      	  case RESET_NODE:
      		  NVIC_SystemReset();
    	  break;
        default:
        break;
        }
    default:
      APP_LOG(ADV_TRACER_TS_OFF, ADV_TRACER_VLEVEL_M, "\r\n Received %d bytes on undefined port: %d \r\n", appData->BufferSize, LORAWAN_APP_PORT);
      break;
    }
  }
}

static void SendTxData(void)
{
  UTIL_TIMER_Time_t nextTxIn = 0;

  UTIL_TIMER_Create(&TxLedTimer, 0xFFFFFFFFU, UTIL_TIMER_ONESHOT, OnTimerLedEvent, NULL);
  UTIL_TIMER_SetPeriod(&TxLedTimer, (APP_TX_DUTYCYCLE * time_multiply * minutes) - 5000);
  // Read only once here to set value in first message
  if(startup){
	  startup = 0;
	  AS7341_read(as7341_RX, ledStatus);
  }

  // User can add any indication here (LED manipulation or Buzzer)
  uint8_t AS7341_size = 24;
  UTIL_TIMER_Start(&TxLedTimer);
  AppData.Port = LORAWAN_APP_PORT;
  AppData.BufferSize = AS7341_size;
  for (int i= 0;i<AS7341_size;i++){
	  AppData.Buffer[i] = as7341_RX[i];
  }
  memset(as7341_RX, 0, sizeof(as7341_RX));
  //AppData.Buffer[1] = 0xBB;
  //AppData.Buffer[2] = 0xCC;

  if (LORAMAC_HANDLER_SUCCESS == LmHandlerSend(&AppData, LORAWAN_DEFAULT_CONFIRMED_MSG_STATE, &nextTxIn, false))
  {
    APP_LOG(ADV_TRACER_TS_ON, ADV_TRACER_VLEVEL_L, "SEND REQUEST\r\n");
  }
  else if (nextTxIn > 0)
  {
    APP_LOG(ADV_TRACER_TS_ON, ADV_TRACER_VLEVEL_L, "Next Tx in  : ~%d second(s)\r\n", (nextTxIn / 1000));
  }
}

static void OnTxTimerEvent(void *context)
{
  UTIL_SEQ_SetTask((1 << CFG_SEQ_Task_LoRaSendOnTxTimerOrButtonEvent), CFG_SEQ_Prio_0);

  /*Wait for next tx slot*/
  UTIL_TIMER_Start(&TxTimer);
}

static void OnTimerLedEvent(void *context)
{
  // User can add any indication here (LED manipulation or Buzzer)
	AS7341_read(as7341_RX, ledStatus);
	if(downlinkData){
		downlinkData = 0;
		save_eeprom_data();
		GNSE_Flash_Init();
		GNSE_Flash_BlockErase(0,3);
		GNSE_Flash_Write(0, 9, eeprom_data);
		GNSE_Flash_DeInit();
	}
}

static void OnTxData(LmHandlerTxParams_t *params)
{
  if ((params != NULL) && (params->IsMcpsConfirm != 0))
  {
    APP_LOG(ADV_TRACER_TS_OFF, ADV_TRACER_VLEVEL_M, "\r\n###### ========== MCPS-Confirm =============\r\n");
    APP_LOG(ADV_TRACER_TS_OFF, ADV_TRACER_VLEVEL_H, "###### U/L FRAME:%04d | PORT:%d | DR:%d | PWR:%d", params->UplinkCounter,
            params->AppData.Port, params->Datarate, params->TxPower);

    APP_LOG(ADV_TRACER_TS_OFF, ADV_TRACER_VLEVEL_H, " | MSG TYPE:");
    if (params->MsgType == LORAMAC_HANDLER_CONFIRMED_MSG)
    {
      APP_LOG(ADV_TRACER_TS_OFF, ADV_TRACER_VLEVEL_H, "CONFIRMED [%s]\r\n", (params->AckReceived != 0) ? "ACK" : "NACK");
    }
    else
    {
      APP_LOG(ADV_TRACER_TS_OFF, ADV_TRACER_VLEVEL_H, "UNCONFIRMED\r\n");
    }
  }
}

static void OnJoinRequest(LmHandlerJoinParams_t *joinParams)
{
  if (joinParams != NULL)
  {
    if (joinParams->Status == LORAMAC_HANDLER_SUCCESS)
    {
      APP_LOG(ADV_TRACER_TS_OFF, ADV_TRACER_VLEVEL_M, "\r\n###### = JOINED = ");
      if (joinParams->Mode == ACTIVATION_TYPE_ABP)
      {
        APP_LOG(ADV_TRACER_TS_OFF, ADV_TRACER_VLEVEL_M, "ABP ======================\r\n");
      }
      else
      {
        APP_LOG(ADV_TRACER_TS_OFF, ADV_TRACER_VLEVEL_M, "OTAA =====================\r\n");
      }
    }
    else
    {
      APP_LOG(ADV_TRACER_TS_OFF, ADV_TRACER_VLEVEL_M, "\r\n###### = JOIN FAILED\r\n");
    }
  }
}

static void OnMacProcessNotify(void)
{
  UTIL_SEQ_SetTask((1 << CFG_SEQ_Task_LmHandlerProcess), CFG_SEQ_Prio_0);
}

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
