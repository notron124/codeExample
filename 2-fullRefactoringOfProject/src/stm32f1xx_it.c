/**
  ******************************************************************************
  * @file    stm32f1xx_it.c 
  * @author  MCD Application Team
  * @version V1.0.0
  * @date    11-February-2014
  * @brief   Main Interrupt Service Routines.
  *          This file provides template for all exceptions handler and 
  *          peripherals interrupt service routine.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT 2014 STMicroelectronics</center></h2>
  *
  * Licensed under MCD-ST Liberty SW License Agreement V2, (the "License");
  * You may not use this file except in compliance with the License.
  * You may obtain a copy of the License at:
  *
  *        http://www.st.com/software_license_agreement_liberty_v2
  *
  * Unless required by applicable law or agreed to in writing, software 
  * distributed under the License is distributed on an "AS IS" BASIS, 
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "stm32f1xx_it.h"
#include "timers.h"
#include "sensors.h"
#include "deadband.h"
#include "debug.h"
#include "lcd1602.h"
#include "keyapi.h"

/** @addtogroup IO_Toggle
  * @{
  */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/******************************************************************************/
/*            Cortex-M Processor Exceptions Handlers                          */
/******************************************************************************/

/**
  * @brief  This function handles NMI exception.
  * @param  None
  * @retval None
  */
void NMI_Handler(void)
{
}

/**
  * @brief  This function handles Hard Fault exception.
  * @param  None
  * @retval None
  */
void HardFault_Handler(void)
{
  /* Go to infinite loop when Hard Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Memory Manage exception.
  * @param  None
  * @retval None
  */
void MemManage_Handler(void)
{
  /* Go to infinite loop when Memory Manage exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Bus Fault exception.
  * @param  None
  * @retval None
  */
void BusFault_Handler(void)
{
  /* Go to infinite loop when Bus Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Usage Fault exception.
  * @param  None
  * @retval None
  */
void UsageFault_Handler(void)
{
  /* Go to infinite loop when Usage Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles SVCall exception.
  * @param  None
  * @retval None
  */
void SVC_Handler(void)
{
}

/**
  * @brief  This function handles Debug Monitor exception.
  * @param  None
  * @retval None
  */
void DebugMon_Handler(void)
{
}

/**
  * @brief  This function handles PendSVC exception.
  * @param  None
  * @retval None
  */
void PendSV_Handler(void)
{
}



/**
  * @brief  This function handles SysTick Handler.
  * @param  None
  * @retval None
  */
void SysTick_Handler(void)
{
   timers.flags.bits._1khz = true;

   if (timers.cntr100Hz++ >= 10)
   {
   	timers.cntr100Hz = 0;
   	timers.flags.bits._100hz = true;
   }

   if (timers.cntr25Hz++ >= 40)
   {
      timers.cntr25Hz = 0;
      timers.flags.bits._25hz = true;
   }

   if (timers.cntrSecond++ >= 1000)
   {
      timers.cntrSecond = 0;
      timers.flags.bits.second = true;
   }

   if (keys[MENU_KEY].flags.bits.pressed || keys[DOWN_KEY].flags.bits.pressed ||
       keys[UP_KEY].flags.bits.pressed || keys[OK_KEY].flags.bits.pressed)
   {
      timers.blinkCntr = 0;
      RESET_BLINK_PARAM;
   }

   if (timers.blinkCntr++ >= 1000)
   {
      timers.blinkCntr = 0;
      TGL_BLINK_PARAM;
   }
}

/******************************************************************************/
/*                 STM32F1xx Peripherals Interrupt Handlers                   */
/*  Add here the Interrupt Handler for the used peripheral(s) (PPP), for the  */
/*  available peripheral interrupt handler's name please refer to the startup */
/*  file (startup_stm32f10x_md.s).                                            */
/******************************************************************************/
void ADC1_2_IRQHandler(void)
{
   if ((ADC1->SR & ADC_SR_JEOC) == ADC_SR_JEOC)
   {
      tRoom.averageValue = GetAverageValue(&tRoom.rawValueArr[0], &tRoom.cntr, DeadBand(&coldJuncFilter, COLD_JUNC_ADC_JDR));
      tCouple.averageValue = GetAverageValue(&tCouple.rawValueArr[0], &tCouple.cntr, DeadBand(&tCoupleFilter, T_COUPLE_ADC_JDR));
      pressureSens.averageValue = GetAverageValue(&pressureSens.rawValueArr[0], &pressureSens.cntr, DeadBand(&pressureFilter, PRESSURE_ADC_JDR));

      rawData = PRESSURE_ADC_JDR;

      ADC1->SR &= ~ADC_SR_JEOC;
   }
}

void USART1_IRQHandler(void)
{
  	if(USART_GetITStatus(USART1, USART_IT_TXE) != RESET)
	{
      SendData_IT(&dUsart);
	}
}


/**
  * @}
  */ 


/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
