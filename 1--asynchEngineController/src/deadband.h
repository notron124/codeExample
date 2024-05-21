#ifndef DEADBAND_H_
#define DEADBAND_H_

struct _DeadBandFilter {
   /* LSB - least significant bit */
   uint8_t numberOfLSBsToIgnore;
   uint16_t deadBandLowerEdge;
   uint16_t previousValue;
};

typedef struct _DeadBandFilter DeadBandFilter;

void InitDeadBandFilter(DeadBandFilter *filter, uint8_t numberOfLSBsToIngore);
uint16_t DeadBand(DeadBandFilter* filter, uint16_t input);

#endif
