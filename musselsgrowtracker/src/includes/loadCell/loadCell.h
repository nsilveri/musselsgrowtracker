#ifndef LOAD_CELL_H
#define LOAD_CELL_H

#include "Arduino.h"

#define MOSFET_PIN 9

class LoadCell
{
    public:
        void begin();
        bool turn_on();
        bool turn_on_off();
        bool turn_off();
        void setOffset();
        void setScale(float weightKnown);
        void calibrate(float weightKnown);
        void read_weight();
    private:

};

extern LoadCell loadCell;

#endif /* LOAD_CELL_H */
