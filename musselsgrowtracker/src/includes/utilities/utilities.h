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

extern intLED intBlueLED;
extern WireScan i2cScan;

#endif // UTILITIES_H
