// utilities.h

#ifndef UTILITIES_H
#define UTILITIES_H

#include <Arduino.h>
#include <STM32L0.h>
#include <EEPROMex.h>
#include "..\batteryManager\batteryMan.h"
#include "..\LoRa\LoRaWANLib.h"

extern bool DEBUG_MODE;
extern uint8_t LOG_LEVEL;
extern volatile uint8_t Step;
extern bool DEBUG;
extern bool LOG_END_LOOP;
extern String LOG_END_LINE_STRING;

class bytePackaging {
  public:
    byte packData();
};

class timerManager {
public:
    void startTimer();
    void stopTimer();
    void updateCurrTimer();
    void setEnable();
    void setDisable();
    bool getTimerStatus();
    unsigned long elapsedTimer(unsigned int delay_ms = 0);
    void saveElapsedTimer();
    unsigned long getSavedElapsedTime();
    bool delay(unsigned int delay_ms);

private:
    bool activeTimer;
    unsigned long startTime;
    unsigned long currentTime;
    unsigned long elapsedTime;
    unsigned long savedElapsedTime;
};

struct DebugState {
  bool mode;
  int level;
};

void setDebugMode(bool value);
void setDebugLevel(uint8_t value);
void log(String log, uint8_t LOG_LEVEL);
void deep_sleep(uint32_t seconds);
float getVDDA();
float getTemp();

class intLED
{
  public:
    void begin();
    void intLED_on();
    void intLED_on_off();
    void intLED_off();
    void visualLog();
};

class WireScan
{
  public:
    void begin();
    void scan();
  private:
    void i2cScan();
};

class systemDate
{
  public:
    void printRtcDate();
    void printGnssDate();
  private:
};

class mosSwitch
{
  public:
    void begin();
    bool getMosfetState();
    bool turn_on_off();
    bool turn_off();
    bool turn_on();

  private:
    bool MOSFET_STATE = false;
    uint8_t MOSFET_PIN = 8;
    bool mosfet_state_change(bool STATE);
};


class UID {
public:
    static const int UID_LENGTH = 12;

    static byte* get_UID();
    static String get_UID_String();
};

class GNSSEeprom {
  public:
      void   saveGNSSCoordinates(double lat, double lon);
      double readLatitude();
      double readLongitude();
      //void   formatEEPROM();

  private:
      void EEPROMWrite(int address, double value);
      double EEPROMRead(int address);
      const uint8_t EEPROM_START_LOC_BYTE = 0;
      uint8_t latAddress = 0;
      uint8_t lonAddress = 10;
};

extern UID getIDDevice;
extern intLED intBlueLED;
extern WireScan i2cScan;
extern mosSwitch mosfetSwitch;
extern UID uidCode;
extern GNSSEeprom gnssEeprom;
extern bytePackaging bytePack;

#endif // UTILITIES_H
