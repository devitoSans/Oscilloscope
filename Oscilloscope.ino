#include "definition.h"
#include <Arduino.h>
#include <MemoryFree.h>

byte g_DECIMAL_POINTS = 3;

short g_BATCH_ANALOG_VALUES[MAX_BATCH_NUM];
unsigned long g_BATCH_TIME[MAX_BATCH_NUM];

byte g_DELAY_TYPE = MICRO;
unsigned long g_DELAY = 20; 

byte countDigits(long x);
byte mapToVoltage(long x, long& mappedValue, byte decimalPoints=3);
void changePrescaler(byte divisionFactor);

void setup() 
{
    Serial.begin(BAUDRATE);
    changePrescaler(8);
}

void loop() 
{
    long delayMultiplier = 0;

    // Configure delay's type
    if(g_DELAY_TYPE == SECOND)
    {
        delayMultiplier = 1e6;
    }
    else if(g_DELAY_TYPE == MILLI)
    {
        delayMultiplier = 1e3;
    }
    else if(g_DELAY_TYPE == MICRO)
    {
        delayMultiplier = 1;
    }

    unsigned long prevTime = 0, nextTime = 0;
    for(short i = 0; i < MAX_BATCH_NUM; i++)
    {
        g_BATCH_ANALOG_VALUES[i] = analogRead(A1);
        g_BATCH_TIME[i] = nextTime - prevTime;
        prevTime += g_BATCH_TIME[i];
        
        while(nextTime - prevTime < g_DELAY*delayMultiplier) 
        {
            nextTime = micros();
        }
    }

    const byte maxVoltageDigits PROGMEM = countDigits(MAX_VOLTAGE);
    for(short i = 0; i < MAX_BATCH_NUM; i++)
    {
        long voltageValue;
        byte desiredDigitNumber = (maxVoltageDigits + g_DECIMAL_POINTS); 
        byte numberOfDigits = mapToVoltage(g_BATCH_ANALOG_VALUES[i], voltageValue, g_DECIMAL_POINTS);

        // Add leading zero for voltage value below 1 V
        byte leadingZeroNumber = desiredDigitNumber - numberOfDigits;
        // Serial.println(leadingZeroNumber);
        if(leadingZeroNumber > 0)
        {
            for(byte i = 0; i < leadingZeroNumber; i++)
            {
                Serial.print('0');
            }
        }

        Serial.print(voltageValue);
        Serial.println(g_BATCH_TIME[i]);
    }
}

// Helper Functions

byte countDigits(long x)
{
    byte counter = 0;
    while(x > 0)
    {
        x /= 10;
        counter++;
    }
    return counter;
}

// Return number of voltage's digits.
// Maximum decimal points is 7, cannot guarantee safety beyond that.
byte mapToVoltage(long x, long& mappedValue, byte decimalPoints)
{
    mappedValue = (long)((x / MAX_ANALOG_INPUT * MAX_VOLTAGE) * ((long)pow(10, decimalPoints)));
    return countDigits(mappedValue); // without the dot/point
}

// Division Factor is one of them: 4, 8, 16, 32, 64, 128.
void changePrescaler(byte divisionFactor)
{
    ADCSRA &= ~(bit (ADPS0) | bit (ADPS1) | bit (ADPS2)); // clear prescaler bits
    ADCSRA |= PRESCALE_(divisionFactor);
}