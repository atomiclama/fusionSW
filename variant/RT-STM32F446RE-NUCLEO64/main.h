#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#define USE_DEBUG_RADIO


#include "DigitalOut.h"
#include "spi.h"
#include "sx126x.h"

extern DigitalOut led;

extern SX126x radio1Hw;  
extern SX126x radio2Hw;

extern DigitalOut radio1Nss;
extern DigitalIn  radio1Busy;

extern DigitalOut radio2Nss;
extern DigitalIn  radio2Busy;

#ifdef __cplusplus
}
#endif
