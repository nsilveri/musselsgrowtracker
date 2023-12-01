#include "GNSSFunctions.h"
#include "GNSS.h"
#include <Arduino.h>
#include <Wire.h>
#include <math.h>
#include "..\utilities\utilities.h"
#include "dateConverter.h"
//#include "intRTC.h"
#include <RTC.h>


// CAM M8Q GNSS configuration
#define GNSS_en      5     // enable for GNSS 3.0 V LDO
#define pps          4     // 1 Hz fix pulse
#define GNSS_backup A0     // RTC backup for MAX M8Q

const uint8_t tolerance = 5;
const uint8_t RTC_GPStolerance = 5;

unsigned long GNStimestamp = 0;
unsigned long RTCtimestamp = 0;

LocationData invalidLocVal = {0.0000000, 0.0000000, 0.000, 0.000};
LocationData prevLocVal = invalidLocVal;
LocationData currLocVal = invalidLocVal;

bool GNSSHandler::displacementAlert(uint8_t movement, uint8_t tolerance)
{
  if(movement >= tolerance)
  {
    log("Displacement alarm!!!", 1);
    return true;
  }return false;
}

void GNSSHandler::savePreviousPosition()
{
  prevLocVal.lat = currLocVal.lat;
  prevLocVal.lon = currLocVal.lon;
  prevLocVal.alt = currLocVal.alt;
  prevLocVal.satNum = currLocVal.satNum;
}

bool GNSSHandler::readPositioningData() {
  GNSSLocation currentLocation = gnssHandler.getLocation();
  log("currentLocation: " + String(currentLocation), 2);

  savePreviousPosition();

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

  currLocVal.displacement = distance;

  if(DEBUG_MODE && currentLocation)
  {
    snprintf(currLocStr, sizeof(currLocStr), "CURRENT LOCATION: %.7f,%.7f,%.3f SATELLITES=%d DISPLACEMENT: %.2f meters",
              currLocVal.lat, currLocVal.lon, currLocVal.alt, currLocVal.satNum, currLocVal.displacement);
    snprintf(prevLocStr, sizeof(prevLocStr), "PREVIOUS LOCATION: %.7f,%.7f,%.3f SATELLITES=%d DISPLACEMENT: %.2f meters",
              prevLocVal.lat, prevLocVal.lon, prevLocVal.alt, prevLocVal.satNum, prevLocVal.displacement);
    snprintf(distLoc, sizeof(distLoc), "DISPLACEMENT: %.2f meters", distance);

    log(currLocStr, 1); //print current position data
    log(prevLocStr, 1); //print last position data
    log(distLoc, 1);    //print dist beetwen positions
    return true;
  }else if(!currentLocation)
  {
    log("GNSS still fixing...",1);
    return false;
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

void GNSSHandler::printRTC()
{
  log("RTC Time: " + String(DateConverter.convertUnixTimeToDate(RTCtimestamp).year) + "-" +
                  String(DateConverter.convertUnixTimeToDate(RTCtimestamp).month) + "-" +
                  String(DateConverter.convertUnixTimeToDate(RTCtimestamp).day) + " " +
                  String(DateConverter.convertUnixTimeToDate(RTCtimestamp).hours) + ":" +
                  String(DateConverter.convertUnixTimeToDate(RTCtimestamp).minutes) + ":" +
                  String(DateConverter.convertUnixTimeToDate(RTCtimestamp).seconds) , 1);
  log(String(RTCtimestamp), 1);
}

void GNSSHandler::readGpsTime() {

  RTCtimestamp = RTC.getEpoch();
  GNSSLocation currentLocation = gnssHandler.getLocation();

  if (currentLocation) {
    uint16_t year = currentLocation.year();
    uint8_t month = currentLocation.month();
    uint8_t day = currentLocation.day();
    uint8_t hours = currentLocation.hours();
    uint8_t minutes = currentLocation.minutes();
    uint8_t seconds = currentLocation.seconds();
    uint16_t milliseconds = currentLocation.millis();

    GNStimestamp = DateConverter.convertDateToUnixTime(year, month, day, hours, minutes, seconds);

    char gpsTimeStr[50];
    snprintf(gpsTimeStr, sizeof(gpsTimeStr), "GPS Time: %04u-%02u-%02u %02u:%02u:%02u.%03u",
            year, month, day, hours, minutes, seconds, milliseconds);
    
    log(gpsTimeStr, 1);
    printRTC();

    log(String(GNStimestamp), 1);

  } else {
    log("GNSS still fixing...", 1);
    printRTC();
    
  }
}

void GNSSHandler::updateRTCViaGNSS()
{
  RTCtimestamp = RTC.getEpoch();
  log("RTCts= " + String(RTCtimestamp), 1);
  log("GNSts= " + String(GNStimestamp), 1);
  log("Difference= " + String(abs(RTCtimestamp - GNStimestamp)) + "s", 1);

  if(abs(RTCtimestamp - GNStimestamp) > RTC_GPStolerance)
  {
    log("The RTC time is not synced with the GNSS time", 1);
    log("Synching...", 1);
    RTC.setEpoch(GNStimestamp);
    log("RTC time was synced.", 1);

  }
}

GNSSHandler gnssHandler;