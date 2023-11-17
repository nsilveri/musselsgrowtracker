// utilities.h

#ifndef UTILITIES_H
#define UTILITIES_H

#include <Arduino.h>

extern bool DEBUG_MODE;
extern uint8_t LOG_LEVEL;
extern volatile uint8_t Step;
extern bool DEBUG;

struct DebugState {
  bool mode;
  int level;
};

void setDebugMode(bool value);
void setDebugLevel(uint8_t value);
void log(String log, uint8_t LOG_LEVEL);

#endif // UTILITIES_H
