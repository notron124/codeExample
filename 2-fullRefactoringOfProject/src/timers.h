#ifndef TIMERS_H_
#define TIMERS_H_

enum FlagsStates 
{
   false = (uint8_t)0,
   true = (uint8_t)1,
};


/* Struct of flags, can be value of FlagStates */
typedef struct 
{
   uint8_t _1khz : 1;
   uint8_t _100hz : 1;
   uint8_t _25hz : 1;
   uint8_t second : 1;
   uint8_t dummy : 4;
} Bits;

typedef struct
{
   uint16_t cntrSecond;
   uint16_t cntr100Hz;
   uint16_t cntr25Hz;
   uint16_t blinkCntr;

   union flags
   {
      Bits bits;
      uint8_t flagsHolder;
   } flags;
   
} Timers;

Timers timers;

// vu8 timersFlags;
// #define F_1KHZ    0x01
// #define F_100HZ   0x02
// #define F_25HZ    0x04
// #define F_SECOND  0x08

// #define SET_1KHZ     (timersFlags |= F_1KHZ)
// #define RESET_1KHZ   (timersFlags &= ~F_1KHZ)
// #define _1KHZ        (timersFlags & F_1KHZ)


#endif
