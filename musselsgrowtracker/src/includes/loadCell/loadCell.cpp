#include "loadCell.h"
#include "hx711_basic.h"
//#include <Wire.h>

bool MOSFET_STATE = true;
float weightKnown = 1000;
float scaleFactorDefault = 43.34;


void LoadCell::begin()
{
    pinMode(MOSFET_PIN, OUTPUT);
    digitalWrite(MOSFET_PIN, MOSFET_STATE);
}

bool LoadCell::turn_on()
{   
    MOSFET_STATE = true;
    digitalWrite(MOSFET_PIN, MOSFET_STATE);
    return MOSFET_STATE;
}

bool LoadCell::turn_on_off()
{   
    MOSFET_STATE = !MOSFET_STATE;
    digitalWrite(MOSFET_PIN, MOSFET_STATE);
    return MOSFET_STATE;
}

bool LoadCell::turn_off()
{
    MOSFET_STATE = false;
    digitalWrite(MOSFET_PIN, MOSFET_STATE);
    return MOSFET_STATE;
}

void LoadCell::setOffset()
{
    loadCellADC.setOffset();
}

void LoadCell::calibrate(float weightKnown)
{
    loadCellADC.calibrate(weightKnown);
}

void LoadCell::setScale(float weightKnown)
{
    loadCellADC.setScale(weightKnown);
}

void LoadCell::read_weight()
{   
    loadCellADC.begin();
    loadCellADC.printTare();
    delay(500);
    loadCellADC.getWeight();
    //loadCellADC.readAverage(20);
}



LoadCell loadCell;