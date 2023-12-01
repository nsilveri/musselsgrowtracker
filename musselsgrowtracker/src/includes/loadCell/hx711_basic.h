#ifndef HX711_BASIC_H
#define HX711_BASIC_H

class HX711_module
{
    public:
        void begin();
        void read();
        void printTare();
        void setOffset();
        void setScale(float weightKnown);
        void calibrate(float weightKnown);
        void getWeight(byte times);
        void readAverage(uint8_t nSamples = 20);
};

extern HX711_module loadCellADC;


#endif /* HX711_BASIC_H */
