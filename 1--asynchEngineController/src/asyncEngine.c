#include "stm32f10x.h"
#include "asyncEngine.h"
#include "deadband.h"

uint32_t absolut(int32_t a)
{
   return a >= 0 ? a : -a;
}

void CalculateAverageADCValue(volatile int *values, int arraySize, volatile int *averageValueHolder)
{
   *averageValueHolder = 0;
   for (uint8_t c = 0; c < arraySize; c++)
      *averageValueHolder += values[c];

   *averageValueHolder /= arraySize;
}

static void WorkFromPotentiometer(void)
{
   static DeadBandFilter filter;
   filter.numberOfLSBsToIgnore = 4;

   /* Hold frequency if switch is off or overcurrent/heat apears */
   if ((GPIOB->IDR & GPIO_IDR_IDR6) || OVERCURRENT)
   {
      desired_frequency = 9;
   }
   else // else calculate and limit desired_frequency
   {
      averageADCSpeed = DeadBand(&filter, averageADCSpeed);

      desired_frequency = 9 + ((averageADCSpeed + 5) * 91) / 0xFFF; // set new desired_frequency

      desired_frequency < 9 ? desired_frequency = 9 : desired_frequency;
      desired_frequency > 99 ? desired_frequency = 100 : desired_frequency;
   }
}

static void WorkFromSwitch(void)
{
   if (averageADCSpeed > 3687)
   {
      if (optimum_mode != 1)
      {
         optimum_mode = 1;
         desired_frequency = 50;
         TIM1->BDTR |= TIM_BDTR_MOE;
         NVIC_EnableIRQ(TIM1_UP_IRQn);
      }
   }
   else if (averageADCSpeed > 409)
   {
      if (current_frequency > 10)
      {
         optimum_mode = 3;
         desired_frequency = 9;
      }
      else
      {
         if (optimum_mode != 0)
         {
            optimum_mode = 0;
            TIM1->BDTR &= ~TIM_BDTR_MOE;
            NVIC_DisableIRQ(TIM1_UP_IRQn);
         }
      }
   }
   else
   {
      if (optimum_mode != 2)
      {
         optimum_mode = 2;
         desired_frequency = 25;
         TIM1->BDTR |= TIM_BDTR_MOE;
         NVIC_EnableIRQ(TIM1_UP_IRQn);
      }
   }
}

void CalculateDesiredFrequency(void)
{
   switch (working_mode)
   {
   case 0:
      WorkFromPotentiometer();
      break;

   case 1:
      WorkFromSwitch();
      break;
   }
}

void OverheatProtection(void)
{
   if (averageADCTemperature > TRIGGER_OVERHEAT_TEMPERATURE && !OVERHEAT)
   {
      TIM1->BDTR &= ~TIM_BDTR_MOE;
      SET_OVERHEAT;
   }
   else if (averageADCTemperature < RELEASE_OVERHEAT_TEMPERATURE && OVERHEAT)
   {
      RESET_OVERHEAT;
      current_frequency = 9;
      desired_frequency = 10;

      if (!working_mode)
      {
         TIM1->BDTR |= TIM_BDTR_MOE;
         NVIC_EnableIRQ(TIM1_UP_IRQn);
      }
      else
         optimum_mode = 0;
   }
}

void StartEngine(void)
{
   if ((!(GPIOB->IDR & GPIO_IDR_IDR6)) || (working_mode == 1))
   {
      cntr_motor_off = 0;

      if (!(TIM1->BDTR & TIM_BDTR_MOE))
      {
         if (OVERCURRENT)
         {
            if (++overcurrent_delay > 25 * OVERCURRENT_DELAY) // 5
            {
               overcurrent_delay = 0;
               RESET_OVERCURRENT;
               current_frequency = 9;
               desired_frequency = 10;

               if (!working_mode)
               {
                  TIM1->BDTR |= TIM_BDTR_MOE;
                  NVIC_EnableIRQ(TIM1_UP_IRQn);
               }
               else
                  optimum_mode = 0;
            }
         }
         else
         {
            if (++cntr_motor_on > 25)
            {
               cntr_motor_on = 0;

               if (!working_mode)
               {
                  TIM1->BDTR |= TIM_BDTR_MOE;
                  NVIC_EnableIRQ(TIM1_UP_IRQn);
               }
            }
         }
      }
   }
   else
   {
      cntr_motor_on = 0;

      if ((TIM1->BDTR & TIM_BDTR_MOE))
      {
         if (++cntr_motor_off > 25)
         {
            cntr_motor_off = 0;

            if (current_frequency > 10)
               desired_frequency = 9;
            else
            {
               TIM1->BDTR &= ~TIM_BDTR_MOE;

               NVIC_DisableIRQ(TIM1_UP_IRQn);
               overcurrent_delay = 0;
               RESET_OVERCURRENT;
            }
         }
      }
   }
}
