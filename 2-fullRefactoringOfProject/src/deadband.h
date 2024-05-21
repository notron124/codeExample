#ifndef DEADBAND_H_
#define DEADBAND_H_

typedef struct {
   /* LSB - least significant bit */
   uint8_t numberOfLSBsToIgnore;
   uint16_t deadBandLowerEdge;
   uint16_t previousValue;
} DeadBandFilter;

DeadBandFilter NewDeadBandFilter(uint8_t numberOfLSBsToIngore);
uint32_t DeadBand(DeadBandFilter *filter, uint32_t input);

DeadBandFilter coldJuncFilter;
DeadBandFilter tCoupleFilter;
DeadBandFilter pressureFilter;

#endif
