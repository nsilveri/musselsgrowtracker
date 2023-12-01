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
    loadCellADC.begin();
}

bool LoadCell::mosfet_state_change(bool STATE)
{
    digitalWrite(MOSFET_PIN, STATE);
    return STATE;
}

bool LoadCell::turn_on()
{   
    MOSFET_STATE = true;
    mosfet_state_change(MOSFET_STATE);
}

bool LoadCell::turn_on_off()
{   
    MOSFET_STATE = !MOSFET_STATE;
    mosfet_state_change(MOSFET_STATE);
}

bool LoadCell::turn_off()
{
    MOSFET_STATE = false;
    mosfet_state_change(MOSFET_STATE);
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

void LoadCell::print_tare()
{
    loadCellADC.printTare();
}

void LoadCell::read_weight(byte times)
{   
    //loadCellADC.begin();
    //loadCellADC.printTare();
    loadCellADC.getWeight(times);
    //loadCellADC.readAverage(20);
}



LoadCell loadCell;