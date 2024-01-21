#ifndef GNSS_HANDLER_H
#define GNSS_HANDLER_H

#include "library\GNSS.h"
#include "dateConverter.h"
#include <RTC.h>
#include <stm32l0_gpio.h>
#include <stm32l0_exti.h>

#define INITIAL_RTC_YEAR 2000

struct LocationData {
    double  lat;
    double  lon;
    double  alt;
    int    satNum;
    float displacement;

    bool operator!=(const LocationData& other) const {
    return lat != other.lat || lon != other.lon || alt != other.alt || satNum != other.satNum;
    }

    bool operator==(const LocationData& other) const {
      return lat == other.lat && lon == other.lon && alt == other.alt && satNum == other.satNum;
    }
};

struct LocationEEPROMData {
    double  lat;
    double  lon;
};

class GNSSHandler {
public:
    //void    initializeGNSSPins();
    void    restoreOldLoc();
    void    initializeArduinoGNSSPins();
    void    initializeStmGNSSPins();
    void    configureGNSS();
    bool    toggleGNSS(bool enable);
    bool    displacementAlert(uint8_t movement, uint8_t tolerance);
    uint8_t getSatNumbers();
    void    savePreviousPosition();
    bool    readPositioningData();
    GNSSLocation getLocation();
    double  toRadians(double degree);
    double  calculateDistance(const LocationData& loc1, const LocationData& loc2);
    bool    distanceEepromCurrentLoc();
    String  getRTCString();
    void    printRTC();
    bool    RTCFix();
    void    readGpsTime();
    void    updateRTCViaGNSS();
    void    update();
    bool    getGNSSModuleState();
    bool    getIsLocationFixed();
    bool    setGNSSStatus(bool STATUS);
    bool    isTracking();
    double  getInvalidLat();
    double  getInvalidLon();
    uint8_t setFixQuality(uint8_t QUALITY_TYPE);
    uint8_t setFixLocType(uint8_t FIX_TYPE);
    void saveOnEeprom(double lat = 0, double lon = 0);
    void restoreFromEeprom(double* lat, double* lon);

private:
    bool tracking;
    //timerManager gnssTimer;
    //GNSSLocation currentLocation;
    //GNSSSatellites GNSSSat;
    //GNSSClass GNSS;
    const uint8_t tolerance = 5;
    const uint8_t RTC_GPStolerance = 5;

    unsigned long GNStimestamp;
    unsigned long RTCtimestamp;

    bool isLocationFixed;
    bool GNSS_MODULE_ON_OFF;

    LocationData invalidLocVal;
    LocationData prevLocVal;
    LocationData currLocVal;
};

extern GNSSHandler gnssHandler;

#endif // GNSS_HANDLER_H
