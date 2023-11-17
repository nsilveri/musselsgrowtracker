// utilities.cpp

#include "utilities.h"
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

void log(String log, uint8_t LEVEL_MSG) {
    if(DEBUG_MODE){
        if(LEVEL_MSG == 1 && LOG_LEVEL >= 1){
            Serial.println("LOG1: " + log);
        }if(LEVEL_MSG == 2 && LOG_LEVEL >= 2){
            Serial.println("LOG2: " + log);
        }
    }
}

/*
template <typename T>
void log(const T &log, uint8_t LEVEL_MSG) {
    if(DEBUG_MODE){
        if(LEVEL_MSG == 1 && LOG_LEVEL >= 1){
            Serial.print("LOG1: ");
            Serial.println(log);
        } else if(LEVEL_MSG == 2 && LOG_LEVEL >= 2){
            Serial.print("LOG2: ");
            Serial.println(log);
        }
    }
}
*/
