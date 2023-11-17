// handleSerialCommand.cpp

#include "handleSerialCommand.h"
#include "GNSSFunctions.h"
#include "SPIFlash.h"
#include <STM32L0.h>

bool DEBUG = true;
uint8_t LOG_LEV =1;
volatile uint8_t Step = 0;

void handleSerialCommand(char command) {
  switch (command) {
    case 's':
      log("Received manual command!", 1);
      log("COMMAND: " + String(command), 2);
      Step = 1;
      break;

    case 'g':
      log("COMMAND: " + String(command), 2);
      gnssHandler.read_positioning_data();
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

    case 'h':
    case '\0':
      log("HELP :\n \t\t\t-s) Trigger GNSS reading manually\n \t\t\t-g) Read Device GPS data\n \t\t\t-d) Enable/disable debug\n \t\t\t-p) deep-sleep test\n \t\t\t-1) Set log level 1\n \t\t\t-2) Set log level 2", 1);
      break;

    default:
      log("invalid command!", 1);
  }
}
