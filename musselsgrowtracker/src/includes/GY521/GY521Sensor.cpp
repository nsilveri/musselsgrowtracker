/*
 *  Mandatory includes
 */
#include <Arduino.h>
#include <TinyMPU6050.h>
#include "GY521Sensor.h"
#include "..\utilities\utilities.h"

timerManager accMeterTimer;

/*
 *  Constructing MPU-6050
 */
//GY521Sensor accMeter (Wire);

/*
 *  Method that prints everything
 */

GY521Sensor::GY521Sensor() : GY521Lib(Wire, MPU6050_ADDRESS) {
  // Inizializzazione degli eventuali membri privati della classe
}

void GY521Sensor::PrintGets () {
    // Shows offsets
    accMeterTimer.startTimer();
    //while(accMeterTimer.elapsedTimer() < 10000){
    //log("--- Offsets:", 1);
    //log("GyroX Offset = " + String(GY521Lib.GetGyroXOffset()), 1);
    //log("GyroY Offset = "+ String(GY521Lib.GetGyroYOffset()), 1);
    //log("GyroZ Offset = " + String(GY521Lib.GetGyroZOffset()), 1);
    // Shows raw data
    //log("--- Raw data:", 1);
    //log("Raw Acc:   x: " + String(GY521Lib.GetRawAccX()) + "   y: " + String(GY521Lib.GetRawAccY()) + "   z: " + String(GY521Lib.GetRawAccZ()), 1);
    //log("Raw Gyro:   x: " + String(GY521Lib.GetRawGyroX()) + "   y: " + String(GY521Lib.GetRawGyroY()) + "   y: " + String(GY521Lib.GetRawGyroZ()), 1);
    // Show readable data
    log("--- Readable data:", 1);
    log("Acc:   X: " + String(GY521Lib.GetAccX()) + "   y: "  + String(GY521Lib.GetAccY()) + "   z: " + String(GY521Lib.GetAccZ()) + "   \n\tGyroX: "  + String(GY521Lib.GetGyroX()) + "degrees/sec   Gyroy: " + String(GY521Lib.GetGyroY()) + " degrees/sec   GyroZ: " + String(GY521Lib.GetGyroZ()) + " degrees/second", 1);
    // Show angles based on accelerometer only
    log("--- Accel angles:", 1);
    log("AccelAng:\n\tX: " + String(GY521Lib.GetAngAccX()) + "   Y: " + String(GY521Lib.GetAngAccY()), 1);
    // Show angles based on gyroscope only
    log("--- Gyro angles:", 1);
    log("GyroAng(X,Y,Z) = " + String(GY521Lib.GetAngGyroX()) + ", " + String(GY521Lib.GetAngGyroY()) + ", " + String(GY521Lib.GetAngGyroZ()), 1);
    // Show angles based on both gyroscope and accelerometer
    log("--- Filtered angles:", 1);
    log("FilteredAng(X,Y,Z) = " + String(GY521Lib.GetAngX()) + "," + String(GY521Lib.GetAngY()) + "," + String(GY521Lib.GetAngZ()), 1);
    // Show filter coefficients
    log("--- Angle filter coefficients:", 1);
    log("Accelerometer percentage = " + String(GY521Lib.GetFilterAccCoeff()) + '%', 1);
    log("Gyroscope percentage = " + String(GY521Lib.GetFilterGyroCoeff()) + '%', 1);
    //}
}

/*
 *  Setup
 */
void GY521Sensor::begin() {

  // Initialization
  GY521Lib.Initialize();

  // Calibration
  log("=====================================", 1);
  log("Starting calibration...", 1);
  GY521Lib.Calibrate();
  log("Calibration complete!", 1);
}

/*
 *  Loop
 */
void GY521Sensor::testData() {
  
  GY521Lib.Execute();
  PrintGets();
}

GY521Sensor accMeter;