#ifndef BATTERY_MAN_H
#define BATTERY_MAN_H

#include <STM32L0.h>
#include "ADS1115Lib\ADS1115Sensor.h"

class powerManagement {
  public:
        void begin();
        float get_VDDA();
        uint8_t get_BatPercent();
        float get_VBAT();

    private:
        //float get_int_VBAT();
        float batt = 0.00;
        uint8_t batteryPercentage = 0;
        uint8_t BATTERY_ERROR_CODE = 150;
};

extern powerManagement pwrMan;

#endif /* BATTERY_MAN_H */
