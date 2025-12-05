#include "definition.h"
#include <Arduino.h>
#include <MemoryFree.h>

byte g_DECIMAL_POINTS = 3;
bool g_IS_VOLTAGE_MEASURED = true;

char g_BATCH_BUFFERS[MAX_BATCH_NUM * MAX_DECIMAL_POINTS + 2]; // +2 for measurement's type and null termination.
short g_BATCH_ANALOG_VALUES[MAX_BATCH_NUM];

byte g_DELAY_TYPE = MICRO;
unsigned long g_DELAY = 2;

byte countDigits(long x);
byte mapToVoltage(long x, long& mappedValue, byte decimalPoints=3);
void allocateBatchBuffers(char value, short& index);
void copyToBatchBuffers(const char* from, size_t size, short& index);
void changePrescaler(byte divisionFactor);

void setup() 
{
    Serial.begin(BAUDRATE);
    changePrescaler(16);
}

void loop() 
{
    short batchBuffersIndex = 0;
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

    // Configure reading's type
    if(g_IS_VOLTAGE_MEASURED)
    {
        allocateBatchBuffers('v', batchBuffersIndex);
    }
    else // CURRENT
    {
        allocateBatchBuffers('i', batchBuffersIndex);
    }

    for(short i = 0; i < MAX_BATCH_NUM; i++)
    {
        g_BATCH_ANALOG_VALUES[i] = analogRead(A1);
        delayMicroseconds(g_DELAY * delayMultiplier);
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
            char leadingZero[leadingZeroNumber+1]; // +1 for null termination
            for(byte i = 0; i < leadingZeroNumber; i++)
            {
                leadingZero[i] = '0';
            }
            leadingZero[leadingZeroNumber] = '\0';
            copyToBatchBuffers(leadingZero, sizeof(leadingZero), batchBuffersIndex);
        }

        // Convert long to string
        char buffer[numberOfDigits+1]; // +1 for null terminated
        snprintf(buffer, sizeof(buffer), "%ld", voltageValue);

        copyToBatchBuffers(buffer, sizeof(buffer), batchBuffersIndex);
    }
    allocateBatchBuffers('\0', batchBuffersIndex);

    Serial.println(g_BATCH_BUFFERS);
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

void allocateBatchBuffers(char value, short& index)
{
    g_BATCH_BUFFERS[index] = value;
    index++;
}

// size includes the null termination.
void copyToBatchBuffers(const char* from, size_t size, short& index)
{
    for(size_t i = 0; i < size-1; i++) // -1 to exclude null termination
    {
        allocateBatchBuffers(from[i], index);
    }
}

// Division Factor is one of them: 4, 8, 16, 32, 64, 128.
void changePrescaler(byte divisionFactor)
{
    ADCSRA &= ~(bit (ADPS0) | bit (ADPS1) | bit (ADPS2)); // clear prescaler bits
    ADCSRA |= PRESCALE_(divisionFactor);
}