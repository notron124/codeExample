#ifndef _KEYAPI_H
#define _KEYAPI_H

#include "stm32f10x.h" // REPLACE WITH YOUR MAIN HEADER (stm8s.h, stm32f10x.h, ..)

#define _32_B_

enum ActiveLevels {
   ACTIVE_LOW = 0,
   ACTIVE_HIGH
};

typedef struct
{
   uint8_t shortPress; // ID for short press event
   uint8_t longPressEvent; // ID for long press event
   uint8_t multiClick; // ID for multi click event (2 or more clicks in some period)
} Key_IDs;

typedef struct
{
   uint32_t pressTime; // counter for press time, used to determine time press milis
   uint32_t holdTime;  // counter to track for how long was holded
   uint16_t betweenClicks; // counter for time between presses
   uint8_t clicks; // clicks counter
   
} Key_Cntrs;

typedef struct
{
   uint16_t shortPress; // necessary delay to call a short press event
   uint16_t longPressEvent; // necessary delay to call a long press event

   // maximum delay between clicks, if delay exeeds this, clicks counter resets to zero
   uint16_t betweenClicks; 

   // delay between repeats, when autorepeat flag is 1
   uint16_t betweenRepeats;
} Key_Delays;

typedef struct
{
   uint8_t pressed : 1;
   uint8_t autorepeat : 1;
   uint8_t longPressEvent : 1;
   uint8_t activeLevel : 1;
   uint8_t multiClickEvent : 1;
   uint8_t dummy : 3;
} kFlags;

typedef union
{
   kFlags bits;
   uint8_t fl;
} Key_Flags;

typedef struct 
{
   GPIO_TypeDef *GPIOx; // GPIO bus to which button is connected

#ifdef _32_B_
   uint16_t pin; // Pin to which button is connected
#else
   uint8_t pin; // Pin to which button is connected
#endif
   Key_Cntrs counters;
   Key_IDs IDs;
   Key_Delays delays;
   Key_Flags flags;
} Key_TypeDef;

void InitKey(Key_TypeDef *key, GPIO_TypeDef *GPIOx, uint16_t pin, Key_Delays delays, Key_IDs IDs, Key_Flags flags);
int8_t HandleKey(Key_TypeDef *key);

/* User defined keys */
#define KEYS_AMOUNT 4

enum keys
{
   MENU_KEY = 0,
   DOWN_KEY,
   UP_KEY,
   OK_KEY,
};

Key_TypeDef keys[KEYS_AMOUNT];
int8_t keyCode;
/* ----------------- */

#endif /* _KEYAPI_H */
