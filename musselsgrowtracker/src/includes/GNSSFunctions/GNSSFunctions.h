#ifndef GNSS_FUNCTIONS_H
#define GNSS_FUNCTIONS_H

#include <Arduino.h>
#include <STM32L0.h>
#include "GNSS.h"
#include <RTC.h>
#include "convertDateToUnixTime.h"

struct LocationData {
  double lat;
  double lon;
  float alt;
  uint8_t satNum;
  float displacement;

  bool operator!=(const LocationData& other) const {
    return lat != other.lat || lon != other.lon || alt != other.alt || satNum != other.satNum;
    }
  bool operator==(const LocationData& other) const {
    return lat == other.lat && lon == other.lon && alt == other.alt && satNum == other.satNum;
  }
};

extern LocationData invalidLocVal;
extern LocationData prevLocVal;
extern LocationData currLocVal;
extern const uint8_t tolerance;

class GNSSHandler {
public:
  GNSSHandler();
  void begin();
  void update();
  void enable();
  void disable();
  bool isTracking();
  GNSSLocation getLocation();
  double calculateDistance(const LocationData& loc1, const LocationData& loc2);
  bool readPositioningData();
  void readGpsTime();
  void updateRTCViaGNSS();
  bool displacementAlert(uint8_t movement, uint8_t tolerance);
private:
  volatile bool tracking;
  double toRadians(double degree);
};

extern GNSSHandler gnssHandler;

#endif  // GNSS_FUNCTIONS_H