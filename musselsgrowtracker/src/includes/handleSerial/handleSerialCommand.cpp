// handleSerialCommand.cpp

#include "handleSerialCommand.h"
#include "..\GNSSFunctions\GNSSFunctions.h"
#include "..\SPI\SPIFlash.h"
#include <STM32L0.h>
#include "..\BMA400\BMA400.h"
#include "..\loadCell\loadCell.h"
#include "..\utilities\utilities.h"

bool DEBUG = true;
uint8_t LOG_LEV =1;
volatile uint8_t Step = 0;
float bottleWeight = 1000.0;

void handleSerialCommand(char command) {
  switch (command) {
    case 's':
      log("Received manual command!", 1);
      log("COMMAND: " + String(command), 2);
      Step = 1;
      break;

    case 'g':
      log("COMMAND: " + String(command), 2);
      gnssHandler.readPositioningData();
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
      //BMA400::initBMA400();
      break;

    case 'i':
      log("I2C bus scanning...", 1);
      i2cScan.begin();
      i2cScan.scan();
      break;

    case 'm':
      bool mosState;
      mosState = loadCell.turn_on_off();
      log("Mosfet state: " + String(mosState), 1);
      break;

    case 'c':
      loadCell.read_weight();
      break;

    case 'x':
      loadCell.setOffset();
      break;

    case 'b':
      loadCell.calibrate(bottleWeight);
      break;

    case 'z':
      loadCell.setScale(bottleWeight);
      break;

    case 'f':
      log("reading intMemory info...", 1);
      intMemory.init();
      intMemory.powerUp();
      intMemory.getChipID();
      intMemory.powerDown();
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
