#include "loadCell.h"
#include "hx711_basic.h"
#include "..\utilities\utilities.h"
//#include <Wire.h>

float weightKnown = 1000;
float scaleFactorDefault = 43.34;
float emptyWeight = 0.00;


void LoadCell::begin()
{
    //mosfetSwitch.turn_on();
    loadCellADC.begin();
}

/*
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
*/

void LoadCell::setOffset()
{
    mosfetSwitch.turn_on();
    loadCellADC.setOffset();
    mosfetSwitch.turn_off();
}

void LoadCell::calibrate(float weightKnown)
{
    mosfetSwitch.turn_on();
    loadCellADC.calibrate(weightKnown);
    mosfetSwitch.turn_off();
}


void LoadCell::setScale(float weightKnown)
{
    mosfetSwitch.turn_on();
    loadCellADC.setScale(weightKnown);
    mosfetSwitch.turn_off();
}

void LoadCell::print_tare()
{
    loadCellADC.printTare();
}

float LoadCell::read_weight(byte times)
{   
    mosfetSwitch.turn_on();
    lastWeightReading = loadCellADC.getWeight(times);
    mosfetSwitch.turn_off();
    return lastWeightReading;
}

float LoadCell::get_LastWeightReading()
{   
    return lastWeightReading;
}

void LoadCell::setEmptyWeight()
{
    mosfetSwitch.turn_on();
    emptyWeight = loadCellADC.setEmptyWeight();
    mosfetSwitch.turn_off();
}

LoadCell loadCell;