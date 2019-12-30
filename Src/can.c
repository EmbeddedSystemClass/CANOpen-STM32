/**
  ******************************************************************************
  * File Name          : CAN.c
  * Description        : This file provides code for the configuration
  *                      of the CAN instances.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2019 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "can.h"

/* USER CODE BEGIN 0 */
#include "usart.h"
/* USER CODE END 0 */

CAN_HandleTypeDef hcan1;

/* CAN1 init function */
void MX_CAN1_Init(void)
{

  hcan1.Instance = CAN1;
  hcan1.Init.Prescaler = 9;
  hcan1.Init.Mode = CAN_MODE_NORMAL;
  hcan1.Init.SyncJumpWidth = CAN_SJW_1TQ;
  hcan1.Init.TimeSeg1 = CAN_BS1_5TQ;
  hcan1.Init.TimeSeg2 = CAN_BS2_4TQ;
  hcan1.Init.TimeTriggeredMode = DISABLE;
  hcan1.Init.AutoBusOff = DISABLE;
  hcan1.Init.AutoWakeUp = DISABLE;
  hcan1.Init.AutoRetransmission = DISABLE;
  hcan1.Init.ReceiveFifoLocked = DISABLE;
  hcan1.Init.TransmitFifoPriority = DISABLE;
  if (HAL_CAN_Init(&hcan1) != HAL_OK)
  {
    Error_Handler();
  }

}

void HAL_CAN_MspInit(CAN_HandleTypeDef* canHandle)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};
  if(canHandle->Instance==CAN1)
  {
  /* USER CODE BEGIN CAN1_MspInit 0 */

  /* USER CODE END CAN1_MspInit 0 */
    /* CAN1 clock enable */
    __HAL_RCC_CAN1_CLK_ENABLE();
  
    __HAL_RCC_GPIOA_CLK_ENABLE();
    /**CAN1 GPIO Configuration    
    PA11     ------> CAN1_RX
    PA12     ------> CAN1_TX 
    */
    GPIO_InitStruct.Pin = GPIO_PIN_11|GPIO_PIN_12;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF9_CAN1;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    /* CAN1 interrupt Init */
    HAL_NVIC_SetPriority(CAN1_RX0_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(CAN1_RX0_IRQn);
    HAL_NVIC_SetPriority(CAN1_RX1_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(CAN1_RX1_IRQn);
    HAL_NVIC_SetPriority(CAN1_SCE_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(CAN1_SCE_IRQn);
  /* USER CODE BEGIN CAN1_MspInit 1 */

  /* USER CODE END CAN1_MspInit 1 */
  }
}

void HAL_CAN_MspDeInit(CAN_HandleTypeDef* canHandle)
{

  if(canHandle->Instance==CAN1)
  {
  /* USER CODE BEGIN CAN1_MspDeInit 0 */

  /* USER CODE END CAN1_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_CAN1_CLK_DISABLE();
  
    /**CAN1 GPIO Configuration    
    PA11     ------> CAN1_RX
    PA12     ------> CAN1_TX 
    */
    HAL_GPIO_DeInit(GPIOA, GPIO_PIN_11|GPIO_PIN_12);

    /* CAN1 interrupt Deinit */
    HAL_NVIC_DisableIRQ(CAN1_RX0_IRQn);
    HAL_NVIC_DisableIRQ(CAN1_RX1_IRQn);
    HAL_NVIC_DisableIRQ(CAN1_SCE_IRQn);
  /* USER CODE BEGIN CAN1_MspDeInit 1 */

  /* USER CODE END CAN1_MspDeInit 1 */
  }
} 

/* USER CODE BEGIN 1 */

CAN_TxHeaderTypeDef CAN_TxMsg;
CAN_RxHeaderTypeDef CAN_RxMsg;

void User_CAN_Init(void)
{
  CAN_FilterTypeDef CAN_Filter_Config;
  HAL_StatusTypeDef HAL_Status;

  CAN_TxMsg.IDE = CAN_ID_TYPE;
  CAN_TxMsg.RTR = CAN_FRAME_TYPE;

  /* one can filter instance. for can1, index between 0 ~ 13 */
  CAN_Filter_Config.FilterBank = 0;

  /* to filter a group id, use maskmode */
  /* to filter a specific id, use listmode */

  /* for bank0, setup list mode */
  CAN_Filter_Config.FilterMode = CAN_FILTERMODE_IDLIST;

  /* for 32bit filter bit sequence is:
   stdid[10:3] stid[2:0] exid[17:13] exid[12:5] exid[4:0] ide rtr 0
   so stdid need left shift 21bit
   and extid need left shift 3bit
  */

  /* for 16bit filter bit sequence is:
   stid[10:3] stid[2:0] rtr ide exd[17:15]
   so if std id, need to left shift 5bit
  */
  CAN_Filter_Config.FilterScale = CAN_FILTERSCALE_32BIT;
  CAN_Filter_Config.FilterIdHigh = F446_NUCLEO_BOARD_CAN_ID << 21;
  CAN_Filter_Config.FilterFIFOAssignment = CAN1FIFO;

  CAN_Filter_Config.FilterActivation = ENABLE;
  CAN_Filter_Config.SlaveStartFilterBank = 0;

  HAL_Status = HAL_CAN_ConfigFilter(&hcan1, &CAN_Filter_Config);



  HAL_Status = HAL_CAN_Start(&hcan1);

  if(HAL_Status != HAL_OK)
  {
    UART_Printf(&huart2, "Start CAN1 device failed.\r\n");
  }

  /*
  CAN_FLAG_FMP0,
  CAN_FLAG_FMP1   : FIFO 0 and 1 Message Pending Flags 
                    set to signal that messages are pending in the receive FIFO.
                    These Flags are cleared only by hardware. 

  CAN_FLAG_FF0,
  CAN_FLAG_FF1    : FIFO 0 and 1 Full Flags
                    set when three messages are stored in the selected FIFO.                        

  CAN_FLAG_FOV0              
  CAN_FLAG_FOV1   : FIFO 0 and 1 Overrun Flags
                    set when a new message has been received and passed set the filter while the FIFO was full. 
   */

  HAL_Status = HAL_CAN_ActivateNotification(&hcan1, CAN_IT_RX_FIFO0_MSG_PENDING);
  if(HAL_Status != HAL_OK)
  {
    UART_Printf(&huart2, "Register CAN receive interrupt notification failed. \r\n");
  }

  HAL_Status = HAL_CAN_ActivateNotification(&hcan1, CAN_IT_RX_FIFO0_FULL);
  if(HAL_Status != HAL_OK)
  {
    UART_Printf(&huart2, "Register CAN receive FIFO full interrupt failed.\r\n");
  }

  HAL_Status = HAL_CAN_ActivateNotification(&hcan1, CAN_IT_RX_FIFO0_OVERRUN);
  if(HAL_Status != HAL_OK)
  {
    UART_Printf(&huart2, "Register CAN receive FIFO overflow interrupt failed.\r\n");
  }


  UART_Printf(&huart2, "CAN Device user initialisation finishged.\r\n");
}

/* USER CODE END 1 */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
