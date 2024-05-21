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
#include "asyncEngine.h"

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
}

/******************************************************************************/
/*                 STM32F1xx Peripherals Interrupt Handlers                   */
/*  Add here the Interrupt Handler for the used peripheral(s) (PPP), for the  */
/*  available peripheral interrupt handler's name please refer to the startup */
/*  file (startup_stm32f10x_md.s).                                            */
/******************************************************************************/


/* Table with PWM values that change sinusoidally.
 * Duty default at 50% and it changes up and down according to this table.
 * Current values usable only form 12 kHz PWM. Recalculate it if you want to change PWM frequency.
 * Formula for calculations: (SystemCoreClock / (PWM_FREQUENCY * 2000)) * (sin(0-360degree)*1000).
 */
volatile int16_t sineTable[SINE_TABLE_SIZE] = {
   0, 52, 105, 157, 209, 261, 314, 366, 418, 469, 521, 572, 624, 675, 726, 776, 827, 877, 927, 977, 1026,
   1075, 1124, 1172, 1220, 1268, 1315, 1362, 1408, 1454, 1500, 1545, 1590, 1634, 1678, 1721, 1763, 1805, 1847, 1888, 1928,
   1968, 2007, 2046, 2084, 2121, 2158, 2194, 2229, 2264, 2298, 2331, 2364, 2396, 2427, 2457, 2487, 2516, 2544, 2572, 2598,
   2624, 2649, 2673, 2696, 2719, 2741, 2762, 2782, 2801, 2819, 2837, 2853, 2869, 2884, 2898, 2911, 2923, 2934, 2945, 2954,
   2963, 2971, 2978, 2984, 2989, 2993, 2996, 2998, 3000, 3000, 3000, 2998, 2996, 2993, 2989, 2984, 2978, 2971, 2963, 2954,
   2945, 2934, 2923, 2911, 2898, 2884, 2869, 2853, 2837, 2819, 2801, 2782, 2762, 2741, 2719, 2696, 2673, 2649, 2624, 2598,
   2572, 2544, 2516, 2487, 2457, 2427, 2396, 2364, 2331, 2298, 2264, 2229, 2194, 2158, 2121, 2084, 2046, 2007, 1968, 1928,
   1888, 1847, 1805, 1763, 1721, 1678, 1634, 1590, 1545, 1500, 1454, 1408, 1362, 1315, 1268, 1220, 1172, 1124, 1075, 1026,
   977, 927, 877, 827, 776, 726, 675, 624, 572, 521, 469, 418, 366, 314, 261, 209, 157, 105, 52, 0,
   -52, -105, -157, -209, -261, -314, -366, -418, -469, -521, -572, -624, -675, -726, -776, -827, -877, -927, -977, -1026,
   -1075, -1124, -1172, -1220, -1268, -1315, -1362, -1408, -1454, -1500, -1545, -1590, -1634, -1678, -1721, -1763, -1805, -1847, -1888, -1928,
   -1968, -2007, -2046, -2084, -2121, -2158, -2194, -2229, -2264, -2298, -2331, -2364, -2396, -2427, -2457, -2487, -2516, -2544, -2572, -2598,
   -2624, -2649, -2673, -2696, -2719, -2741, -2762, -2782, -2801, -2819, -2837, -2853, -2869, -2884, -2898, -2911, -2923, -2934, -2945, -2954,
   -2963, -2971, -2978, -2984, -2989, -2993, -2996, -2998, -3000, -3000, -3000, -2998, -2996, -2993, -2989, -2984, -2978, -2971, -2963, -2954,
   -2945, -2934, -2923, -2911, -2898, -2884, -2869, -2853, -2837, -2819, -2801, -2782, -2762, -2741, -2719, -2696, -2673, -2649, -2624, -2598,
   -2572, -2544, -2516, -2487, -2457, -2427, -2396, -2364, -2331, -2298, -2264, -2229, -2194, -2158, -2121, -2084, -2046, -2007, -1968, -1928,
   -1888, -1847, -1805, -1763, -1721, -1678, -1634, -1590, -1545, -1500, -1454, -1408, -1362, -1315, -1268, -1220, -1172, -1124, -1075, -1026,
   -977, -927, -877, -827, -776, -726, -675, -624, -572, -521, -469, -418, -366, -314, -261, -209, -157, -105, -52,
};



void TIM1_UP_IRQHandler(void)
{
   TIM1->SR &= ~TIM_SR_UIF;

   relativeTime += 1;

   delay -= delay > 0 ? 1 * globalSpeedChangeCoeff : 0;

   if (delay > 10000)
      delay = 0;

   pointer = (pointerChangeSpeedCoef * current_frequency * relativeTime / 100) % SINE_TABLE_SIZE; // Calculate current position of the pointer

   /* If we are at zero in the sine table and desired_frequency differs from current_frequency, 
    * then check in witch zero we are.
    * 
    * If we are at positive half-wave of sine, then just zero everything out.
    * 
    * If we are at negative half-wave of sine, then pointer should be 180 (index of zero of negative half-wave)
    * and we should calculate raltiveTime, knowing formula for pointer, so next pinter would be calculated correctly.
    * 
    * Note: if we detect just positive half-wave to avoid calculations, then there is not smooth frequency transition after roughly 4k RPM.
    * 
    *  */
   if (sineTable[pointer] == 0)
   {
      /* This is to avoid variable overflow */
      if (relativeTime >= 12000)
      {
         if (sineTable[pointer + 1] > 0)
         {
            relativeTime = 0;
            pointer = 0;
         }
         else if (sineTable[pointer + 1] < 0)
         {
            pointer = 180;
            relativeTime = (pointer * 100) / (pointerChangeSpeedCoef * current_frequency);
         }
      }

      if (desired_frequency != current_frequency && !delay)
      {
         static int32_t delta;
         delta = absolut(desired_frequency - current_frequency);

         relativeTime = 0;

         if (sineTable[pointer + 1] > 0)
         {
            relativeTime = 0;
            pointer = 0;
         }
         else if (sineTable[pointer + 1] < 0)
         {
            pointer = 180;
            relativeTime = (pointer * 100) / (pointerChangeSpeedCoef * current_frequency);
         }

         if (desired_frequency > current_frequency)
            delay = delta / rumpUpDivider + ACCELERATION_CONSTANT;
         else
            delay = delta * slowDownCoeff + DECELERATION_CONSTANT;

         current_frequency += desired_frequency > current_frequency ? 1 : -1;
      }
   }

   /* If current frequency more than 50 Hz break the U/F ratio */
   if (current_frequency > 50)
   {
      TIM1->CCR3 = timerARR / 2 + sineTable[pointer] * 9 / 10;
      TIM1->CCR2 = timerARR / 2 + sineTable[(pointer + 120) % SINE_TABLE_SIZE] * 9 / 10;
      TIM1->CCR1 = timerARR / 2 + sineTable[(pointer + 240) % SINE_TABLE_SIZE] * 9 / 10;
   }
   else /* Else keep the U/F ratio */
   {
      static int32_t voltToFreqRatio;
      voltToFreqRatio = current_frequency + ((50 - current_frequency) / VOLTAGE_CONSTANT);
      TIM1->CCR3 = timerARR / 2 + (sineTable[pointer] * voltToFreqRatio / 50) * 9 / 10;
      TIM1->CCR2 = timerARR / 2 + (sineTable[(pointer + 120) % SINE_TABLE_SIZE] * voltToFreqRatio / 50) * 9 / 10;
      TIM1->CCR1 = timerARR / 2 + (sineTable[(pointer + 240) % SINE_TABLE_SIZE] * voltToFreqRatio / 50) * 9 / 10;
   }
}

void TIM4_IRQHandler(void)
{
   ADC_SoftwareStartConvCmd(ADC1, ENABLE);
   TIM4->SR &= ~TIM_SR_UIF;

   SET_25HZ;
}

void ADC1_2_IRQHandler(void)
{
   if (++cntrTemperature >= TEMPERATURE_ARR_LENGTH)
      cntrTemperature = 0;

   if (++cntrSpeed >= SPEED_ARR_LENGTH)
      cntrSpeed = 0;

   speed[cntrSpeed] = adcToDma[0];

   temperature[cntrTemperature] = adcToDma[1];
}

void EXTI9_5_IRQHandler(void)
{
   if ((EXTI->PR & EXTI_PR_PR6) == EXTI_PR_PR6)
   {
      EXTI->PR |= EXTI_PR_PR6;
      TIM1->BDTR &= ~TIM_BDTR_MOE;
      SET_OVERCURRENT;
      overcurrent_delay = 0;
   }
}

/**
  * @brief  This function handles PPP interrupt request.
  * @param  None
  * @retval None
  */
/*void PPP_IRQHandler(void)
{
}*/

/**
  * @}
  */ 


/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
