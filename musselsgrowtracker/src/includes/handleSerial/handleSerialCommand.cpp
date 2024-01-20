// handleSerialCommand.cpp

#include "handleSerialCommand.h"
#include "..\GNSSFunctions\GNSSFunctions.h"
#include "..\SPI\SPIFlash.h"
#include <STM32L0.h>
#include "..\BMA400\BMA400Functions.h"
#include "..\loadCell\loadCell.h"
#include "..\utilities\utilities.h"
#include "..\LoRa\LoRaWANLib.h"
#include "..\GY521\GY521Sensor.h"
#include "..\ECCProcessor\ECCProcessor.h"
//#include "..\ECCProcessor\CryptoUtils.h"

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
byte SysStatusByte = 156;
byte* dataMsg = nullptr;;
String msgToSendStr;
String msgToSendHex;

void handleSerialCommand(char command) {
  switch (command) {
    case 'u':
      log(uidCode.get_UID_String(), 1);
    break;
    
    case 'y':
      if(EncryptionMode)
      {
        log("ECC Encryption selected", 1);
        delay(2000);
        dataMsg = MSGPayload.createDataMsg(getIDDevice.get_UID(), RTC.getEpoch(), loadCell.get_LastWeightReading(), SysStatusByte);
        msgToSendHex = MSGPayload.bytesToHexString(dataMsg, MSGPayload.get_MsgSize());
        log("dataMsg HEX: " + msgToSendHex, 1);

        String PText = eccProcessor.plainText(msgToSendHex);
        log("Plaintext: " + PText, 1);
        
        String signature = eccProcessor.generateSignature(msgToSendHex);
        
        eccProcessor.encryption();

        bool isSignatureValid = eccProcessor.verifySignature(msgToSendHex, signature);

        String additionalInfo;
        if (isSignatureValid) {
            additionalInfo = " | Valid!"; // La stringa da aggiungere se la condizione è vera
        } else {
            additionalInfo = " | Not valid!"; // La stringa da aggiungere se la condizione è falsa
        }

        log("Signature ECC: " + signature + additionalInfo, 1);
        delete[] dataMsg;

      }else if(!EncryptionMode)
      {
        log("Ethereum-RPL encryption selected", 1);
        delay(2000);
        dataMsg = MSGPayload.createDataMsg(getIDDevice.get_UID(), RTC.getEpoch(), loadCell.get_LastWeightReading(), SysStatusByte);
        ethTx.CreateRawTransaction(dataMsg, MSGPayload.get_MsgSize());
        delete[] dataMsg;
      }
    break;
    
    /*
    case 'y':
    if (EncryptionMode) {
      log("ECC Encryption selected", 1);
      delay(2000);
      dataMsg = MSGPayload.createDataMsg(getIDDevice.get_UID(), RTC.getEpoch(), loadCell.get_LastWeightReading(), SysStatusByte);
      msgToSendHex = MSGPayload.bytesToHexString(dataMsg, MSGPayload.get_MsgSize());
      log("dataMsg HEX: " + msgToSendHex, 1);

      // Genera la chiave privata e pubblica con tinyECC
      const struct uECC_Curve_t *curve = uECC_secp160r1();
      uint8_t private_key[21];
      uint8_t public_key[40];
      uECC_make_key(public_key, private_key, curve);

      // Condividi la chiave pubblica con l'altro endpoint se necessario

      // Crittografa il messaggio utilizzando tinyECC
      uint8_t ciphertext[MSGPayload.get_MsgSize()];
      int encrypted = uECC_encrypt(public_key, (const uint8_t *)msgToSendHex.c_str(), MSGPayload.get_MsgSize(), ciphertext, curve);

      if (encrypted == 1) {
        // Crea una stringa esadecimale a partire dal testo crittografato
        String encryptedHex = MSGPayload.bytesToHexString(ciphertext, MSGPayload.get_MsgSize());
        log("Encrypted Text HEX: " + encryptedHex, 1);

        // Decrittografia del messaggio (per dimostrazione)
        uint8_t decrypted[MSGPayload.get_MsgSize()];
        int decrypted_size = uECC_decrypt(private_key, ciphertext, MSGPayload.get_MsgSize(), decrypted, curve);

        if (decrypted_size > 0) {
          String decryptedText = String((char *)decrypted, decrypted_size);
          log("Decrypted Text: " + decryptedText, 1);
        } else {
          log("Decryption failed!", 1);
        }
      } else {
        log("Encryption failed!", 1);
      }

      // Genera e verifica la firma ECC (per dimostrazione)
      uint8_t signature[64];
      uECC_sign(private_key, (const uint8_t *)msgToSendHex.c_str(), MSGPayload.get_MsgSize(), signature, curve);
      bool isSignatureValid = uECC_verify(public_key, (const uint8_t *)msgToSendHex.c_str(), MSGPayload.get_MsgSize(), signature, curve);

      String additionalInfo;
      if (isSignatureValid) {
        additionalInfo = " | Valid!";
      } else {
        additionalInfo = " | Not valid!";
      }

      log("Signature ECC: " + String((char *)signature, 64) + additionalInfo, 1);

      // Deallocazione delle risorse
      delete[] dataMsg;

    } else if (!EncryptionMode) {
      log("Ethereum-RPL encryption selected", 1);
      delay(2000);
      dataMsg = MSGPayload.createDataMsg(getIDDevice.get_UID(), RTC.getEpoch(), loadCell.get_LastWeightReading(), SysStatusByte);
      ethTx.CreateRawTransaction(dataMsg, MSGPayload.get_MsgSize());
      delete[] dataMsg;
    }
    break;
    */

    case 'q':
      dataMsg = MSGPayload.createDataMsg(getIDDevice.get_UID(), RTC.getEpoch(), loadCell.get_LastWeightReading(), SysStatusByte);
      msgToSendStr = MSGPayload.bytesToString(dataMsg, MSGPayload.get_MsgSize());
      msgToSendHex = MSGPayload.bytesToHexString(dataMsg, MSGPayload.get_MsgSize());
      delete[] dataMsg;
      log("msgToSend:  \n\t[ASCII]: " + String(msgToSendStr) + "\n\t[HEX]: " + String(msgToSendHex), 1);
    break;

    case 'S':
      LoRaWANManager.begin();
    break;
    
    case 's':
      LoRaWANManager.sendMessage(999, 946686173, 100, 1);
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

    case 'b':
      loadCell.calibrate(bottleWeight);
      break;

    case 'z':
      loadCell.setScale(bottleWeight);
      break;

    case 'T':
      gnssHandler.updateRTCViaGNSS();
      break;

    case 'f':
      log("reading intMemory info...", 1);
      intMemory.init();      // start SPI
      intMemory.powerUp();   // MX25R6435FZAI defaults to power down state
      intMemory.getChipID(); // Verify SPI flash communication
      log("flash_read_status: " + String(intMemory.flash_read_status()), 1);
      intMemory.powerDown(); // power down SPI flash
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
      getVDDA();
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
