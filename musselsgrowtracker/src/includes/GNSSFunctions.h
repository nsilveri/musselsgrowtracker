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
  float shift;

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
  void read_positioning_data();
  void readGpsTime();
private:
  volatile bool tracking;
  double toRadians(double degree);
};

extern GNSSHandler gnssHandler;

#endif  // GNSS_FUNCTIONS_H