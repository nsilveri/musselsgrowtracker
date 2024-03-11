#include <STM32L0.h>
#include <Arduino.h>
#include "src\includes\GNSSFunctions\GNSSFunctions.h"
#include "src\includes\handleSerial\handleSerialCommand.h"
#include "src\includes\loadCell\loadCell.h"
#include "src\includes\SPI\SPIFlash.h"
#include "src\includes\LoRa\LoRaWANLib.h"
#include <RTC.h>
#include "src\includes\ECCProcessor\ECCProcessor.h"
#include <Wire.h>

#define SLEEP 0
#define RTC_INT 1
#define FIX_WAIT 2
#define DATA_GATERED 3
#define LORAW_SENDING 4
#define TIMEOUT_EXE 99

#define USER_BUTTON 2

#define MOV_TOLERANCE 5

#define csPin 25

//const char *appEui  = "0101010101010101";
//const char *appKey  = "2B7E151628AED2A6ABF7158809CF4F3C";
//const char *devEui  = "0101010101010101";

//LoRaWANLib loraWAN(appEui, appKey, devEui);

bool SerialDebug = true;
bool deep_state = false;

SPIFlash intMemory(csPin);

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

void setup() {
  Serial.begin(115200);

  pinMode(2, INPUT);
  setDebugMode(DEBUG);
  setDebugLevel(LOG_LEV);

  uidBytes = UID::get_UID();

  mosfetSwitch.begin();
  mosfetSwitch.turn_on();

  gnssHandler.restoreOldLoc();
  gnssHandler.toggleGNSS(true);

  intMemory.init();
  loadCell.begin();
  intBlueLED.begin();
  
  gnssHandler.initializeStmGNSSPins();
  gnssHandler.configureGNSS(GNSS_ANTENNA_INTERNAL); //GNSS_ANTENNA_EXTERNAL

  LoRaWANManager.begin();
}



void GNSS_routine()
{  
  bool GNSFix = gnssHandler.getIsLocationFixed();
  bool RTCFix = gnssHandler.RTCFix();
  log("Eeprom locVal => lat:" + String(gnssEeprom.readLatitude())
     + ", lon: " + String(gnssEeprom.readLongitude()),1);
  log("getIsLocationFixed: " + String(GNSFix)
     + "\t RTCFix:" + String(RTCFix), 1);
  
  if(!GNSFix || !RTCFix)
  {
    gnssHandler.toggleGNSS(true);
    bool GNSSModuleState = gnssHandler.getGNSSModuleState(); 
    log("GNSSModuleState: " + String(GNSSModuleState),2);
    if(GNSSModuleState) {
        log("RUN: GNSS update.", 2); 
        gnssHandler.update();  // Update GNSS data 
    }else log("RUN: GNSS OFF.", 1);
  }else if(GNSFix || RTCFix){
    log("GNS COMPLETED!", 1);
    gnssHandler.setGNSSStatus(false);
    gnssHandler.toggleGNSS(false); 
    gnssHandler.saveOnEeprom();
    ROUTINE_STEP++;
  }  
}

void LoadCell_routine()
{
  weight = loadCell.read_weight(100);
  log("loadCell step: " + String(weight), 1);
  ROUTINE_STEP++;
}


void SendMessage_routine()
{
  MSGPayload.sendMergedMsg();
  ROUTINE_STEP++;
}


void routine() {
  if(deep_state){
    deep_state = false;
  }
  if(digitalRead(USER_BUTTON)) {
    ROUTINE_STEP = 3;
  }
  if (!enableRoutine) {
        return;
    }
  if(routineTimer.delay(1000)) {
    log("ROUTINE:\n\tSTEP:" + String(ROUTINE_STEP) + 
    "\n\tITERATION_NUMBER: " + String(ROUTINE_ITERATION_NUMBER), 1);

    if(ROUTINE_STEP == 0) {
      mosfetSwitch.turn_on();
      GNSS_routine();
    }else if(ROUTINE_STEP == 1) {
      LoadCell_routine();
    }else if(ROUTINE_STEP == 2) {
      SendMessage_routine();
    }else if(ROUTINE_STEP == 3) {
      log("Routine compleate\n\tDeep sleeping for 1 min...", 1);
      ROUTINE_STEP = 0;
      ROUTINE_ITERATION_NUMBER++;
      mosfetSwitch.turn_off();
      intBlueLED.intLED_off();
      gnssHandler.toggleGNSS(false);
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

  if (Serial.available()) {
    char c = Serial.read();

    handleSerialCommand(c);
  }

  if(LOG_END_LOOP) {
    log(LOG_END_LINE_STRING, 1);
  }
}