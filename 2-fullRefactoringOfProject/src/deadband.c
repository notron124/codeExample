#include "stm32f10x.h"
#include "deadband.h"

DeadBandFilter NewDeadBandFilter(uint8_t numberOfLSBsToIngore)
{
   DeadBandFilter newFilter;
   newFilter.numberOfLSBsToIgnore = numberOfLSBsToIngore;
   newFilter.deadBandLowerEdge = 0;
   newFilter.previousValue = 0;
   return newFilter;
}

uint32_t DeadBand(DeadBandFilter *filter, uint32_t input)
{
   if (filter->numberOfLSBsToIgnore == 0)
      return input;

   uint32_t window = (1 << filter->numberOfLSBsToIgnore) - 1;
   uint32_t deadBandUpperEdge = filter->deadBandLowerEdge + window;

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

