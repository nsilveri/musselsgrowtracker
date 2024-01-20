#ifndef BAF5C53D_5073_4F75_997B_7B3F2BD64277
#define BAF5C53D_5073_4F75_997B_7B3F2BD64277
#ifndef MPU6050_H
#define MPU6050_H

#include <Arduino.h>
#include <TinyMPU6050.h>
#include <Wire.h>
#include "..\utilities\utilities.h"

class GY521Sensor {
public:
    GY521Sensor();
    void begin();
    void testData();

private:
    void PrintGets();
    MPU6050 GY521Lib; // Rimuovi l'argomento "Wire" dal costruttore di MPU6050
};

extern GY521Sensor accMeter;

#endif


#endif /* BAF5C53D_5073_4F75_997B_7B3F2BD64277 */
