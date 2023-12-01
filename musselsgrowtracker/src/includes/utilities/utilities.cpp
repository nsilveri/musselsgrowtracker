// utilities.cpp

#include "utilities.h"
#include <STM32L0.h>
#include <Wire.h>

bool DEBUG_MODE;
uint8_t LOG_LEVEL;

#define intLedPin 10
bool LED_STATE = false;


void setDebugMode(bool value) {
  DEBUG_MODE = value;
}

void setDebugLevel(uint8_t value) {
  LOG_LEVEL = value;
}

DebugState getDebugState() {
  return {DEBUG_MODE, LOG_LEVEL};
}

void log(String log, uint8_t LEVEL_MSG) 
{
    if(DEBUG_MODE){
        if(LEVEL_MSG == 1 && LOG_LEVEL >= 1){
            Serial.println("LOG1: " + log);
        }if(LEVEL_MSG == 2 && LOG_LEVEL >= 2){
            Serial.println("LOG2: " + log);
        }
    }
}

void deep_sleep(uint32_t seconds)
{
    log("deep_sleeping...", 1);
    unsigned long startTime;
    startTime = millis();

    STM32L0.stop(seconds * 1000);

    unsigned long stopTime = millis();
    unsigned long elapsedTime = stopTime - startTime;
    log("deep_sleep for " + String(elapsedTime/1000) + " seconds.", 1);
}
/*
void STM32L0_Voltage::(void(*callback)(void))
{
    _locationCallback = Callback(callback);
}
*/

float getVDDA()
{
  float VDDA = STM32L0.getVDDA();
  log("VDDA= " + String(VDDA), 1);
  return VDDA;
}

float getTemp()
{
  float Temperature = STM32L0.getTemperature();
  log("Temp= " + String(Temperature), 1);
  return Temperature;
}

void intLED::begin()
{
  pinMode(intLedPin, OUTPUT);
  digitalWrite(intLedPin, LED_STATE);
}

void intLED::intLED_on()
{
  if(LED_STATE != true)
  {
    LED_STATE = true;
    digitalWrite(intLedPin, LED_STATE);
    log("intLED ON", 2);
  }
}

void intLED::intLED_on_off()
{
  LED_STATE = !LED_STATE;
  digitalWrite(intLedPin, LED_STATE);
  log("intLED " + String(LED_STATE), 2);
}

void intLED::intLED_off()
{
  if(LED_STATE != false)
  {
    LED_STATE = false;
    digitalWrite(intLedPin, LED_STATE);
    log("intLED OFF", 2);
  }
}

void intLED::visualLog()
{
  LED_STATE = false;
  for(uint8_t i = 0; i < 3; i++){
    intBlueLED.intLED_on_off();
    delay(100);
  }
  log("LED log", 2);
}

void WireScan::begin()
{
  pinMode(14, OUTPUT); // hold SDA low to wake up ECC608A
  digitalWrite(14, LOW);
  delay(1000);

  Wire.begin(); // set master mode 
  Wire.setClock(400000); // I2C frequency at 400 kHz  
  delay(1000);
}

void WireScan::scan()
{
  i2cScan();
}

void WireScan::i2cScan()
{
// scan for i2c devices
  byte error, address;
  int nDevices;

  Serial.println("Scanning...");

  nDevices = 0;
  for(address = 1; address < 127; address++ ) 
  {
    // The i2c_scanner uses the return value of
    // the Write.endTransmission to see if
    // a device did acknowledge to the address.
    Wire.beginTransmission(address);
    error = Wire.endTransmission();
      
    if (error == 0)
    {
      Serial.print("I2C device found at address 0x");
      if (address<16) 
        Serial.print("0");
      Serial.print(address,HEX);
      Serial.println("  !");

      nDevices++;
    }
    else if (error==4) 
    {
      Serial.print("Unknown error at address 0x");
      if (address<16) 
        Serial.print("0");
      Serial.println(address,HEX);
    }    
  }
  if (nDevices == 0)
    Serial.println("No I2C devices found\n");
  else
    Serial.println("done\n");
    
}

intLED intBlueLED;
WireScan i2cScan;