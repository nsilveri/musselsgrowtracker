// LoRaWANLib.h
#ifndef LoRaWANLib_h
#define LoRaWANLib_h

#include "Arduino.h"
#include "library\_LoRaWAN.h"
#include "..\ECCProcessor\ECCProcessor.h"
#include "..\loadCell\loadCell.h"

class LoRaWANLib {
public:
    LoRaWANLib(const char *appEui, const char *appKey, const char *devEui);
    void begin();
    void getInfo();
    void sendMessage(byte* payload, size_t payloadSize);

private:
    const char *_appEui;
    const char *_appKey;
    const char *_devEui;
};

class LoRaPayload {
public:
    uint8_t* createDataMsg(const uint8_t* SerialID, uint32_t TimestampLinux, uint8_t* GpsData, uint16_t LoadCellMeasurement, uint8_t SysStatus);
    
    String bytesToHexString(const uint8_t* buffer, size_t bufferSize);
    String bytesToString(const uint8_t* buffer, size_t bufferSize);
    void hexToSHA256(String data, uint8_t* hash_out);
    
    size_t get_MsgSignedSize() const;
    size_t get_MsgSize();
    uint8_t* get_DataMsg() const;
    uint8_t* get_DataSignedMsg() const;
    uint8_t* get_CombinedMsg() const;
    
    String get_msgStr();
    String get_msgHex();
    uint8_t* get_msgHashUint8();
    String get_msgHashString();

    void set_msgByte(uint8_t* msg, size_t msgSize);
    void setDataMsg(const uint8_t* msg, size_t size);
    void setDataSignedMsg(const uint8_t* msg, size_t size);
    void setCombinedMsg(const uint8_t* msg, size_t size);

    String set_msgStr(String msg);
    String set_msgHex(String msg);
    
    uint8_t* sendMergedMsg(bool TEST = false);

    void logBytes(const uint8_t* data, size_t size, const char* message);

private:
    void clearData(); // Potrebbe essere necessario rimuovere o aggiornare in base alla nuova logica
    void clearDataMsg(); // Pulisce dataMsg
    void clearDataSignedMsg(); // Pulisce dataSignedMsg
    void clearCombinedMsg(); // Pulisce combinedMsg
    String byteToHexString(uint8_t byte);
    String arrayToHexString(uint8_t* array, size_t arraySize);
    void hexStringToArray(String hexString, uint8_t* array, size_t arraySize);

    String msgToSendStr;
    String msgToSendHex;
    String msgToSendHashStr;
    
    uint8_t* dataMsg = nullptr;
    size_t dataMsgSize = 0;

    uint8_t* dataSignedMsg = nullptr;
    size_t dataSignedMsgSize = 0;

    uint8_t* combinedMsg = nullptr;
    size_t combinedMsgSize = 0;

    uint8_t msgToSendHash[32];
};

class MSGSend {
  public:
      void sendMsg();

  private:
      void sendMsgECC();
};

extern LoRaPayload MSGPayload;
extern MSGSend msgService;
extern LoRaWANLib LoRaWANManager;

#endif
