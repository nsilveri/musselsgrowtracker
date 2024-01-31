// utilities.h

#ifndef UTILITIES_H
#define UTILITIES_H

#include <Arduino.h>
#include <STM32L0.h>
#include <EEPROMex.h>
#include "..\batteryManager\batteryMan.h"
#include "..\LoRa\LoRaWANLib.h"
//#include <EEPROM.h>

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

class ethTransaction {
  public:
      void CreateRawTransaction(byte *byteArray, size_t arraySize); // Aggiungi la stringa data come parametro
  private:
//    EthTransaction ethTx; // Istanza della classe EthereumTransaction
};

class UID {
public:
    static const int UID_LENGTH = 12; // Definizione della lunghezza dell'UID

    static byte* get_UID(); // Per ottenere l'UID come array di byte
    static String get_UID_String(); // Per ottenere l'UID come stringa esadecimale
};

class LoRaPayload {
  public:
      byte* createDataMsg(const byte* SerialID, uint32_t TimestampLinux, byte* GpsData, uint16_t LoadCellMeasurement, byte SysStatus);
      
      String bytesToHexString(const byte* buffer, size_t bufferSize);
      String bytesToString(const byte* buffer, size_t bufferSize);
      
      size_t get_MsgSize();
      byte* get_DataMsg() const;
      
      String get_msgStr();
      String get_msgHex();

      void set_msgByte(byte* msg, size_t msgSize);

      String set_msgStr(String msg);
      String set_msgHex(String msg);

      void logBytes(const byte* data, size_t size, const char* message);

  private:
      void clearData();

      String msgToSendStr;
      String msgToSendHex;

      byte* dataMsg;
      size_t dataMsgSize;
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

class MSGSend {
  public:
      void sendMsg();
      void set_EccSign();
      void set_EthSign();
  private:
      bool ECC_MODE = true;
      bool ETH_MODE = false;
      bool SEND_MODE = ECC_MODE;
      
      void sendMsgETH();
      void sendMsgECC();
};

extern UID getIDDevice;
extern intLED intBlueLED;
extern WireScan i2cScan;
extern mosSwitch mosfetSwitch;
extern UID uidCode;
extern ethTransaction ethTx;
extern LoRaPayload MSGPayload;
extern GNSSEeprom gnssEeprom;
extern bytePackaging bytePack;
extern MSGSend msgService;
//extern timerManager timerMan;

#endif // UTILITIES_H
