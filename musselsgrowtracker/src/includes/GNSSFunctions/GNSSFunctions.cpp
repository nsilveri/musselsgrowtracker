#include "GNSSFunctions.h"
#include "library\GNSS.h"
#include <Arduino.h>
#include <Wire.h>
#include "..\utilities\utilities.h"
#include "dateConverter.h"
#include <RTC.h>
#include "..\utilities\utilities.h"
//#include "..\..\STM32\STM32L0\system\STM32L0xx\Include\stm32l0_gpio.h"
//#include "..\..\STM32\STM32L0\system\STM32L0xx\Include\stm32l0_exti.h"
//#include "library\stm32\include\stm32l0_exti.h"
//#include "library\stm32\include\stm32l0_gpio.h"
#include <stm32l0_exti.h>
#include <stm32l0_gpio.h>
#include <array>


#define GNSS_ENABLE  (5u)
#define GNSS_PPS     (4u)
#define GNSS_BACKUP  A0

#define STM32L0_EXTI_CONTROL_PRIORITY_CRITICAL  0x00000000

#define EPE_TOLLERANCE 15
#define DISTANCE_TOLLERANCE 30

uint16_t stm32_enable_pin = g_APinDescription[GNSS_ENABLE].pin;
uint16_t stm32_pps_pin    = g_APinDescription[GNSS_PPS].pin;
uint16_t stm32_backup_pin = g_APinDescription[GNSS_BACKUP].pin;

volatile bool pps_irq;

timerManager gnssTimer;
timerManager gnssDistanceTimer;

GNSSLocation currentLocation;
GNSSSatellites GNSSSat;
GNSSClass GNSS;

const uint8_t tolerance = 5;
const uint8_t RTC_GPStolerance = 5;

unsigned long GNStimestamp = 0;
unsigned long RTCtimestamp = 0;

bool isLocationFixed = false;
bool GNSS_MODULE_ON_OFF = true;

bool GNSS_TOGGLE = true;

extern bool GNSS_DISPLACEMENT_ALARM = false;

LocationData invalidLocVal = {0.0000000, 0.0000000, 0.000, 0.000};
LocationData prevLocVal = invalidLocVal;
LocationData currLocVal = invalidLocVal;
LocationData eepromLocVal = invalidLocVal;
LocationEEPROMData LocValOnEEPROM;

void int_pps(void) {
  pps_irq = true;
}

bool GNSSHandler::getDisplacementStatus() {
    return GNSS_DISPLACEMENT_ALARM;
}

void GNSSHandler::initializeArduinoGNSSPins() {
    pinMode(GNSS_ENABLE, OUTPUT);
    digitalWrite(GNSS_ENABLE, HIGH);

    pinMode(GNSS_PPS, INPUT);
    attachInterrupt(GNSS_PPS, int_pps, FALLING);

    pinMode(GNSS_BACKUP, OUTPUT);
    digitalWrite(GNSS_BACKUP, HIGH);
}

void GNSSHandler::restoreOldLoc() {
    eepromLocVal.lat = gnssEeprom.readLatitude();
    eepromLocVal.lon = gnssEeprom.readLongitude();
    log("prevLoc from EEPROM: " + String(prevLocVal.lat) + ", " + String(prevLocVal.lat), 1);
    delay(3000);
}

double  GNSSHandler::getInvalidLat() {
    return invalidLocVal.lat;
}
double  GNSSHandler::getInvalidLon() {
    return invalidLocVal.lon;
}

void GNSSHandler::initializeStmGNSSPins() {
  //backup pin
  stm32l0_gpio_pin_configure(stm32_backup_pin, (STM32L0_GPIO_PARK_NONE | STM32L0_GPIO_PUPD_PULLUP | STM32L0_GPIO_OSPEED_LOW | STM32L0_GPIO_OTYPE_PUSHPULL | STM32L0_GPIO_MODE_OUTPUT));
  stm32l0_gpio_pin_write(stm32_backup_pin, 1);

  //enable pin
  stm32l0_gpio_pin_configure(stm32_enable_pin, (STM32L0_GPIO_PARK_NONE | STM32L0_GPIO_PUPD_PULLUP | STM32L0_GPIO_OSPEED_LOW | STM32L0_GPIO_OTYPE_PUSHPULL | STM32L0_GPIO_MODE_OUTPUT));
  stm32l0_gpio_pin_write(stm32_enable_pin, 1);

  //pps pin
  stm32l0_gpio_pin_configure(stm32_pps_pin, (STM32L0_GPIO_PARK_NONE | STM32L0_GPIO_PUPD_PULLDOWN | STM32L0_GPIO_OSPEED_HIGH | STM32L0_GPIO_OTYPE_PUSHPULL | STM32L0_GPIO_MODE_INPUT));
  stm32l0_exti_attach(stm32_pps_pin, STM32L0_EXTI_CONTROL_EDGE_FALLING | STM32L0_EXTI_CONTROL_PRIORITY_CRITICAL, (stm32l0_exti_callback_t)&int_pps, (void*)NULL);
}


void GNSSHandler::configureGNSS(bool EXT_ANT = false) {

    GNSS.begin(Serial1, GNSS.MODE_UBLOX, GNSS.RATE_10HZ);
    toggleGNSS(true); //turn on GNSS

    GNSS.setConstellation(GNSS.CONSTELLATION_GPS_AND_GLONASS);
    while (GNSS.busy()) { }

    GNSS.setPlatform(GNSS.PLATFORM_SEA);
    while (GNSS.busy()) { }

    if(EXT_ANT) {
        GNSS.setAntenna(GNSS.ANTENNA_EXTERNAL);
    }else GNSS.setAntenna(GNSS.ANTENNA_INTERNAL);
    
    while (GNSS.busy()) { }
}

bool GNSSHandler::toggleGNSS(bool enable) {
  if(GNSS_TOGGLE) 
  {
    if (enable && !GNSS_MODULE_ON_OFF) 
    {
      GNSS_MODULE_ON_OFF = true;
      GNSS.resume();
    
    } else if (!enable && GNSS_MODULE_ON_OFF) 
    {
      GNSS_MODULE_ON_OFF = false;
      GNSS.suspend();
    }
    return GNSS_MODULE_ON_OFF;
  }else return GNSS_MODULE_ON_OFF = true;
}

bool GNSSHandler::displacementAlert(uint8_t movement, uint8_t tolerance) {
    if (movement >= tolerance) {
        log("Displacement alarm!!!", 1);
        //GNSS_DISPLACEMENT_ALARM = true;
        return true;
    }
    return false;
}

uint8_t GNSSHandler::getSatNumbers() {
    GNSS.satellites(GNSSSat);
    uint8_t _satNum = GNSSSat.count();
    log("satNum: " + String(_satNum), 2);
    return _satNum;
}

void GNSSHandler::savePreviousPosition() {
    prevLocVal.lat = currLocVal.lat;
    prevLocVal.lon = currLocVal.lon;
    prevLocVal.alt = currLocVal.alt;
    prevLocVal.satNum = currLocVal.satNum;
}

void GNSSHandler::saveOnEeprom(double lat, double lon) {
    if(lat == 0 && lon == 0) {
        gnssEeprom.saveGNSSCoordinates(currLocVal.lat, currLocVal.lon);
    }else gnssEeprom.saveGNSSCoordinates(lat, lon);
}

void GNSSHandler::restoreFromEeprom(double* lat, double* lon) {
    if (lat != nullptr) {
        *lat = gnssEeprom.readLatitude();
    }
    if (lon != nullptr) {
        *lon = gnssEeprom.readLongitude();
    }
}

bool GNSSHandler::readPositioningData() {
    GNSSLocation currentLocation = getLocation();
    log("currentLocation:\n\tLAT:" + String(currLocVal.lat, 6) + "\n\tLON:" + String(currLocVal.lon, 6), 1);

    
    savePreviousPosition();
    

    currLocVal.lat = currentLocation.latitude();
    currLocVal.lon = currentLocation.longitude();
    currLocVal.alt = currentLocation.altitude();
    currLocVal.satNum = currentLocation.satellites();

    char currLocStr[100];
    char prevLocStr[100];
    char distLoc[128];
    char satNumStr[128];

    double distance = -1.0;

    if (prevLocVal != invalidLocVal && currLocVal != invalidLocVal) {
        distance = calculateDistance(currLocVal, prevLocVal);
        float EPE = currentLocation.ehpe();
        log("\tEPE: " + String(EPE), 1);
        if(EPE > EPE_TOLLERANCE){
          return false;
        }
        gnssDistanceTimer.startTimer();
    } else {
        return false;
    }

    currLocVal.displacement = distance;

    if(distance <= 2)
    {
        gnssEeprom.saveGNSSCoordinates(prevLocVal.lat, prevLocVal.lon);
    }

    if (DEBUG_MODE) {
        snprintf(currLocStr, sizeof(currLocStr), "CURRENT LOCATION: %.7f,%.7f,%.3f SATELLITES=%d DISPLACEMENT: %.2f meters",
                 currLocVal.lat, currLocVal.lon, currLocVal.alt, currLocVal.satNum, currLocVal.displacement);
        snprintf(prevLocStr, sizeof(prevLocStr), "PREVIOUS LOCATION: %.7f,%.7f,%.3f SATELLITES=%d DISPLACEMENT: %.2f meters",
                 prevLocVal.lat, prevLocVal.lon, prevLocVal.alt, prevLocVal.satNum, prevLocVal.displacement);
        snprintf(distLoc, sizeof(distLoc), "DISPLACEMENT: %.2f meters", distance);
        snprintf(satNumStr, sizeof(satNumStr), "Connected satellites: %d", getSatNumbers());

        log("GNSS Module info:\n\t" + String(currLocStr) + "\n\t" + String(prevLocStr) + "\n\t" + String(distLoc) + "\n\t" + String(satNumStr), 1);
    }

    return true;
}

bool GNSSHandler::isTracking() {
  return tracking;
}

GNSSLocation GNSSHandler::getLocation() {
    if(currentLocation.fixType() != GNSS_LOCATION_TYPE_3D) {
      currentLocation.setFixType(GNSS_LOCATION_TYPE_3D);
    }

    log("Fix -> Qty: " + String(currentLocation.fixQuality()) + " Type: " + String(currentLocation.fixType()), 2);
    GNSS.location(currentLocation);

    float EPE = currentLocation.ehpe();
    
    if (/*(EPE <= 30.0f) && */currentLocation.fullyResolved()) {
        if (!isTracking()) {
            tracking = true;
            log("***GNSS go to sleep!***", 1);
            toggleGNSS(false);
        }
    } else {
        log("GNSS accuracy not yet good.\n\tElapsed time: " + String(int(gnssTimer.elapsedTimer() / 1000)) + "s", 1);
    }

    return currentLocation;
}

double GNSSHandler::toRadians(double degree) {
    return degree * M_PI / 180.0;
}

double GNSSHandler::calculateDistance(const LocationData& loc1, const LocationData& loc2) {
    constexpr double earthRadius = 6371000.0;
    double dLat = toRadians(loc2.lat - loc1.lat);
    double dLon = toRadians(loc2.lon - loc1.lon);
    double a = sin(dLat / 2.0) * sin(dLat / 2.0) +
               cos(toRadians(loc1.lat)) * cos(toRadians(loc2.lat)) *
               sin(dLon / 2.0) * sin(dLon / 2.0);
    double c = 2 * atan2(sqrt(a), sqrt(1 - a));
    double distance = earthRadius * c;
    return distance;
}

String GNSSHandler::getRTCString() {
    String rtcTimeString = "RTC Time: " +
        String(DateConverter.convertUnixTimeToDate(RTCtimestamp).year) + "-" +
        String(DateConverter.convertUnixTimeToDate(RTCtimestamp).month) + "-" +
        String(DateConverter.convertUnixTimeToDate(RTCtimestamp).day) + " " +
        String(DateConverter.convertUnixTimeToDate(RTCtimestamp).hours) + ":" +
        String(DateConverter.convertUnixTimeToDate(RTCtimestamp).minutes) + ":" +
        String(DateConverter.convertUnixTimeToDate(RTCtimestamp).seconds);
    return rtcTimeString;
}

void GNSSHandler::printRTC() {
    String rtcString = getRTCString();
    log(rtcString + " TS: " + String(RTCtimestamp), 1);
}

bool GNSSHandler::RTCFix() {
    RTCtimestamp = RTC.getEpoch();
    uint16_t fixControl = DateConverter.convertUnixTimeToDate(RTCtimestamp).year;
    if (fixControl > INITIAL_RTC_YEAR) {
        log("timeSatFix 1: " + String(fixControl), 2);
        return true;
    } else {
        log("timeSatFix 2: " + String(fixControl), 2);
    }
    return false;
}

void GNSSHandler::intToByteArray(int val, byte* byteArray) {
  union {
    int intValue;
    byte byteValue[4];
  } converter;

  converter.intValue = val;

  for (int i = 0; i < 4; i++) {
    byteArray[i] = converter.byteValue[i];
  }
}

byte*  GNSSHandler::generateLoRaMsg() {
  double latDouble = currLocVal.lat;
  double lonDouble = currLocVal.lon;

  int latInt = static_cast<int>(latDouble * 10000000.0);
  int lonInt = static_cast<int>(lonDouble * 10000000.0);

  byte latBytes[4];
  byte lonBytes[4];

  intToByteArray(latInt, latBytes);
  intToByteArray(lonInt, lonBytes);

  byte coordinatesBytes[8];

  for (int i = 0; i < 4; ++i) {
    coordinatesBytes[i] = latBytes[i];
    coordinatesBytes[i + 4] = lonBytes[i];
  }

  return coordinatesBytes;
}


void GNSSHandler::readGpsTime() {
    RTCtimestamp = RTC.getEpoch();
    currentLocation = getLocation();

    if (RTCFix()) {
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
        
        log(String(gpsTimeStr) + " TS:" + String(GNStimestamp), 1);
        printRTC();

    } else {
        log("READ GNSS TS: GNSS still fixing..." + String(currentLocation.year()) + " RTCFix: " + String(RTCFix()) + "\n\t" + String(getRTCString()), 1);
        printRTC();
    }
}

void GNSSHandler::updateRTCViaGNSS() {
    RTCtimestamp = RTC.getEpoch();
    log("RTCts= " + String(RTCtimestamp), 1);
    log("GNSts= " + String(GNStimestamp), 1);
    log("Difference= " + String(abs(RTCtimestamp - GNStimestamp)) + "s", 1);

    if (abs(RTCtimestamp - GNStimestamp) > RTC_GPStolerance) {
        log("The RTC time is not synced with the GNSS time", 1);
        log("Synching...", 1);
        RTC.setEpoch(GNStimestamp);
        log("RTC time was synced.", 1);
    }
}

bool GNSSHandler::distanceEepromCurrentLoc()
{
    float distance = calculateDistance(eepromLocVal, prevLocVal);
    if(distance >= DISTANCE_TOLLERANCE) {
        log("ALARM! displacement eepromLocVal <=> currLocVal > DISTANCE_TOLLERANCE (" + String(distance) + ")", 1);
        delay(2000);
        GNSS_DISPLACEMENT_ALARM = true;
        return true;
    }
    GNSS_DISPLACEMENT_ALARM = false;
    return false;
}

void GNSSHandler::update() {
    uint8_t SatNum = GNSSSat.count();

    log("UPDATE: GNSS_MODULE_ON_OFF: " + String(GNSS_MODULE_ON_OFF), 2);

    if (!GNSS_MODULE_ON_OFF) {
        log("Enabling GNSS...", 1);
        toggleGNSS(true);
    }

    isLocationFixed = readPositioningData();

    if (isLocationFixed) {
        if(gnssTimer.getTimerStatus()) {
          gnssTimer.setDisable();
          gnssTimer.saveElapsedTimer();
        }
        log("READ GNSS POS: \n\t" + String(getSatNumbers()) + " satellites found."
            + "\n\tisLocationFixed: " + String(isLocationFixed)
             + "\n\tGNSS data acquired in " + String(int(gnssTimer.getSavedElapsedTime()) / 1000) + "s.", 1);
        
        float EPE = currentLocation.ehpe();

        if (EPE < 15.0)
            if(distanceEepromCurrentLoc()) {
                GNSS_DISPLACEMENT_ALARM = true;
            }
            toggleGNSS(false);
            return;
    }

    if (!isLocationFixed) {
        if(!gnssTimer.getTimerStatus()){
          gnssTimer.startTimer();
        }

        log("READ GNSS POS: \n\tGNSS still fixing..." + String(getSatNumbers()) + " satellites found." + "\n\tisLocationFixed: " + String(isLocationFixed), 1);
        return;
    }
}

bool GNSSHandler::getGNSSModuleState() {
    return GNSS_MODULE_ON_OFF;
}

bool GNSSHandler::getIsLocationFixed() {
    return isLocationFixed;
}

bool GNSSHandler::setGNSSStatus(bool STATUS) {
    isLocationFixed = STATUS;
}

uint8_t GNSSHandler::setFixQuality(uint8_t QUALITY_TYPE) {
    return currentLocation.setQualityType(QUALITY_TYPE);
}

uint8_t GNSSHandler::setFixLocType(uint8_t FIX_TYPE) {
    return currentLocation.setFixType(FIX_TYPE);
}

GNSSHandler gnssHandler;