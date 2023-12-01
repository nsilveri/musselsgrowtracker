#ifndef LOAD_CELL_H
#define LOAD_CELL_H

#include "Arduino.h"

#define MOSFET_PIN 9

class LoadCell
{
    public:
        void begin();
        bool mosfet_state_change(bool STATE);
        bool turn_on();
        bool turn_on_off();
        bool turn_off();
        void setOffset();
        void setScale(float weightKnown);
        void calibrate(float weightKnown);
        void read_weight(byte times);
        void print_tare();
    private:

};

extern LoadCell loadCell;

#endif /* LOAD_CELL_H */
