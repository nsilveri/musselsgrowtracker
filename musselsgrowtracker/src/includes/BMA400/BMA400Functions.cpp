#include "library\BMA400.h"
#include "BMA400Functions.h"
#include <STM32L0.h>

int32_t rawPress, rawTemp, rawHumidity, compTemp;   // pressure, humidity, and temperature raw count output for BME280
uint32_t compHumidity, compPress;                   // variables to hold compensated BME280 humidity and pressure values
float temperature_C, temperature_F, pressure, humidity, altitude; // Scaled output of the BME280
int16_t accelBias[3];

//BMA400 definitions
#define BMA400_intPin1 3    // interrupt1 pin definitions
#define BMA400_intPin2 A4   // interrupt2 pin definitions

/* Specify sensor parameters (sample rate is twice the bandwidth)
 * choices are:
      AFS_2G, AFS_4G, AFS_8G, AFS_16G  
      BW_7_81Hz, BW_15_63Hz, BW_31_25Hz, BW_62_5Hz, BW_125Hz, BW_250Hz, BW_500Hz, BW_1000Hz
      normal_Mode, deepSuspend_Mode, lowPower_Mode, suspend_Mode
      sleep_0_5ms, sleep_1ms, sleep_2ms, sleep_4ms, sleep_6ms, sleep_10ms, sleep_25ms, sleep_50ms, sleep_100ms, sleep_500ms, sleep_1000ms
*/ 
uint8_t Ascale = AFS_4G, SR = SR_200Hz, power_Mode = lowpower_Mode, OSR = osr0, acc_filter = acc_filt2;
//uint8_t Ascale = AFS_8G, BW = BW_250Hz, power_Mode = lowPower_Mode, sleep_dur = sleep_1000ms;
//uint8_t low_power_Mode = lp_mode_1, motion_threshold = 20, tapStatus, tapType;

// Logic flags to keep track of device states
bool BMA400_wake_flag = true;
bool BMA400_sleep_flag = false;
bool InMotion = false;
bool ActivityOn = true;

float aRes;             // scale resolutions per LSB for the sensor
int16_t accelCount[3];  // Stores the 16-bit signed accelerometer sensor output
int16_t tempCount;      // temperature raw count output
float   temperature;    // Stores the real internal chip temperature in degrees Celsius
float ax, ay, az;       // variables to hold latest sensor data values 
float offset[3];        // accel bias offsets

//pinMode(BMA400_intPin1, INPUT);  // define BMA400 wake and sleep interrupt pins as L082 inputs
//pinMode(BMA400_intPin2, INPUT);

BMA400 BMA400(BMA400_intPin1, BMA400_intPin2); // instantiate BMA400 class

/*
void myinthandler2()
{
  BMA400_wake_flag = true; 
  STM32L0.wakeup();
  Serial.println("BMA400 is awake!");
}


void myinthandler1()
{
  BMA400_sleep_flag = true;
  STM32L0.wakeup();
  Serial.println("BMA400 is asleep!");
}
*/

void BMA400Functions::begin()
{
    aRes = BMA400.getAres(Ascale);                                       // get sensor resolutions, only need to do this once
    //BMA400.resetBMA400();                                                // software reset before initialization
    delay(100);      
    BMA400.selfTestBMA400();                                             // perform sensor self test
    //BMA400.resetBMA400();                                                // software reset before initialization
    delay(1000);                                                         // give some time to read the screen
    BMA400.CompensationBMA400(Ascale, SR, normal_Mode, OSR, acc_filter, offset); // quickly estimate offset bias in normal mode
    BMA400.initBMA400(Ascale, SR, power_Mode, OSR, acc_filter);          // Initialize sensor in desired mode for application
    BMA400.deactivateNoMotionInterrupt();

    //attachInterrupt(BMA400_intPin1, myinthandler1, RISING);  // attach no-motion interrupt for INT1 pin output of BMA400
    //attachInterrupt(BMA400_intPin2, myinthandler2, RISING);  // attach   wake-up interrupt for INT2 pin output of BMA400
    BMA400.getStatus(); // read status of interrupts to clear
}

float BMA400Functions::convertToMetersPerSecondSquared(int16_t rawValue) {
    const float scaleFactor = 9.81f / 8192.0f; // Fattore di scala per AFS_8G
    return rawValue * scaleFactor;
}

void BMA400Functions::readAcceleration(float *destination) {
    uint8_t rawData[6];  // dati del registro di accelerazione x/y/z
    BMA400.readBytes(BMA400_ADDRESS, BMA400_ACCD_X_LSB, 6, rawData);  // Leggi i 6 registri di dati grezzi

    // Converti il MSB e il LSB in un valore firmato a 16 bit e applica la compensazione
    int16_t rawAcc[3] = {
        (int16_t)(((int16_t)rawData[1] << 8) | rawData[0]) - accelBias[0],
        (int16_t)(((int16_t)rawData[3] << 8) | rawData[2]) - accelBias[1],
        (int16_t)(((int16_t)rawData[5] << 8) | rawData[4]) - accelBias[2]
    };

    // Converti in metri al secondo quadrato
    for (int i = 0; i < 3; i++) {
        destination[i] = convertToMetersPerSecondSquared(rawAcc[i]);
    }
}

void BMA400Functions::setOffset(float *destination) {
    uint8_t rawData[6];  // dati del registro di accelerazione x/y/z
    BMA400.readBytes(BMA400_ADDRESS, BMA400_ACCD_X_LSB, 6, &rawData[0]);  // Leggi i 6 registri di dati grezzi nella matrice data
    destination[0] = (int16_t)(((int16_t)rawData[1] << 8) | rawData[0]);  // Converti il MSB e il LSB in un valore firmato a 16 bit
    destination[1] = (int16_t)(((int16_t)rawData[3] << 8) | rawData[2]);
    destination[2] = (int16_t)(((int16_t)rawData[5] << 8) | rawData[4]);
    // Applica la compensazione se necessario
    destination[0] -= accelBias[0];
    destination[1] -= accelBias[1];
    destination[2] -= accelBias[2];
    accelBias[0] =    destination[0];
    accelBias[1] =    destination[1];
    accelBias[2] =    destination[2];
}

int16_t BMA400Functions::readTempData()
{
    byte c = BMA400.getChipID();  // Read CHIP_ID register for BMA400
    Serial.print("BMA400 "); Serial.print("I AM "); Serial.print(c, HEX); Serial.print(" I should be "); Serial.println(0x90, HEX);
    return BMA400.readBMA400TempData();
}

BMA400Functions BMA400Func;

/*
void BMA280Functions::readDataBME280()
{
    BME280.BME280forced();  // get one data sample, then go back to sleep
    
    rawTemp =  BME280.readBME280Temperature();
    compTemp = BME280.BME280_compensate_T(rawTemp);
    temperature_C = (float) compTemp/100.0f;
    temperature_F = 9.0f*temperature_C/5.0f + 32.0f;
     
    rawPress =  BME280.readBME280Pressure();
    compPress = BME280.BME280_compensate_P(rawPress);
    pressure = (float) compPress/25600.0f; // Pressure in mbar
    altitude = 145366.45f*(1.0f - powf((pressure/1013.25f), 0.190284f));   
   
    rawHumidity =  BME280.readBME280Humidity();
    compHumidity = BME280.BME280_compensate_H(rawHumidity);
    humidity = (float)compHumidity/1024.0f; // Humidity in %RH
 
    if(SerialDebug){
    Serial.println("BME280:");
    Serial.print("Altimeter temperature = "); 
    Serial.print( temperature_C, 2); 
    Serial.println(" C"); // temperature in degrees Celsius
    Serial.print("Altimeter temperature = "); 
    Serial.print(temperature_F, 2); 
    Serial.println(" F"); // temperature in degrees Fahrenheit
    Serial.print("Altimeter pressure = "); 
    Serial.print(pressure, 2);  
    Serial.println(" mbar");// pressure in millibar
    Serial.print("Altitude = "); 
    Serial.print(altitude, 2); 
    Serial.println(" feet");
    Serial.print("Altimeter humidity = "); 
    Serial.print(humidity, 1);  
    Serial.println(" %RH");// pressure in millibar
    Serial.println(" ");
}
*/