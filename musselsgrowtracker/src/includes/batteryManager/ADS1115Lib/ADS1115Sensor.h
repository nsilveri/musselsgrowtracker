#ifndef ADS1115SENSOR_H
#define ADS1115SENSOR_H

#include "ADS1X15.h"

class ADS1115Sensor {
public:
    ADS1115Sensor(uint8_t address = 0x48);
    void begin();
    float readVoltage();
    
    
private:
    ADS1115 _ads;
    float _voltageFactor;
    uint8_t CHANNEL_0 = 0;
    int16_t readAverageRaw(int numReadings);
    int16_t readRaw();
};

extern ADS1115Sensor ads;

#endif // ADS1115SENSOR_H
