#include "definition/definition.h"
#include <Arduino.h>
#include <MemoryFree.h>

// Dynamic Config 
// (Can be change by the user during run time)
byte g_DECIMAL_POINTS = 3;
byte g_DELAY_TYPE = MICRO;
unsigned long g_DELAY = 20;

short g_BATCH_ANALOG_VALUES[MAX_BATCH_NUM];
unsigned long g_BATCH_TIME[MAX_BATCH_NUM];

unsigned long getTime();
byte countDigits(long x);
byte mapToVoltage(long x, long& mappedValue, byte decimalPoints=3);
void changePrescaler(byte divisionFactor);
bool readInput(byte& configType, unsigned long long& value);
void setting();

void setup() 
{
    Serial.begin(BAUDRATE);
    changePrescaler(16);
}

void loop() 
{
    // Change dynamic configuration based on user's preferences
    setting();

    unsigned long prevTime = 0, nextTime = 0;
    for(short i = 0; i < MAX_BATCH_NUM; i++)
    {
        g_BATCH_TIME[i] = nextTime - prevTime;
        prevTime += g_BATCH_TIME[i];
        g_BATCH_ANALOG_VALUES[i] = analogRead(A1);
        
        while(nextTime - prevTime < g_DELAY+1) 
        {
            nextTime = getTime();
        }
    }

    const byte maxVoltageDigits = countDigits(MAX_VOLTAGE);
    for(short i = 0; i < MAX_BATCH_NUM; i++)
    {
        long voltageValue;
        byte desiredDigitNumber = (maxVoltageDigits + g_DECIMAL_POINTS); 
        byte numberOfDigits = mapToVoltage(g_BATCH_ANALOG_VALUES[i], voltageValue, g_DECIMAL_POINTS);

        // Add leading zero for voltage value below 1 V
        byte leadingZeroNumber = desiredDigitNumber - numberOfDigits;
        if(leadingZeroNumber > 0)
        {
            for(byte i = 0; i < leadingZeroNumber; i++)
            {
                #if DEBUG_MODE == 0
                    Serial.print('0');
                #endif
            }
        }
        #if DEBUG_MODE == 0
            Serial.print(voltageValue);
            Serial.println(g_BATCH_TIME[i]);
        #endif
    }
}

// Helper Functions

// The units depends on g_DELAY_TYPE
unsigned long getTime()
{
    if(g_DELAY_TYPE == MILLI)
    {
        return millis();
    }
    if(g_DELAY_TYPE == MICRO)
    {
        return micros();
    }
    return millis() / 1000; // defaulting to second
}

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
    mappedValue = (long)((float)x / (float)MAX_ANALOG_INPUT * MAX_VOLTAGE) * ((long)pow(10, decimalPoints)));
    return countDigits(mappedValue); // without the dot/point
}

// Division Factor is one of them: 4, 8, 16, 32, 64, 128.
void changePrescaler(byte divisionFactor)
{
    ADCSRA &= ~(bit (ADPS0) | bit (ADPS1) | bit (ADPS2)); // clear prescaler bits
    ADCSRA |= PRESCALE_(divisionFactor);
}

bool readInput(byte& configType, unsigned long long& value)
{
    int readByte = Serial.read();
    if(readByte == -1)
    {
        return false;
    }
    
    configType = readByte;
    
    // Reading the config's value
    // Only supports UTF-8 Formatter
    readByte = Serial.read();

    byte valueLength = 1; value = 0;
    while(readByte != END_CONFIG_CHANGE && readByte != -1) // end of the 1 configuration change, or no input anymore
    {
        if(value != 0)
        {
            value *= 10;
            valueLength++;
        }
        if(valueLength+1 <= 17) // avoiding overflow
        {
            value += readByte-int('0');
        }
        readByte = Serial.read();
        
    }

    return true;
}

void setting()
{
    byte configType;
    unsigned long long value;

    while(readInput(configType, value))
    {
        switch (configType)
        {
            case PRESCALER_CONFIG:
                changePrescaler(value);
                break;

            case DELAY_CONFIG:
                g_DELAY = value;
                break;

            case DELAY_TYPE_CONFIG:
                g_DELAY_TYPE = value;
                break;
                
            case DECIMAL_POINTS_CONFIG:
                g_DECIMAL_POINTS = value;
                break;
        }
    }
}