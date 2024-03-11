#include "handleSerialCommand.h"
#include "..\GNSSFunctions\GNSSFunctions.h"
#include <STM32L0.h>
#include "..\loadCell\loadCell.h"
#include "..\utilities\utilities.h"
#include "..\LoRa\LoRaWANLib.h"
#include "..\ECCProcessor\ECCProcessor.h"
#include "..\batteryManager\batteryMan.h"
#include "..\ECCProcessor\sha256.h"

#include <cstring>

#define csPin 25

bool DEBUG = true;
uint8_t LOG_LEV =1;
volatile uint8_t Step = 0;
float bottleWeight = 1000.0;
float accelData[3];

bool EncryptionMode = true;

bool GNSS_RESUME_SUSPEND = false;
bool enableRoutine = true;

uint32_t UID[3] = {0, 0, 0};
byte   SysStatusByte;

const char* message;
size_t messageLen = strlen(message);

uint8_t hash_to_sign[32];

void send_lora(bool test)
{
  MSGPayload.sendMergedMsg(test);
  delay(3000);
}

void handleSerialCommand(char command) {

  switch (command) {
    case 'u':
      log(uidCode.get_UID_String(), 1);
    break;

    case 'r':
      log("Data on EEPROM: " + String(gnssEeprom.readLatitude(), 6) + ", " + String(gnssEeprom.readLongitude(), 6), 1);
    break;
    
    case 'y':
      msgService.sendMsg();
    
    break;

    case 'q':
      MSGPayload.createDataMsg(getIDDevice.get_UID(), RTC.getEpoch(), gnssHandler.generateLoRaMsg(), loadCell.get_LastWeightReading(), bytePack.packData());

      log("msgToSend:  \n\t[HEX]: " + String(MSGPayload.get_msgHex()) + "\n\t[HASH]: " + String(MSGPayload.get_msgHashString()), 1);
      eccProcessor.sign_message(MSGPayload.get_msgHashUint8());
    break;

    case 'S':
      send_lora(true);
    break;
    
    case 's':
      send_lora(false);
      break;

    case 'I':
      LoRaWANManager.getInfo();
      break;

    case 'g':
      log("COMMAND: " + String(command), 2);
      gnssHandler.update();
      break;

    case 'G':
      log("COMMAND: " + String(command), 2);
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
      break;

    case 'z':
      loadCell.setScale(bottleWeight);
      break;

    case 'T':
      gnssHandler.updateRTCViaGNSS();
      break;

    case 'F':
      log("Change fix type...", 1);
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
