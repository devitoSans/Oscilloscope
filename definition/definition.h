#ifndef DEFINITION
#define DEFINITION

#define DEBUG_MODE 0

#if DEBUG_MODE
    #define DEBUG(X) Serial.println(X)
#else
    #define DEBUG(X)
#endif

// Static Config
// Must restart the whole program
// to change it
#include "maxBatchNumDef.h"
#include "baudrateDef.h"

#include <math.h>

// Will be defined later on ATMega328P, I think
#ifndef ADCSRA && ADPS0 && ADPS1 && ADPS2
    #define ADCSRA  (*(volatile uint8_t *)(0x08))
    #define ADPS0   0
    #define ADPS1   1
    #define ADPS2   2
#endif

// This program only accounts for up to 9 Volt.
// But Arduino AVR UNO's max is 5 volt 
#define MAX_VOLTAGE 5.00
#define MAX_ANALOG_INPUT 1023.0
// Based on mapToVoltage()
#define MAX_DECIMAL_POINTS 7

// ADC Prescaler for ATMega328P (Arduino AVR UNO's Microcontroller)
#define PRESCALE_(X) ((byte)(log(X) / log(2)))

// Delay's type (i.e. second, milli, micro)
#define SECOND 0
#define MILLI 1
#define MICRO 2

// Dynamic Configuration's
#define PRESCALER_CONFIG 0
#define DELAY_CONFIG 1
#define DELAY_TYPE_CONFIG 2
#define DECIMAL_POINTS_CONFIG 3
#define END_CONFIG_CHANGE 127

#endif