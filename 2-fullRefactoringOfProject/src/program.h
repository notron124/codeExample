#ifndef PROGRAM_H_
#define PROGRAM_H_

#define MIN_PARAMETER   0
#define MAX_PARAMETER   6

enum Types
{
   AUTOMATIC = 0,
   SEMI_AUTOMATIC,
   MANUAL,
};

enum ParamNames
{
   P_NUMBER = 0,
   P_TYPE,
   P_TARGET_TEMPERATURE,
   P_HOLDING_TIME,
   P_FLOW,
   P_TARGET_PRESSURE,
   P_PRESSING_TIME,
};

typedef struct
{
   // original order of flags
   // uint16_t automatiMode : 1;
   // uint16_t pressAfterHeat : 1;
   // uint16_t flow : 1;
   // uint16_t heatOnly : 1;
   // uint16_t semiAutomatic : 1;
   // uint16_t flowTmp : 1;
   // uint16_t paramChanged : 1;
   uint16_t type : 2;
   uint16_t pressAfterHeat : 1;
   uint16_t heatOnly : 1;
   uint16_t flow : 1;
   uint16_t flowTmp : 1;
   uint16_t paramChanged : 1;
} p_bits;

typedef union
{
   p_bits bits;
   uint16_t flagsHolder;
} p_flags;


typedef struct
{
   uint16_t targetPressure;
   uint16_t targetTemperature;
   uint16_t holdingTime;
   uint16_t pressingTime;
} Parameters;

typedef struct
{
   Parameters parameters;
   p_flags flags;
} Program;



uint8_t paramNumber;
uint8_t programNumber;
Program programs[10];

extern const uint16_t minParameters[7];
extern const uint16_t maxParameters[7];

#endif
