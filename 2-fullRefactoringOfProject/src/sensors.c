#include "stm32f10x.h"
#include "sensors.h"
#include "coupletable.h"

uint32_t GetAverageValue(uint32_t *rawValueArr, uint8_t *cntr, uint32_t rawValue)
{
   uint32_t averageValue = 0;

   if ((*cntr) >= AVERAGING_ARRAY_SIZE)
      (*cntr) = 0;

   rawValueArr[(*cntr)++] = rawValue;

   for (uint8_t i = 0; i < AVERAGING_ARRAY_SIZE; i ++)
      averageValue += rawValueArr[i];

   averageValue /= AVERAGING_ARRAY_SIZE;
   
   return averageValue;
}

uint32_t CalculateRoomTemperature(uint32_t samples, uint32_t offset)
{
   uint32_t roomTemperature;
#ifdef SMART
   roomTemperature = samples * 5) / 16; //- factor.w.bias_room;
#else
   // roomTemperature = (samples * 39) / 80; //- factor.w.bias_room;
   roomTemperature = (samples - Vof) / Tc;
   if (roomTemperature > 150)
      roomTemperature = 0;
#endif
   if (roomTemperature > offset)
      return roomTemperature - offset;
   else
      return roomTemperature;
}

uint32_t CalculateTemperature(uint32_t samples, uint32_t offset, uint16_t tCoupCoeff, uint32_t roomTemperature)
{
   u16 tCoupSamples, roomSamples;
   u32 BAX;

   if (samples > offset)
   {
      tCoupSamples = samples - offset;
      BAX = (u32)(tCoupSamples) * (u32)(tCoupCoeff);
      tCoupSamples = (BAX / 220); // MID_KFC);
      roomSamples = DegreeToSample(roomTemperature);
      return SampleToDegree(tCoupSamples + roomSamples);
   }
   else
      return roomTemperature;
}

#define PRESSURE_FILTER_WINDOW_SIZE 4  

uint32_t CalculatePressure(uint32_t samples, uint32_t oldPressure, uint32_t pressure_6atm, uint32_t pressure_2atm)
{
   int32_t K, B, M;
   uint32_t newPressure = oldPressure;

   static uint32_t sumPressure;
   static uint8_t cntrFltPr;


   // 60000 = 6atm, 20000 = 2atm
   K = (60000 - 20000) / (pressure_6atm - pressure_2atm); 
   B = 20000 - K * pressure_2atm;
   M = K * ((int32_t)samples + B);

   if (M >= 0)
      sumPressure += (u16)(M / 1000);

   if (++cntrFltPr >= PRESSURE_FILTER_WINDOW_SIZE)
   {
      cntrFltPr = 0;

      newPressure =  sumPressure / PRESSURE_FILTER_WINDOW_SIZE;
      sumPressure = 0;
   }

   return newPressure;
}
