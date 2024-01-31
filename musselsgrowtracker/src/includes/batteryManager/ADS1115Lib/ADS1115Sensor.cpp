#include "ADS1115Sensor.h"
#include <Arduino.h>

ADS1115Sensor::ADS1115Sensor(uint8_t address) : _ads(address) {}

void ADS1115Sensor::begin() {
    _ads.begin();
    _ads.setGain(1);
    _voltageFactor = _ads.toVoltage(1);
}

int16_t ADS1115Sensor::readAverageRaw(int numReadings) {
    long sum = 0;
    for (int i = 0; i < numReadings; i++) {
        sum += readRaw();  // Assumendo che readRaw() sia la tua funzione per leggere il valore grezzo
        delay(10);  // Piccola pausa tra le letture per stabilità
    }
    return (int16_t)(sum / numReadings);
}

float ADS1115Sensor::readVoltage() {
    int16_t raw = readAverageRaw(10);//readRaw();
    Serial.print("raw: ");Serial.println(raw); //27263
    int16_t milliVolt = map(raw, 26468, 28740, 3200, 4200);
    Serial.println(milliVolt);
    return milliVolt;// * _voltageFactor;
}

int16_t ADS1115Sensor::readRaw() {
    return _ads.readADC(CHANNEL_0);
}

ADS1115Sensor ads;