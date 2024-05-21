#include "stm32f10x.h"
#include "deadband.h"

void InitDeadBandFilter(DeadBandFilter* filter, uint8_t numberOfLSBsToIngore)
{
   filter->numberOfLSBsToIgnore = numberOfLSBsToIngore;
   filter->deadBandLowerEdge = 0;
   filter->previousValue = 0;
}

uint16_t DeadBand(DeadBandFilter* filter, uint16_t input)
{
   if (filter->numberOfLSBsToIgnore == 0)
      return input;

   uint16_t window = (1 << filter->numberOfLSBsToIgnore) - 1;
   uint16_t deadBandUpperEdge = filter->deadBandLowerEdge + window;

   if (input > deadBandUpperEdge)
   {
      deadBandUpperEdge = input;
      filter->deadBandLowerEdge = deadBandUpperEdge - window;
      filter->previousValue = input;
      return deadBandUpperEdge;
   }
   else if (input < filter->deadBandLowerEdge)
   {
      filter->deadBandLowerEdge = input;
      filter->previousValue = input;
      return filter->deadBandLowerEdge;
   }
   else
      return filter->previousValue;
}

