#include "handleSerialCommand.h"
#include "..\GNSSFunctions\GNSSFunctions.h"
#include "..\SPI\SPIFlash.h"
#include <STM32L0.h>
#include "..\BMA400\BMA400Functions.h"
#include "..\loadCell\loadCell.h"
#include "..\utilities\utilities.h"
#include "..\LoRa\LoRaWANLib.h"
#include "..\GY521\GY521Sensor.h"
//#include "..\ECCProcessor\ECCProcessor.h"
#include "..\ECCProcessor\ECCProcessor.h"
#include "..\batteryManager\batteryMan.h"
//#include "..\ECCProcessor\nanoEccLib\libs\bignum256.h"
//#include "..\ECCProcessor\nanoEccLib\types.h"
//#include "..\ECCProcessor\CryptoUtils.h"
#include "..\ECCProcessor\sha256.h"

#include <cstring>

#define csPin 25

bool DEBUG = true;
uint8_t LOG_LEV =1;
volatile uint8_t Step = 0;
float bottleWeight = 1000.0;
float accelData[3];

bool EncryptionMode = true;

SPIFlash intMemory(csPin);

bool GNSS_RESUME_SUSPEND = false;
bool enableRoutine = true;

uint32_t UID[3] = {0, 0, 0};
byte SysStatusByte;
byte* dataMsg = nullptr;
String msgToSendStr;
String msgToSendHex;

//const char* message = "Messaggio segreto";
//size_t messageLength = strlen(message);
//uint8_t encryptedMessage[128]; // Assicurati che sia abbastanza grande
//size_t encryptedLength = sizeof(encryptedMessage);

//BigNum256 privateKey;
//PointAffine publicKey;

const char* message;
size_t messageLen = strlen(message);

String hashMsg;//[SHA256_HASH_LENGTH];

// Prepara le variabili per la firma
//BigNum256 r, s;
//EccPoint publicKey;
//uint8_t privateKey[NUM_ECC_DIGITS];
//PointAffine affinePublicKey;

void handleSerialCommand(char command) {
  switch (command) {
    case 'u':
      log(uidCode.get_UID_String(), 1);
    break;

    /*
    case 'R':
      log("Format EEPROM", 1);
      gnssEeprom.formatEEPROM();
      gnssEeprom.saveGNSSCoordinates(gnssHandler.getInvalidLat(), gnssHandler.getInvalidLon());
      delay(1000);
      log("EEPROM formatted", 1);
      log("lat: " + String(gnssEeprom.readLatitude()) + ", lon: " + String(gnssEeprom.readLongitude()), 1);
    break;
    */

    case 'r':
      log("Data on EEPROM: " + String(gnssEeprom.readLatitude(), 6) + ", " + String(gnssEeprom.readLongitude(), 6), 1);
    break;
    
    //case 'y':
    //  eccProcessor.encryptMessage((const uint8_t*)message, messageLength, encryptedMessage);
    //
    //  break;
    
    case 'y':
      //eccProcessor.signMessageFunc();
      msgService.sendMsg();
    
    break;


    case 'q':
      dataMsg = MSGPayload.createDataMsg(getIDDevice.get_UID(), RTC.getEpoch(), gnssHandler.generateLoRaMsg(), loadCell.get_LastWeightReading(), bytePack.packData());
      msgToSendStr = MSGPayload.bytesToString(dataMsg, MSGPayload.get_MsgSize());
      msgToSendHex = MSGPayload.bytesToHexString(dataMsg, MSGPayload.get_MsgSize());
      hashMsg = eccProcessor.generateSHA256Hash(msgToSendHex);
      delete[] dataMsg;
      log("msgToSend:  \n\t[HEX]: " + String(msgToSendHex) + "\n\t[HASH]: " + String(hashMsg), 1);
      eccProcessor.signature_func();
    break;

    case 'S':
      LoRaWANManager.begin();
    break;
    
    case 's':
      dataMsg = MSGPayload.createDataMsg(getIDDevice.get_UID(), RTC.getEpoch(), gnssHandler.generateLoRaMsg(), loadCell.get_LastWeightReading(), bytePack.packData());
      msgToSendHex = MSGPayload.bytesToHexString(dataMsg, MSGPayload.get_MsgSize());
      log("msgT", 1);
      LoRaWANManager.sendMessage(dataMsg, sizeof(dataMsg));
      delete[] dataMsg;
      /*
        log("Received manual command!", 1);
        log("COMMAND: " + String(command), 2);
        Step = 1;
      */
      break;

    case 'I':
      LoRaWANManager.getInfo();
      break;

    case 'g':
      log("COMMAND: " + String(command), 2);
      //gnssHandler.readPositioningData();
      gnssHandler.update();
      break;

    case 'G':
      log("COMMAND: " + String(command), 2);
      //gnssHandler.readPositioningData();
      if(GNSS_RESUME_SUSPEND) {
        gnssHandler.toggleGNSS(false);
        GNSS_RESUME_SUSPEND = !GNSS_RESUME_SUSPEND;
      }else if(!GNSS_RESUME_SUSPEND) {
        gnssHandler.toggleGNSS(true);
        GNSS_RESUME_SUSPEND = !GNSS_RESUME_SUSPEND;
      }
      log("GNSS_RESUME_SUSPEND: " + String(GNSS_RESUME_SUSPEND), 1);
      break;

    case 'd':
      log("DEBUG_MODE :" + String(DEBUG), 1);
      log("COMMAND: " + String(command), 2);
      DEBUG = !DEBUG;
      setDebugMode(DEBUG);
      break;

    case '1':
      log("DEBUG_MODE :" + String(DEBUG), 1);
      log("COMMAND: " + String(command), 2);
      LOG_LEV = 1;
      setDebugLevel(LOG_LEV);
      break;

    case '2':
      log("DEBUG_MODE :" + String(DEBUG), 1);
      log("COMMAND: " + String(command), 2);
      LOG_LEV = 2;
      setDebugLevel(LOG_LEV);
      break;

    case 'o':
      gnssHandler.readGpsTime();
      break;
    
    case 'a':
      log("Internal:", 1);
      log(BMA400Func.readTempData() + "C°", 1);
      float destination[3];
      BMA400Func.readAcceleration(destination);
      log("X: " + String(destination[0]) + " Y: " + String(destination[1]) + " Z: " + String(destination[2]) , 1);
      log("External:", 1);
      accMeter.testData();
      break;

    case '5':
      BMA400Func.setOffset(accelData);
      log("Acceleration data--> X: " + String(accelData[0]) + " Y: " + String(accelData[1]) + " Z: " + String(accelData[2]), 1);
      break;

    case 'i':
      log("I2C bus scanning...", 1);
      i2cScan.begin();
      i2cScan.scan();
      break;

    case 'm':
      log("Mosfet state: " + String(mosfetSwitch.turn_on_off()), 1);
      break;

    case 'c':
      loadCell.read_weight(100);
      break;

    case 'x':
      loadCell.setOffset();
      //loadCell.setEmptyWeight();
      break;

    case 'P':
      loadCell.print_tare();
      break;

    case 'e':
      enableRoutine = !enableRoutine;
      if(enableRoutine){
        log("Routine enabled", 1);
      }else if(!enableRoutine){
        log("Routine disabled", 1);
      }
      break;

    case 'B':
      loadCell.calibrate(bottleWeight);
      break;

    case 'b':
      pwrMan.get_BatPercent();
      //bytePackaging
      break;

    case 'z':
      loadCell.setScale(bottleWeight);
      break;

    case 'T':
      gnssHandler.updateRTCViaGNSS();
      break;

    case 'F':
      log("Change fix type...", 1);
      //gnssHandler.
      break;

    case 'p':
      deep_sleep(10);
      break;

    case 'l':
      intBlueLED.intLED_on_off();
      break;

    case 'v':
      pwrMan.get_VDDA();
      break;
    /*
    case 'k':
      Wire.end();
      break;
    */
    case 't':
      getTemp();
      break;

    case 'h':
    case '\0':
      log("HELP :\n \t\t\t-s) Trigger GNSS reading manually\n \t\t\t-g) Read Device GPS data\n \t\t\t-d) Enable/disable debug\n \t\t\t-c) Read load cell output\n \t\t\t-p) deep-sleep test\n \t\t\t-m) Enable/disable mosfet\n \t\t\t-1) Set log level 1\n \t\t\t-2) Set log level 2\n \t\t\t-o) Read GPS time\n \t\t\t-i) I2C bus scanning\n \t\t\t-f) Read internal memory info\n \t\t\t-l) Turn on/off blue LED\n \t\t\t-v) Get VDDA\n \t\t\t-t) Get temperature\n \t\t\t-x) Set load cell offset", 1);
      break;

    default:
      log("invalid command!", 1);
  }
}
