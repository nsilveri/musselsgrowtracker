#include "batteryMan.h"
#include "..\utilities\utilities.h"
#include <STM32L0.h>

//#define ADS1115_ENABLED

#ifdef ADS1115_ENABLED
void powerManagement::begin() {
    ads.begin();
}
#endif

float powerManagement::get_VDDA()
{
  float VDDA = STM32L0.getVDDA();
  log("VDDA= " + String(VDDA), 1);
  return VDDA;
}

uint8_t powerManagement::get_BatPercent()
{

    uint8_t batt = get_VBAT();
    if(batt > 3) {
        log("Bat= " + String(batteryPercentage) + "%", 1);
        return batt;
    }else {
        log("Bat= No battery detected!", 1);
        return BATTERY_ERROR_CODE;
    }
}

float powerManagement::get_VBAT()
{
#ifdef ADS1115_ENABLED
    float battRead = STM32L0.getVBAT();
    log("internal read: " + String(battRead), 2);
    if(battRead > 3) {
        batteryPercentage = map(battRead, 3.2, 4.2, 0, 100);
        return battRead;
    }else if(battRead < 3){
        mosfetSwitch.turn_on();
        battRead = ads.readVoltage();
        batteryPercentage = map(battRead, 3200, 4200, 0, 100);
        log("ADS1115 read: " + String(battRead), 2);
        if(battRead > 3) {
            return battRead;
        }
    }
#endif

#ifndef ADS1115_ENABLED
    float battRead = STM32L0.getVBAT();
        log("internal read: " + String(battRead), 2);
        if(battRead > 3) {
            batteryPercentage = map(battRead, 3.2, 4.2, 0, 100);
            return battRead;
        }
#endif
}

powerManagement pwrMan;