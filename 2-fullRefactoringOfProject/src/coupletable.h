#ifndef COUPLE_TABLE_H_
#define COUPLE_TABLE_H_

// Defined by hardware
#define COUP_SIGNAL_MULTIPLICATION  256

#define MAX_TEMPERATURE 450

void InitSamplesLookUpTable(void);
uint32_t DegreeToSample(uint32_t temperature);
uint32_t SampleToDegree(uint32_t sample);

#endif
