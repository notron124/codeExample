#include "stm32f10x.h"
#include "program.h"

const uint16_t minParameters[7] = {
    0,   // program number
    0,   // program type
    100, // target temperature
    1,   // holding time
    0,   // flow_tmp flag
    5,   // target pressure
    1    // pressing time
};

const uint16_t maxParameters[7] = {
    9,   // program number
    2,   // program type
    400, // target temperature
    30,  // holding time
    1,   // flow_tmp flag
    60,  // target pressure
    30   // pressing time
};
