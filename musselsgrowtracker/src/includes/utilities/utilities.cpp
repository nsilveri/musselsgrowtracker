// utilities.cpp

#include "utilities.h"
#include <STM32L0.h>
#include <Wire.h>
#include "..\Ethereum-RPL\ETrans.h"
#include <string>
#include "..\ECCProcessor\ECCProcessor.h"
#include "..\GNSSFunctions\GNSSFunctions.h"
#include "..\loadCell\loadCell.h"
//#include "..\ECCProcessor\ECCtest.h"

bool DEBUG_MODE;
uint8_t LOG_LEVEL;
bool LOG_END_LOOP = false;
String LOG_END_LINE_STRING = "END_LINE";
String LOG_WAIT_STRING = "wait";

#define intLedPin 10
bool LED_STATE = false;

byte bytePackaging::packData() {
    byte compressedBatteryLevel;
    if (pwrMan.get_BatPercent() <= 10) {
        compressedBatteryLevel = 0; // 00
    } else if (pwrMan.get_BatPercent() <= 25) {
        compressedBatteryLevel = 1; // 01
    } else if (pwrMan.get_BatPercent() <= 50) {
        compressedBatteryLevel = 2; // 10
    } else {
        compressedBatteryLevel = 3; // 11
    }

    // Assicurati che gpsMovementDetected sia solo 0 o 1
    byte gpsMovementBit = gnssHandler.getDisplacementStatus() ? 1 : 0;

    // Il bit di spostamento GPS viene inserito nel bit 6
    // e i livelli della batteria compressi nei bit 7 e 8.
    return (compressedBatteryLevel << 6) | (gpsMovementBit << 5);
}



byte* UID::get_UID() {
    static uint32_t UID[3] = {0, 0, 0};
    STM32L0.getUID(UID);  // Assumendo che questa funzione popoli l'array UID

    static byte uidArray[UID_LENGTH];

    // Converti ogni uint32_t in 4 byte
    for (int i = 0; i < 3; i++) {
        uidArray[4 * i] = (byte)((UID[i] >> 24) & 0xFF);
        uidArray[4 * i + 1] = (byte)((UID[i] >> 16) & 0xFF);
        uidArray[4 * i + 2] = (byte)((UID[i] >> 8) & 0xFF);
        uidArray[4 * i + 3] = (byte)(UID[i] & 0xFF);
    }

    return uidArray;
}

String UID::get_UID_String() {
    byte* uid = get_UID();   // Ottenere l'UID come array di byte
    String uidHex = "";      // Creare una stringa vuota per l'HEX
    String uidAscii = "";    // Creare una stringa vuota per l'ASCII

    for (int i = 0; i < UID_LENGTH; i++) {
        // Aggiungi la rappresentazione esadecimale del byte
        if (uid[i] < 16) uidHex += "0";  // Aggiungi uno zero per valori esadecimali a una cifra
        uidHex += String(uid[i], HEX);   // Convertire ogni byte in esadecimale e aggiungerlo alla stringa HEX

        // Aggiungi la rappresentazione ASCII del byte, se stampabile
        uidAscii += isPrintable(uid[i]) ? (char)uid[i] : '.';
    }

    uidHex.toUpperCase();  // Convertire la stringa HEX in maiuscolo

    // Combinare le due rappresentazioni in una stringa
    String combinedString = "UID: \n\tHEX: " + uidHex + "\n\tASCII: " + uidAscii;
    return combinedString; // Restituire la stringa combinata
}



void timerManager::startTimer() {
  startTime = millis();
  setEnable();
}

void timerManager::stopTimer() {
  savedElapsedTime = elapsedTimer();
  setDisable();
}

void timerManager::updateCurrTimer() {
  currentTime = millis();
}

unsigned long timerManager::elapsedTimer(unsigned int delay_ms) {
  updateCurrTimer();
  elapsedTime = (currentTime - startTime);
  log("currentTime: " + String(currentTime) + " | startTime: " + String(startTime), 2);
  log("elapsedTime: " + String(elapsedTime) + " | delay_ms: " + String(delay_ms), 2);
  return elapsedTime;
}

void timerManager::setEnable() {
  activeTimer = true;
}

void timerManager::setDisable() {
  activeTimer = false;
}

bool timerManager::getTimerStatus() {
  return activeTimer;
}

void timerManager::saveElapsedTimer() {
  savedElapsedTime = elapsedTimer();
}

unsigned long timerManager::getSavedElapsedTime() {
  return savedElapsedTime;
}

bool timerManager::delay(unsigned int delay_ms) {
  elapsedTime = elapsedTimer(delay_ms);  // Calcola il tempo trascorso finora
  if (elapsedTime >= delay_ms) {
    //log("DELAY: true.", 1);
    startTimer();
    return true;
  }else return false;
}

void setDebugMode(bool value) {
  DEBUG_MODE = value;
}

void setDebugLevel(uint8_t value) {
  LOG_LEVEL = value;
}

DebugState getDebugState() {
  return {DEBUG_MODE, LOG_LEVEL};
}

void log(String log, uint8_t LEVEL_MSG) 
{
  if(DEBUG_MODE){
      if(LEVEL_MSG == 1 && LOG_LEVEL >= 1 && log != LOG_WAIT_STRING && log != LOG_END_LINE_STRING){
          LOG_END_LOOP = true;
          Serial.println("LOG1: " + log);
      }if(LEVEL_MSG == 2 && LOG_LEVEL >= 2 && log != LOG_WAIT_STRING && log != LOG_END_LINE_STRING){
          LOG_END_LOOP = true;
          Serial.println("LOG2: " + log);
      }if(log == LOG_WAIT_STRING){
          Serial.print(" . ");
      }if(log == LOG_END_LINE_STRING){
          Serial.println("\t============|END LOOP|============\n");
      }
  }
}

void deep_sleep(uint32_t seconds)
{
    log("deep_sleeping...", 1);
    unsigned long startTime;
    startTime = millis();

    STM32L0.stop(seconds * 1000);

    unsigned long stopTime = millis();
    unsigned long elapsedTime = stopTime - startTime;
    log("deep_sleep for " + String(elapsedTime/1000) + " seconds.", 1);
}
/*
void STM32L0_Voltage::(void(*callback)(void))
{
    _locationCallback = Callback(callback);
}
*/

float getTemp()
{
  float Temperature = STM32L0.getTemperature();
  log("Temp= " + String(Temperature), 1);
  return Temperature;
}

void intLED::begin()
{
  pinMode(intLedPin, OUTPUT);
  digitalWrite(intLedPin, LED_STATE);
}

void intLED::intLED_on()
{
  if(LED_STATE != true)
  {
    LED_STATE = true;
    digitalWrite(intLedPin, LED_STATE);
    log("intLED ON", 2);
  }
}

void intLED::intLED_on_off()
{
  LED_STATE = !LED_STATE;
  digitalWrite(intLedPin, LED_STATE);
  log("intLED " + String(LED_STATE), 2);
}

void intLED::intLED_off()
{
  if(LED_STATE != false)
  {
    LED_STATE = false;
    digitalWrite(intLedPin, LED_STATE);
    log("intLED OFF", 2);
  }
}

void intLED::visualLog()
{
  LED_STATE = false;
  for(uint8_t i = 0; i < 3; i++){
    intBlueLED.intLED_on_off();
    delay(100);
  }
  log("LED log", 2);
}

void WireScan::begin()
{
  pinMode(14, OUTPUT); // hold SDA low to wake up ECC608A
  digitalWrite(14, LOW);
  delay(1000);

  Wire.begin(); // set master mode 
  Wire.setClock(400000); // I2C frequency at 400 kHz  
  delay(1000);
}

void WireScan::scan()
{

  i2cScan();
}

void WireScan::i2cScan() {
  // scan for i2c devices
  byte error, address;
  int nDevices;

  Serial.println("Scanning...");

  // Crea una mappa di indirizzi e nomi associati
  struct I2CDevice {
    byte address;
    const char* name;
  };

  I2CDevice devices[] = {
    {0x14, "BMA400 (accelerometer)"},
    {0x48, "ADS1115 (ADC for read external batt voltage)"},
    {0x60, "Internal device"},
    {0x68, "GY521 (accelerometer)"},
    {0x77, "Internal device"}
    // Aggiungi altri dispositivi qui con i loro indirizzi e nomi
  };

  nDevices = 0;
  for (int i = 0; i < sizeof(devices) / sizeof(devices[0]); i++) {
    address = devices[i].address;
    Wire.beginTransmission(address);
    error = Wire.endTransmission();

    if (error == 0) {
      Serial.print("I2C device found at address 0x");
      if (address < 16) 
        Serial.print("0");
      Serial.print(address, HEX);
      Serial.print(" (");
      Serial.print(devices[i].name);
      Serial.println(") !");
      
      nDevices++;
    } else if (error == 4) {
      Serial.print("Unknown error at address 0x");
      if (address < 16) 
        Serial.print("0");
      Serial.println(address, HEX);
    }    
  }
  if (nDevices == 0)
    Serial.println("No I2C devices found\n");
  else
    Serial.println("done\n");
}

void mosSwitch::begin()
{
  pinMode(MOSFET_PIN,OUTPUT);
};

bool mosSwitch::mosfet_state_change(bool STATE)
{
    digitalWrite(MOSFET_PIN, STATE);
    return MOSFET_STATE;
}

bool mosSwitch::getMosfetState()
{
  return MOSFET_STATE;
}

bool mosSwitch::turn_on_off()
{   
    MOSFET_STATE = !MOSFET_STATE;
    mosfet_state_change(MOSFET_STATE);
    return MOSFET_STATE;
}

bool mosSwitch::turn_off()
{
    if(MOSFET_STATE){
      MOSFET_STATE = false;
      mosfet_state_change(MOSFET_STATE);
    }
}

bool mosSwitch::turn_on()
{   
    if(!MOSFET_STATE){
      MOSFET_STATE = true;
      mosfet_state_change(MOSFET_STATE);
    }
}

void ethTransaction::CreateRawTransaction(byte *byteArray, size_t arraySize) {
    const char *charArray = reinterpret_cast<const char*>(byteArray);
    EthTr.assembleTx(charArray);
}      

byte* LoRaPayload::createDataMsg(const byte* SerialID, uint32_t TimestampLinux, byte* GpsData, uint16_t LoadCellMeasurement, byte SysStatus) {
    const uint8_t SERIALID_SIZE =   6; // UID  data size
    const uint8_t TIMESTAMP_SIZE =  4; // TS   data size
    const uint8_t GNSS_LOCATION =   8; // GNSS data size
    const uint8_t LOADCELL_SIZE =   2; // LC   data size
    const uint8_t SYSSTATUS_SIZE =  1; // SYS  data size

    size_t bufferSize = SERIALID_SIZE + TIMESTAMP_SIZE + GNSS_LOCATION + LOADCELL_SIZE + SYSSTATUS_SIZE;
    byte* buffer = new byte[bufferSize];
    int pos = 0;

    memcpy(buffer + pos, SerialID, SERIALID_SIZE);
    // Log SerialID
    logBytes(buffer + pos, SERIALID_SIZE, "SerialID: ");
    pos += SERIALID_SIZE;
    
    memcpy(buffer + pos, &TimestampLinux, TIMESTAMP_SIZE);
    // Log TimestampLinux
    logBytes(buffer + pos, TIMESTAMP_SIZE, "TimeStamp: ");
    pos += TIMESTAMP_SIZE;

    // Aggiungi dati GPS al buffer
    memcpy(buffer + pos, GpsData, GNSS_LOCATION);
    // Log GPS Data
    logBytes(buffer + pos, GNSS_LOCATION, "GPS Data: ");
    pos += GNSS_LOCATION;
    
    memcpy(buffer + pos, &LoadCellMeasurement, LOADCELL_SIZE);
    // Log LoadCellMeasurement
    logBytes(buffer + pos, LOADCELL_SIZE, "LoadCell: ");
    pos += LOADCELL_SIZE;

    buffer[pos] = SysStatus;
    // Log SysStatus
    logBytes(buffer + pos, SYSSTATUS_SIZE, "SysStatus: ");

    set_msgByte(buffer, bufferSize);

    dataMsg = buffer;
    dataMsgSize = bufferSize;

    return buffer;
}

void LoRaPayload::clearData() {
    msgToSendStr = "";
    msgToSendHex = "";

    if (dataMsg != nullptr) {
        delete[] dataMsg;
        dataMsg = nullptr;
        dataMsgSize = 0;
    }
}

void LoRaPayload::set_msgByte(byte* msg, size_t msgSize) {
    if (msg == nullptr || msgSize == 0) {
        clearData();
        return;
    }

    clearData();

    dataMsg = new byte[msgSize];
    memcpy(dataMsg, msg, msgSize);
    dataMsgSize = msgSize;

    msgToSendStr = bytesToString(dataMsg, dataMsgSize);
    msgToSendHex = bytesToHexString(dataMsg, dataMsgSize);
}

String LoRaPayload::get_msgStr() {
    return msgToSendStr;
}

String LoRaPayload::get_msgHex() {
    return msgToSendHex;
}

String LoRaPayload::set_msgStr(String msg) {
    clearData();
    msgToSendStr = msg;
}

String LoRaPayload::set_msgHex(String msg) {
    clearData();
    msgToSendHex = msg;
}

void LoRaPayload::logBytes(const byte* data, size_t size, const char* message) {
    String output = message;
    output += " [ASCII]: ";
    for (size_t i = 0; i < size; ++i) {
        // Aggiunge il carattere ASCII o un punto se non è stampabile
        output += isPrintable(data[i]) ? (char)data[i] : '.';
    }
    output += " [HEX]: ";
    for (size_t i = 0; i < size; ++i) {
        // Aggiunge la rappresentazione esadecimale
        char buf[3];
        sprintf(buf, "%02X", data[i]);
        output += buf;
        if (i < size - 1) output += " ";
    }
    log(output, 1);
}


size_t LoRaPayload::get_MsgSize(){
  return dataMsgSize;
}

String LoRaPayload::bytesToHexString(const byte* buffer, size_t bufferSize) {
    String hexString = "";
    for (unsigned int i = 0; i < bufferSize; i++) {
        if (buffer[i] < 16) {
            hexString += "0";  // Aggiunge uno zero davanti se il numero è minore di 16 (es. 0A invece di A)
        }
        hexString += String(buffer[i], HEX); // Converte il byte in esadecimale e lo aggiunge alla stringa
    }
    hexString.toUpperCase(); // Opzionale: converte la stringa in caratteri maiuscoli
    return hexString;
}



byte* LoRaPayload::get_DataMsg() const {
    return dataMsg;
}

String LoRaPayload::bytesToString(const byte* buffer, size_t bufferSize) {
    String result = "";
    
    for (size_t i = 0; i < bufferSize; ++i) {
        result += (char)buffer[i];  // Converti il byte in un carattere
        result += ' ';              // Aggiungi uno spazio dopo il carattere
    }

    return result; // Rimuove lo spazio finale
}

void GNSSEeprom::saveGNSSCoordinates(double lat, double lon) {
    EEPROMWrite(latAddress, lat); // Salva la latitudine all'indirizzo 0
    EEPROMWrite(lonAddress, lon); // Salva la longitudine all'indirizzo successivo
}

double GNSSEeprom::readLatitude() {
    return EEPROMRead(latAddress); // Legge la latitudine dall'indirizzo 0
}

double GNSSEeprom::readLongitude() {
    return EEPROMRead(lonAddress); // Legge la longitudine dall'indirizzo successivo
}

void GNSSEeprom::EEPROMWrite(int address, double value) {
    //byte* p = (byte*)(void*)&value;
    //for (int i = 0; i < sizeof(value); i++) {
    EEPROM.updateDouble(address, value);
    //}
}

double GNSSEeprom::EEPROMRead(int address) {
    double value = EEPROM.readDouble(address++); //0.0;
    //byte* p = (byte*)(void*)&value;
    //for (int i = 0; i < sizeof(value); i++) {
    //    //*p++ = EEPROM.read(address++);
    //    *p++ = EEPROM.readDouble(address++);
    //}
    return value;
}

void MSGSend::sendMsg() {
    if(SEND_MODE == ECC_MODE) {
        log("Sending data with ECC\n", 1);
        sendMsgECC();
    }else if(SEND_MODE == ETH_MODE) {
        log("Sending data with ETH\n", 1);
        sendMsgETH();
    }
}

void MSGSend::set_EccSign() {
    SEND_MODE = ECC_MODE;
}

void MSGSend::set_EthSign() {
    SEND_MODE = ETH_MODE;
}

void MSGSend::sendMsgECC() 
{
    log("ECC Encryption selected", 1);
    delay(2000);                                                              //generateLoRaMsg()
    MSGPayload.createDataMsg(getIDDevice.get_UID(), RTC.getEpoch(), gnssHandler.generateLoRaMsg(), loadCell.get_LastWeightReading(), bytePack.packData());
    //msgToSendHex = MSGPayload.get_msgHex();
    log("\ndataMsg HEX: " + MSGPayload.get_msgHex(), 1);

    String hashMsg = MyECC::generateSHA256Hash(MSGPayload.get_msgHex());
    log("dataMsg Hash (HEX): " + String(hashMsg), 1);
    log("dataMsg Size (HEX): " + String(hashMsg), 1);

    LoRaWANManager.sendMessage(MSGPayload.get_DataMsg(), MSGPayload.get_MsgSize());
}
    
void MSGSend::sendMsgETH() 
{
    log("Ethereum-RPL encryption selected", 1);
    delay(2000);
    MSGPayload.createDataMsg(getIDDevice.get_UID(), RTC.getEpoch(), gnssHandler.generateLoRaMsg(), loadCell.get_LastWeightReading(), bytePack.packData());
    ethTx.CreateRawTransaction(MSGPayload.get_DataMsg(), MSGPayload.get_MsgSize());
    //MSGPayload.clearData();
}

UID getIDDevice;
LoRaPayload MSGPayload;
ethTransaction ethTx;
mosSwitch mosfetSwitch;
intLED intBlueLED;
WireScan i2cScan;
GNSSEeprom gnssEeprom;
bytePackaging bytePack;
MSGSend msgService;