#include "GNSSFunctions.h"
#include "GNSS.h"
#include <Arduino.h>
#include <Wire.h>
#include <math.h>
#include "utilities.h"
#include "convertDateToUnixTime.h"


// CAM M8Q GNSS configuration
#define GNSS_en      5     // enable for GNSS 3.0 V LDO
#define pps          4     // 1 Hz fix pulse
#define GNSS_backup A0     // RTC backup for MAX M8Q

LocationData invalidLocVal = {0.0000000, 0.0000000, 0.000, 0.000};
LocationData prevLocVal = invalidLocVal;
LocationData currLocVal = invalidLocVal;

void GNSSHandler::read_positioning_data() {
  GNSSLocation currentLocation = gnssHandler.getLocation();

  currLocVal.lat = currentLocation.latitude();
  currLocVal.lon = currentLocation.longitude();
  currLocVal.alt = currentLocation.altitude();
  currLocVal.satNum = currentLocation.satellites();

  char currLocStr[100];
  char prevLocStr[100];
  char distLoc[128];  // Assicurati che la dimensione della stringa sia sufficiente

  double distance = -1.0;

  if (prevLocVal != invalidLocVal && currLocVal != invalidLocVal) {
    distance = gnssHandler.calculateDistance(currLocVal, prevLocVal);
  }

  currLocVal.shift = distance;

snprintf(currLocStr, sizeof(currLocStr), "CURRENT LOCATION: %.7f,%.7f,%.3f SATELLITES=%d SHIFT: %.2f meters",
           currLocVal.lat, currLocVal.lon, currLocVal.alt, currLocVal.satNum, currLocVal.shift);
snprintf(prevLocStr, sizeof(prevLocStr), "PREVIOUS LOCATION: %.7f,%.7f,%.3f SATELLITES=%d SHIFT: %.2f meters",
           prevLocVal.lat, prevLocVal.lon, prevLocVal.alt, prevLocVal.satNum, prevLocVal.shift);
snprintf(distLoc, sizeof(distLoc), "SHIFT: %.2f meters", distance);

  log(currLocStr, 1); //print current position data
  log(prevLocStr, 1); //print last position data
  log(distLoc, 1);    //print dist beetwen positions

  if (currLocVal != prevLocVal) {
    log("OLD GNSS VAL != NEW GNSS VAL!", 2);
    prevLocVal.lat = currLocVal.lat;
    prevLocVal.lon = currLocVal.lon;
    prevLocVal.alt = currLocVal.alt;
    prevLocVal.satNum = currLocVal.satNum;
  }
}

GNSSHandler::GNSSHandler() {
  tracking = false;
}

void GNSSHandler::begin() {
  pinMode(GNSS_en, OUTPUT);
  digitalWrite(GNSS_en, HIGH); // enable GNSS

  Wire.begin();
  Wire.setClock(400000);

  GNSS.begin(Serial1, GNSS.MODE_UBLOX, GNSS.RATE_1HZ);
  while (GNSS.busy()) { }

  GNSS.setConstellation(GNSS.CONSTELLATION_GPS_AND_GLONASS);
  while (GNSS.busy()) { }

  GNSS.setAntenna(GNSS.ANTENNA_INTERNAL);
  while (GNSS.busy()) { }
}

void GNSSHandler::update() {
  // Your existing GNSS update code here
}

void GNSSHandler::enable() {
  digitalWrite(GNSS_en, HIGH);
  tracking = false;
}

void GNSSHandler::disable() {
  digitalWrite(GNSS_en, LOW);
}

bool GNSSHandler::isTracking() {
  return tracking;
}

GNSSLocation GNSSHandler::getLocation() {
  GNSSLocation location;
  GNSS.location(location);
  return location;
}

double GNSSHandler::toRadians(double degree) {
  return degree * M_PI / 180.0;
}

double GNSSHandler::calculateDistance(const LocationData& loc1, const LocationData& loc2) {
  // Formula dell'emisenoverso (Haversine) per calcolare la distanza tra due coordinate geografiche
  constexpr double earthRadius = 6371000.0;  // Raggio medio della Terra in metri

  double dLat = toRadians(loc2.lat - loc1.lat);
  double dLon = toRadians(loc2.lon - loc1.lon);

  double a = sin(dLat / 2.0) * sin(dLat / 2.0) +
             cos(toRadians(loc1.lat)) * cos(toRadians(loc2.lat)) *
             sin(dLon / 2.0) * sin(dLon / 2.0);

  double c = 2 * atan2(sqrt(a), sqrt(1 - a));

  double distance = earthRadius * c;

  return distance;
}

void GNSSHandler::readGpsTime() {
  GNSSLocation currentLocation = gnssHandler.getLocation();

  if (currentLocation) {
    uint16_t year = currentLocation.year();
    uint8_t month = currentLocation.month();
    uint8_t day = currentLocation.day();
    uint8_t hours = currentLocation.hours();
    uint8_t minutes = currentLocation.minutes();
    uint8_t seconds = currentLocation.seconds();
    uint16_t milliseconds = currentLocation.millis();

    unsigned long timestamp = (year, month, day, hours, minutes, seconds);

    char gpsTimeStr[50];
    snprintf(gpsTimeStr, sizeof(gpsTimeStr), "GPS Time: %04u-%02u-%02u %02u:%02u:%02u.%03u",
            year, month, day, hours, minutes, seconds, milliseconds);

    log(gpsTimeStr, 1);
    log(String(timestamp), 1);

  } else {
    log("Satellite fix still in progress...",1);
  }
}

GNSSHandler gnssHandler;