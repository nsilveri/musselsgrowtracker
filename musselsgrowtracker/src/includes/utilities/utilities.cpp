// utilities.cpp

#include "utilities.h"
#include <STM32L0.h>
#include <Wire.h>

#include <string>
#include "..\ECCProcessor\ECCProcessor.h"
#include "..\GNSSFunctions\GNSSFunctions.h"
#include "..\loadCell\loadCell.h"
#include "..\ECCProcessor\sha256.h"

bool DEBUG_MODE;
uint8_t LOG_LEVEL;
bool LOG_END_LOOP = false;
String LOG_END_LINE_STRING = "END_LINE";
String LOG_WAIT_STRING = "wait";

#define intLedPin 10
bool LED_STATE = false;

byte bytePackaging::packData() {
    byte compressedBatteryLevel;
    if (pwrMan.get_BatPercent() <= 10) {
        compressedBatteryLevel = 0; // 00
    } else if (pwrMan.get_BatPercent() <= 25) {
        compressedBatteryLevel = 1; // 01
    } else if (pwrMan.get_BatPercent() <= 50) {
        compressedBatteryLevel = 2; // 10
    } else {
        compressedBatteryLevel = 3; // 11
    }
    byte gpsMovementBit = gnssHandler.getDisplacementStatus() ? 1 : 0;

    return (compressedBatteryLevel << 6) | (gpsMovementBit << 5);
}



byte* UID::get_UID() {
    static uint32_t UID[3] = {0, 0, 0};
    STM32L0.getUID(UID);  // Assumendo che questa funzione popoli l'array UID

    static byte uidArray[UID_LENGTH];

    // Converti ogni uint32_t in 4 byte
    for (int i = 0; i < 3; i++) {
        uidArray[4 * i] = (byte)((UID[i] >> 24) & 0xFF);
        uidArray[4 * i + 1] = (byte)((UID[i] >> 16) & 0xFF);
        uidArray[4 * i + 2] = (byte)((UID[i] >> 8) & 0xFF);
        uidArray[4 * i + 3] = (byte)(UID[i] & 0xFF);
    }

    return uidArray;
}

String UID::get_UID_String() {
    byte* uid = get_UID();   // Ottenere l'UID come array di byte
    String uidHex = "";      // Creare una stringa vuota per l'HEX
    String uidAscii = "";    // Creare una stringa vuota per l'ASCII

    for (int i = 0; i < UID_LENGTH; i++) {
        // Aggiungi la rappresentazione esadecimale del byte
        if (uid[i] < 16) uidHex += "0";  // Aggiungi uno zero per valori esadecimali a una cifra
        uidHex += String(uid[i], HEX);   // Convertire ogni byte in esadecimale e aggiungerlo alla stringa HEX

        // Aggiungi la rappresentazione ASCII del byte, se stampabile
        uidAscii += isPrintable(uid[i]) ? (char)uid[i] : '.';
    }

    uidHex.toUpperCase();  // Convertire la stringa HEX in maiuscolo

    // Combinare le due rappresentazioni in una stringa
    String combinedString = "UID: \n\tHEX: " + uidHex + "\n\tASCII: " + uidAscii;
    return combinedString;
}



void timerManager::startTimer() {
  startTime = millis();
  setEnable();
}

void timerManager::stopTimer() {
  savedElapsedTime = elapsedTimer();
  setDisable();
}

void timerManager::updateCurrTimer() {
  currentTime = millis();
}

unsigned long timerManager::elapsedTimer(unsigned int delay_ms) {
  updateCurrTimer();
  elapsedTime = (currentTime - startTime);
  log("currentTime: " + String(currentTime) + " | startTime: " + String(startTime), 2);
  log("elapsedTime: " + String(elapsedTime) + " | delay_ms: " + String(delay_ms), 2);
  return elapsedTime;
}

void timerManager::setEnable() {
  activeTimer = true;
}

void timerManager::setDisable() {
  activeTimer = false;
}

bool timerManager::getTimerStatus() {
  return activeTimer;
}

void timerManager::saveElapsedTimer() {
  savedElapsedTime = elapsedTimer();
}

unsigned long timerManager::getSavedElapsedTime() {
  return savedElapsedTime;
}

bool timerManager::delay(unsigned int delay_ms) {
  elapsedTime = elapsedTimer(delay_ms);  // Calcola il tempo trascorso finora
  if (elapsedTime >= delay_ms) {
    //log("DELAY: true.", 1);
    startTimer();
    return true;
  }else return false;
}

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
      if(LEVEL_MSG == 1 && LOG_LEVEL >= 1 && log != LOG_WAIT_STRING && log != LOG_END_LINE_STRING){
          LOG_END_LOOP = true;
          Serial.println("LOG1: " + log);
      }if(LEVEL_MSG == 2 && LOG_LEVEL >= 2 && log != LOG_WAIT_STRING && log != LOG_END_LINE_STRING){
          LOG_END_LOOP = true;
          Serial.println("LOG2: " + log);
      }if(log == LOG_WAIT_STRING){
          Serial.print(" . ");
      }if(log == LOG_END_LINE_STRING){
          Serial.println("\t============|END LOOP|============\n");
      }
  }
}

void deep_sleep(uint32_t seconds)
{
    log("deep_sleeping...", 1);
    unsigned long startTime;
    startTime = millis();

    //STM32L0.enablePowerSave();
    STM32L0.stop(seconds * 1000);
    //STM32L0.
    //HAL_PWREx_EnterSHUTDOWNMode();
    //STM32L0.deepsleep(seconds * 1000);
    //STM32L0.stop(seconds * 1000);
    unsigned long stopTime = millis();
    unsigned long elapsedTime = stopTime - startTime;
    log("deep_sleep for " + String(elapsedTime/1000) + " seconds.", 1);
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
    LED_STATE = false;
    digitalWrite(intLedPin, LED_STATE);
    log("intLED OFF", 2);
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

void WireScan::i2cScan() {
  // scan for i2c devices
  byte error, address;
  int nDevices;

  Serial.println("Scanning...");

  struct I2CDevice {
    byte address;
    const char* name;
  };

  I2CDevice devices[] = {
    {0x14, "BMA400 (accelerometer)"},
    {0x48, "ADS1115 (ADC for read external batt voltage)"},
    {0x60, "Internal device"},
    {0x68, "GY521 (accelerometer)"},
    {0x77, "Internal device"}
  };

  nDevices = 0;
  for (int i = 0; i < sizeof(devices) / sizeof(devices[0]); i++) {
    address = devices[i].address;
    Wire.beginTransmission(address);
    error = Wire.endTransmission();

    if (error == 0) {
      Serial.print("I2C device found at address 0x");
      if (address < 16) 
        Serial.print("0");
      Serial.print(address, HEX);
      Serial.print(" (");
      Serial.print(devices[i].name);
      Serial.println(") !");
      
      nDevices++;
    } else if (error == 4) {
      Serial.print("Unknown error at address 0x");
      if (address < 16) 
        Serial.print("0");
      Serial.println(address, HEX);
    }    
  }
  if (nDevices == 0)
    Serial.println("No I2C devices found\n");
  else
    Serial.println("done\n");
}

void mosSwitch::begin()
{
  pinMode(MOSFET_PIN,OUTPUT);
};

bool mosSwitch::mosfet_state_change(bool STATE)
{
    if(STATE) {
      pinMode(MOSFET_PIN, OUTPUT);
      digitalWrite(MOSFET_PIN, STATE);
    }else if(!STATE) {
      digitalWrite(MOSFET_PIN, STATE);
      pinMode(MOSFET_PIN, INPUT);
    }
    
    //analogWrite(MOSFET_PIN, )
    return MOSFET_STATE;
}

bool mosSwitch::getMosfetState()
{
  return MOSFET_STATE;
}

bool mosSwitch::turn_on_off()
{   
    MOSFET_STATE = !MOSFET_STATE;
    mosfet_state_change(MOSFET_STATE);
    return MOSFET_STATE;
}

bool mosSwitch::turn_off()
{
    if(MOSFET_STATE){
      MOSFET_STATE = false;
      mosfet_state_change(MOSFET_STATE);
    }
}

bool mosSwitch::turn_on()
{   
    if(!MOSFET_STATE){
      MOSFET_STATE = true;
      mosfet_state_change(MOSFET_STATE);
    }
}

void GNSSEeprom::saveGNSSCoordinates(double lat, double lon) {
    EEPROMWrite(latAddress, lat);
    EEPROMWrite(lonAddress, lon);
}

double GNSSEeprom::readLatitude() {
    return EEPROMRead(latAddress);
}

double GNSSEeprom::readLongitude() {
    return EEPROMRead(lonAddress);
}

void GNSSEeprom::EEPROMWrite(int address, double value) {
    EEPROM.updateDouble(address, value);
}

double GNSSEeprom::EEPROMRead(int address) {
    double value = EEPROM.readDouble(address++);
    return value;
}

UID getIDDevice;
mosSwitch mosfetSwitch;
intLED intBlueLED;
WireScan i2cScan;
GNSSEeprom gnssEeprom;
bytePackaging bytePack;