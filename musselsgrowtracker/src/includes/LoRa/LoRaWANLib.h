// LoRaWANLib.h
#ifndef LoRaWANLib_h
#define LoRaWANLib_h

#include "Arduino.h"
#include "library\_LoRaWAN.h"

class LoRaWANLib {
public:
    LoRaWANLib(const char *appEui, const char *appKey, const char *devEui);
    void begin();
    void getInfo();
    void sendMessage(int serialID, unsigned long timestamp, long hx711Value, byte systemState);

private:
    const char *_appEui;
    const char *_appKey;
    const char *_devEui;
};

extern LoRaWANLib LoRaWANManager;

#endif
