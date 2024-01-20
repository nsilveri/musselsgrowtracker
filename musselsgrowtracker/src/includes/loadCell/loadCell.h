#ifndef LOAD_CELL_H
#define LOAD_CELL_H

#include "Arduino.h"

class LoadCell
{
    public:
        void begin();
        //bool mosfet_state_change(bool STATE);
        //bool turn_on();
        //bool turn_on_off();
        //bool turn_off();
        void setOffset();
        void setScale(float weightKnown);
        void calibrate(float weightKnown);
        float read_weight(byte times);
        void setEmptyWeight();
        void print_tare();
        float get_LastWeightReading();
    private:
        float lastWeightReading = -1;

};

extern LoadCell loadCell;

#endif /* LOAD_CELL_H */
