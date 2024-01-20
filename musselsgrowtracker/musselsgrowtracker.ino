#include <STM32L0.h>
#include <Arduino.h>
#include "src\includes\utilities\utilities.h"
#include "src\includes\GNSSFunctions\GNSSFunctions.h"
#include "src\includes\handleSerial\handleSerialCommand.h"
#include "src\includes\loadCell\loadCell.h"
#include "src\includes\SPI\SPIFlash.h"
#include "src\includes\LoRa\LoRaWANLib.h"
//#include "src\includes\BMA280\BMA280Functions.h"
#include "src\includes\GY521\GY521Sensor.h"
#include <RTC.h>

#define SLEEP 0
#define RTC_INT 1
#define FIX_WAIT 2
#define DATA_GATERED 3
#define LORAW_SENDING 4
#define TIMEOUT_EXE 99

#define intPin1 3
#define intPin2 2

#define MOV_TOLERANCE 5

#define csPin 25

//const char *appEui  = "0101010101010101";
//const char *appKey  = "2B7E151628AED2A6ABF7158809CF4F3C";
//const char *devEui  = "0101010101010101";

//LoRaWANLib loraWAN(appEui, appKey, devEui);

bool SerialDebug = true;

//SPIFlash intMemory(csPin);

unsigned long previousMillis = 0; 
const long interval = 1000;

float weight = 0.00;

const uint16_t SecondsBetweenActivations = 30;

uint8_t ROUTINE_STEP = 0;
uint16_t ROUTINE_ITERATION_NUMBER = 0;

timerManager gnssUpdateTimer;
timerManager blueLedTimer;
timerManager routineTimer;

byte* uidBytes;

//uint32_t UID[3] = {0, 0, 0};

//MPU6050 accMeter;

void setNextAlarmIn(const uint32_t &seconds) {
  RTC.setAlarmEpoch(RTC.getEpoch() + seconds);
}

void setup() {
  Serial.begin(115200);

  setDebugMode(DEBUG);
  setDebugLevel(LOG_LEV);

  uidBytes = UID::get_UID();

  mosfetSwitch.begin();
  mosfetSwitch.turn_on();

  gnssHandler.toggleGNSS(true);

  intMemory.init();
  loadCell.begin();
  intBlueLED.begin();
  accMeter.begin();
  
  //BMA280Func.begin();
  //bma280handler.begin();
  //attachInterrupt(digitalPinToInterrupt(intPin1), []{ bma280handler.handleInterrupt1(); }, RISING);
  //attachInterrupt(digitalPinToInterrupt(intPin2), []{ bma280handler.handleInterrupt2(); }, RISING);
  //gnssHandler.initializeArduinoGNSSPins();
  gnssHandler.initializeStmGNSSPins();
  gnssHandler.configureGNSS();
  LoRaWANManager.begin();

  Step = 0;

  //set alarm to update the RTC periodically
  //setNextAlarmIn(SecondsBetweenActivations);
  //RTC.enableAlarm(RTC.MATCH_YYMMDDHHMMSS);
  //RTC.attachInterrupt(alarmMatch);

  mosfetSwitch.turn_off();
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

void run()
{
  gnssHandler.updateRTCViaGNSS();
  loadCell.read_weight(100);
  intBlueLED.intLED_on_off();
  getVDDA();
  getTemp();
  deep_sleep(10);
}

void GNSS_routine()
{  
  bool GNSFix = gnssHandler.getIsLocationFixed();
  bool RTCFix = gnssHandler.RTCFix();

  log("getIsLocationFixed: " + String(GNSFix) + "\t| RTCFix:" + String(RTCFix), 1);
  
  if(!GNSFix || !RTCFix) //aggiungere lo step
  {
    gnssHandler.toggleGNSS(true);
    bool GNSSModuleState = gnssHandler.getGNSSModuleState();
    log("GNSSModuleState: " + String(GNSSModuleState),1);
    if(GNSSModuleState) {
        log("RUN: GNSS update.", 2);
        gnssHandler.update();  // Update GNSS data
        //gnssHandler.readGpsTime();
    }else log("RUN: GNSS OFF.", 1);
  }else if(GNSFix || RTCFix){
    log("GNS COMPLETED!", 1);
    gnssHandler.setGNSSStatus(false);
    gnssHandler.toggleGNSS(false);
    ROUTINE_STEP++;
  }  
}

void LoadCell_routine()
{
  weight = loadCell.read_weight(100);
  log("loadCell step: " + String(weight), 1);
  ROUTINE_STEP++;
}

void routine() {
  if (!enableRoutine) {
        return;  // Se la routine non è abilitata, esci dalla funzione
    }

  if(routineTimer.delay(1000)) {
    log("ROUTINE:\n\tSTEP:" + String(ROUTINE_STEP) + "\n\tITERATION_NUMBER: " + String(ROUTINE_ITERATION_NUMBER), 1);

    if(ROUTINE_STEP == 0) {
      GNSS_routine();
    }
  
    if(ROUTINE_STEP == 1) {
      LoadCell_routine();
    }

    if(ROUTINE_STEP == 2) {
      log("Routine compleated\n\tDeep sleeping for 1 min...", 1);
      ROUTINE_STEP = 0;
      ROUTINE_ITERATION_NUMBER++;
      deep_sleep(60);
    }
  }
}

void loop() {
  
  LOG_END_LOOP = false;

  if (blueLedTimer.delay(1000)) {
    intBlueLED.intLED_on_off();
    log(String(Step),2);
  }

  routine();
  
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
  
  /*
  if (gnssHandler.isTracking()) {
    gnssHandler.readPositioningData();
  }
  */

  if (Serial.available()) {
    char c = Serial.read();

    handleSerialCommand(c);
  }

  if(LOG_END_LOOP) {
    log(LOG_END_LINE_STRING, 1);
  }
}