#include <STM32L0.h>
#include <Arduino.h>
#include "src\includes\utilities\utilities.h"
#include "src\includes\GNSSFunctions\GNSSFunctions.h"
#include "src\includes\handleSerial\handleSerialCommand.h"
#include "src\includes\loadCell\loadCell.h"
#include "src\includes\SPI\SPIFlash.h"
//#include "src\includes\intRTC.h"
#include <RTC.h>

#define SLEEP 0
#define RTC_INT 1
#define FIX_WAIT 2
#define DATA_GATERED 3
#define LORAW_SENDING 4
#define TIMEOUT_EXE 99

#define MOV_TOLERANCE 5

bool SerialDebug = true;

unsigned long previousMillis = 0; 
const long interval = 1000;

const uint16_t SecondsBetweenActivations = 30;

void setNextAlarmIn(const uint32_t &seconds) {
  RTC.setAlarmEpoch(RTC.getEpoch() + seconds);
}

void setup() {
  Serial.begin(9600);

  setDebugMode(DEBUG);
  setDebugLevel(LOG_LEV);

  intMemory.init();
  loadCell.begin();
  intBlueLED.begin();

  gnssHandler.begin();  // Initialize GNSS
  gnssHandler.enable();

  Step = 0;

  // set alarm to update the RTC periodically
  //setNextAlarmIn(SecondsBetweenActivations);
  //RTC.enableAlarm(RTC.MATCH_YYMMDDHHMMSS);
  //RTC.attachInterrupt(alarmMatch);
}

void alarmMatch()
{
	if( SerialDebug )
		log("\t\t\t\t\t{ ALARM! }", 1);
	// RTC alarm occurred, time to wakeup for duty!
	// Relaunch RTC for next wakeup and restart the job
	Step = RTC_INT;
	setNextAlarmIn(SecondsBetweenActivations);
	STM32L0.wakeup();
}

void gnssDataAcquisition()
{
  //while(gnssHandler.readPositioningData() == false)
  //{
    gnssHandler.readPositioningData();
  //}
  if(gnssHandler.displacementAlert(currLocVal.displacement, MOV_TOLERANCE))
  {
    log("The buoy has been moved by " + String(currLocVal.displacement) + " meters", 1);
    //associare bit alarm displacement
  }
}

void run()
{
  //GNSS data acquisition
  //sync RTC with GNSS
  //reading the load cell value
}

void loop() {
  
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    intBlueLED.intLED_on_off();
    log(String(Step),2);
  }

  /*
  switch (Step) {
    case 0:
      STM32L0.stop(10000); //stop per 1 ora
      Serial.println("Case 0");
      return;
      break;

    case 1:
      gnssHandler.enable();  // Enable GNSS
      break;
  }
  */

  gnssHandler.update();  // Update GNSS data

  if (gnssHandler.isTracking()) {
    gnssHandler.readPositioningData();
  }

  if (Serial.available()) {
    char c = Serial.read();

    handleSerialCommand(c);
  }
}