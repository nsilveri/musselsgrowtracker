// utilities.cpp

#include "utilities.h"
#include <STM32L0.h>

bool DEBUG_MODE;
uint8_t LOG_LEVEL;

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
