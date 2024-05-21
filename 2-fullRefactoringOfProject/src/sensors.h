#ifndef SENSORS_H_
#define SENSORS_H_

#define AVERAGING_ARRAY_SIZE 25

typedef struct
{
   uint32_t rawValueArr[AVERAGING_ARRAY_SIZE];
   uint32_t averageValue;
   int32_t convertedValue;
   uint8_t cntr;
} Sensor;


#define COLD_JUNC_ADC_JDR     (ADC1->JDR1)
#define T_COUPLE_ADC_JDR      (ADC1->JDR2)
#define PRESSURE_ADC_JDR      (ADC1->JDR3)

#define ADC_BIT_DEPTH         12  // for stm32f10x
#define REFERENCE_VOLTAGE     3300000     // in uV
#define uV_IN_SAMPLE    ((REFERENCE_VOLTAGE) / (1 << ADC_BIT_DEPTH))

/* NS235 parameters for -40-100°C
   Note: In datasheet it is in mV, we are working in uV to get more accuracy without using float
*/
#define Tc     10 * 1000 
#define Vof    500 * 1000

/* Here we are converting uV to samples, to avoid unnescessary caclulations further
   Formula: samples = Voltage / (Voltage in one sample*)
   *Refer to iV_IN_SAMPLE
*/
#define Tc_Samples     (Tc / uV_IN_SAMPLE)
#define Vof_Samples    (Vof / uV_IN_SAMPLE)


uint32_t GetAverageValue(uint32_t *rawValueArr, uint8_t *cntr, uint32_t rawValue);
uint32_t CalculateRoomTemperature(uint32_t samples, uint32_t offset);
uint32_t CalculateTemperature(uint32_t samples, uint32_t offset, uint16_t tCoeff, uint32_t roomTemperature);
uint32_t CalculatePressure(uint32_t samples, uint32_t pressureSens, uint32_t pressure_6atm, uint32_t pressure_2atm);

Sensor tRoom, tCouple, pressureSens;

#endif
