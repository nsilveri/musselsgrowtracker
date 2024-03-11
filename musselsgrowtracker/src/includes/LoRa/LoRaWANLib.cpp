#include "Arduino.h"
#include "LoRaWANLib.h"
#include "..\utilities\utilities.h"
#include "..\GNSSFunctions\GNSSFunctions.h"
#include "..\ECCProcessor\ECCProcessor.h"


bool autoRejoin = false;

LoRaWANLib::LoRaWANLib(const char *appEui, const char *appKey, const char *devEui)
    : _appEui(appEui), _appKey(appKey), _devEui(devEui) {}

void LoRaWANLib::begin() {

    const uint8_t joinMode	= 1;
    // LoRaWAN join keys:
    /* OTAA 1.0
        - "appEUI" : "random" appEUI key
        - "appKey" : application OTAA auth key
        - "devEui" : device hardware serial
    */
    char devEuiStr[17];

    const char *appEui = "70B3D57ED00093FB";
    const char *appKey = "91923C75B75353C9A669A15B4E96C974";
    const char *devEui = "3739323265378b09";

    /* ABP
        - "devAddr" : short device address assigned on activation
        - "nwkSKey" : network session key generated from activation
        - "appSKey" : application session key generated from activation
    */
    const char *devAddr		= "012a5f63";
    const char *nwkSKey		= "72415e7030c9a642892537c57f2ff2f3";
    const char *appSKey		= "862d2042f92caff54073712819df83aa";
    // whether to use or not ADR for LoRa
    const bool useADR		= false;
    // LoRa TX power
    const float txPower		= 10;
    /*	LoRaWAN operative band

        - Asia       AS923
        - Australia  AU915
        - Europe     EU868
        - India      IN865
        - Korea      KR920
        - US         US915 (64 + 8 channels)
    */

    const LoRaWANBand *BAND = &EU868;
    // LoRaWAN datarate
    const unsigned int dataRate = 1;
    /* LoRaWAN sub-band
        
        - 0 : library default
        - 1 : MTCAP
        - 2 : TT gateways
    */
    const unsigned int subBand = 0;
    /* LoRaWAN periodic re-send strategy

        - "lrwTxPeriodSeconds" (s): time lapse to wait between one re-send and the other
    */

    // Configure LoRaWAN connection
    LoRaWAN.begin(*BAND);
    LoRaWAN.setADR(useADR);
    LoRaWAN.setDataRate(dataRate);
    LoRaWAN.setTxPower(txPower);
	LoRaWAN.setRetries(0);
	if( subBand > 0 )
		LoRaWAN.setSubBand(subBand); // 1 for MTCAP, 2 for TT gateways
    if( joinMode == 1 )
	{
		LoRaWAN.setAppKey(appKey);
		LoRaWAN.setDevEui(devEui);
        uint8_t joinLOG = LoRaWAN.joinOTAA(appEui, appKey);

        log("joinLOG: " + String(joinLOG), 1);

		if( joinLOG == 0 ){
            delay(5000);
			log("Err: OTAA join failed!", 1);
            log("LoRa states: -> Join:" + String(LoRaWAN.joined()) + " - Busy:" + String(LoRaWAN.busy()), 1);
        }else{
            log("LoRaWAN joined through OTAA", 1);
            log("LoRa states: -> Join:" + String(LoRaWAN.joined()) + " - Busy:" + String(LoRaWAN.busy()), 1);
        }
    }else
	{
		LoRaWAN.setDevAddr(devAddr);
		LoRaWAN.setNwkSKey(nwkSKey);
		LoRaWAN.setAppSKey(appSKey);
		if( LoRaWAN.joinABP() == 0 )
			Serial.println("Err: ABP join failed");
		else
			Serial.println("LoRaWAN activated through ABP");
	}
}

void LoRaWANLib::getInfo()
{
    char devEuiStr[17];
    int success = LoRaWAN.getDevEui(devEuiStr, sizeof(devEuiStr));

    if (success) {
        log("STM32L0 Device EUI = " + String(devEuiStr), 1);
    } else {
        log("Error getting Device EUI", 1);
    }
}


void LoRaWANLib::sendMessage(uint8_t* payload, size_t payloadSize) {
    // Log dell'invio del messaggio con i dati firmati.
    MSGPayload.logBytes(payload, payloadSize, "sendMSG) \n\tMSG Data signed :");

    // Tenta di unirsi alla rete LoRa se non già unito.
    while (!LoRaWAN.joined()) {
        log("Attempting to join LoRa network...", 1);
        LoRaWAN.rejoinOTAA();
        delay(5000); // Attesa prima di riprovare, per dare tempo al dispositivo di unirsi.
    }

    // Una volta unito, invia il messaggio.
    bool sent = false;
    bool writeAvailable = false;

    bool beginPacket_state  = false;
    bool write_state        = false;
    bool endPacket_state    = false;

    log("SENDING MESSAGE: ", 1);
    while (!sent) {
        if (!LoRaWAN.busy()) 
        {
            String payloadStr = MSGPayload.bytesToHexString(payload, payloadSize);

            log("PAYLOAD: '" + payloadStr + "'", 1);
            log("PAYLOAD_SIZE: '" + String(payloadSize) + "'", 1);
            uint8_t attempt = 0;
            while((!beginPacket_state || !write_state)) 
            {
                beginPacket_state  = LoRaWAN.beginPacket();
                write_state        = LoRaWAN.sendPacket(payload, payloadSize);
                LoRaWAN.endPacket();
                attempt++;
                if(attempt > 5) {
                    beginPacket_state  = true;
                    write_state        = true;
                }   
            }
            log("SEND STATUS: beginPacket_state: " + String(beginPacket_state) + ", write_state: " + String(write_state) + ", endPacket_state: " + String(endPacket_state), 1);
            delay(500);
            sent = true; // Il messaggio è stato inviato.
        } else {
            log("Network busy, waiting...", 1);
            delay(500); // Attesa prima di riprovare, se la rete è occupata.
        }
    }
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
    setDataMsg(buffer, bufferSize);
    dataMsg = buffer;

    return buffer;
}

void LoRaPayload::setDataMsg(const uint8_t* msg, size_t size) {
    clearDataMsg(); // Pulisce i dati esistenti prima di impostarne di nuovi
    dataMsg = new uint8_t[size];
    memcpy(dataMsg, msg, size);
    dataMsgSize = size;
}

void LoRaPayload::setDataSignedMsg(const uint8_t* msg, size_t size) {
    clearDataSignedMsg(); // Pulisce i dati esistenti prima di impostarne di nuovi
    dataSignedMsg = new uint8_t[size];
    memcpy(dataSignedMsg, msg, size);
    dataSignedMsgSize = size;
}

void LoRaPayload::setCombinedMsg(const uint8_t* msg, size_t size) {
    clearCombinedMsg(); // Pulisce i dati esistenti prima di impostarne di nuovi
    combinedMsg = new uint8_t[size];
    memcpy(combinedMsg, msg, size);
    combinedMsgSize = size;
}

void LoRaPayload::clearDataMsg() {
    if (dataMsg != nullptr) {
        delete[] dataMsg;
        dataMsg = nullptr;
    }
    dataMsgSize = 0;
}

void LoRaPayload::clearDataSignedMsg() {
    if (dataSignedMsg != nullptr) {
        delete[] dataSignedMsg;
        dataSignedMsg = nullptr;
    }
    dataSignedMsgSize = 0;
}

void LoRaPayload::clearCombinedMsg() {
    if (combinedMsg != nullptr) {
        delete[] combinedMsg;
        combinedMsg = nullptr;
    }
    combinedMsgSize = 0;
}

// Funzione per convertire un byte in una stringa esadecimale
String LoRaPayload::byteToHexString(uint8_t byte) {
  String hexString = String(byte, HEX);
  if (hexString.length() < 2) {
    hexString = "0" + hexString; // Assicura che la stringa sia di due caratteri
  }
  return hexString;
}

// Funzione per convertire un array di uint8_t in una stringa esadecimale
String LoRaPayload::arrayToHexString(uint8_t* array, size_t arraySize) {
  String hexString = "";
  for (size_t i = 0; i < arraySize; i++) {
    hexString += byteToHexString(array[i]);
  }
  return hexString;
}

// Funzione per convertire una stringa esadecimale in un array di uint8_t
void LoRaPayload::hexStringToArray(String hexString, uint8_t* array, size_t arraySize) {
  for (size_t i = 0; i < arraySize; i++) {
    uint32_t byte = strtoul(hexString.substring(i * 2, i * 2 + 2).c_str(), nullptr, 16);
    array[i] = static_cast<uint8_t>(byte);
  }
}

uint8_t* LoRaPayload::sendMergedMsg(bool TEST)
{
    if(!TEST) {
        uint8_t SIGNED_SIZE = 64;
        log("\nMerging message ======================", 1);
        createDataMsg(getIDDevice.get_UID(), RTC.getEpoch(), gnssHandler.generateLoRaMsg(), 
                        loadCell.get_LastWeightReading(), bytePack.packData());

        dataMsg = get_DataMsg();
        String msgToSendHashHex = get_msgHashString();

        eccProcessor.sign_message(get_msgHashUint8());

        uint8_t* dataSignedMsg = eccProcessor.get_msgSigned();

        String dataSignedMsgHex = arrayToHexString(dataSignedMsg, SIGNED_SIZE);
        size_t dataSignedMsgHexSize = dataSignedMsgHex.length() / 2;

        String combinedHexString = msgToSendHex + dataSignedMsgHex;
        size_t combinedMsgSize = combinedHexString.length() / 2;

        combinedMsg = new uint8_t[combinedMsgSize];

        hexStringToArray(combinedHexString, combinedMsg, combinedMsgSize);

        log("msgToSend:  \n\t[HEX]: " + msgToSendHex + "\n\t[HASH]: " + msgToSendHashHex, 1);
        log("\nmsgSignedToSend:  \n\t[HEX]: " + dataSignedMsgHex, 1);
        log("\ncombinedHexString:  \n\t[HEX]: " + combinedHexString, 1);

        log("\nMerged message (" + String(combinedMsgSize) + " bytes): " + combinedHexString, 1);

        LoRaWANManager.sendMessage(combinedMsg, combinedMsgSize);

    }else {
        const uint8_t payloadSizeTest = 85;
        //uint8_t payloadTest[payloadSizeTest];

        // Riempie l'array con dei byte di esempio
        //uint8_t payloadTest[] = {0xDE, 0xAD, 0xBE, 0xEF, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06};
        uint8_t payloadTest[] = {
            0x09, 0x47, 0x37, 0x30, 0x32, 0x32, 0xb2, 0x6f, 0x6d, 0x38, 0x00, 0x00, 
            0x80, 0xbf, 0xcc, 0x10, 0x00, 0x20, 0xff, 0xff, 0xc0, 0x46, 0x0b, 0x6d, 
            0xe4, 0x42, 0x3c, 0x73, 0xc3, 0x3b, 0x1c, 0x2a, 0xa3, 0x4f, 0x33, 0xf7, 
            0xc0, 0x9b, 0x80, 0xdd, 0x4b, 0xaa, 0xc6, 0x21, 0x5d, 0xcf, 0x43, 0x1f, 
            0x14, 0x0a, 0x52, 0xb0, 0xf7, 0x7e, 0x4f, 0xe5, 0xc1, 0x96, 0x21, 0x49, 
            0x4b, 0xe8, 0x02, 0xa6, 0x92, 0x42, 0xe6, 0x99, 0x5b, 0xd2, 0x6a, 0xcc, 
            0x94, 0x4d, 0xff, 0xc4, 0xff, 0xeb, 0xe9, 0x78, 0xab, 0x96, 0x7c, 0x7e, 0xa8
        };

        LoRaWANManager.sendMessage(payloadTest, payloadSizeTest);
    }
    return combinedMsg;
}

void LoRaPayload::clearData() {
    msgToSendStr = "";
    msgToSendHex = "";

    if (dataMsg != nullptr) {
        delete[] dataMsg;
        dataMsg = nullptr;
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

    msgToSendStr  = bytesToString(dataMsg, msgSize);
    msgToSendHex  = bytesToHexString(dataMsg, msgSize);
    hexToSHA256(msgToSendHex, msgToSendHash);
    msgToSendHashStr = bytesToHexString(msgToSendHash, sizeof(msgToSendHash));
}

void LoRaPayload::hexToSHA256(String data, uint8_t* hash_out) {
    if (!hash_out) return;

    uint8_t data_buffer[data.length()];
    for (size_t i = 0; i < data.length(); i++) {
        data_buffer[i] = (uint8_t)data.charAt(i);
    }

    SHA256_CTX ctx;
    sha256_init(&ctx);
    sha256_update(&ctx, data_buffer, data.length());
    sha256_final(&ctx, hash_out);
}

uint8_t* LoRaPayload::get_msgHashUint8() {
    return msgToSendHash;
}

String LoRaPayload::get_msgHashString() {
    return msgToSendHashStr;
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
  bool _ascii = false;
  bool _hex   = true;
  bool spaceBetweenHex = false;
  String output = message;

  if(_ascii) {
    output += " [ASCII]: ";
    for (size_t i = 0; i < size; ++i) {
        output += isPrintable(data[i]) ? (char)data[i] : '.';
    }
  }

  if(_hex) {
    output += " [HEX]: ";
    for (size_t i = 0; i < size; ++i) {
        char buf[3];
        sprintf(buf, "%02x", data[i]);
        output += buf;
        if (i < size - 1 && spaceBetweenHex) output += " ";
    }
  }
  log(output, 1);
}

size_t LoRaPayload::get_MsgSignedSize() const{
  return dataSignedMsgSize;
}

size_t LoRaPayload::get_MsgSize(){
  return dataMsgSize;
}

String LoRaPayload::bytesToHexString(const byte* buffer, size_t bufferSize) {
    String hexString = "";
    for (unsigned int i = 0; i < bufferSize; i++) {
        if (buffer[i] < 16) {
            hexString += "0";
        }
        hexString += String(buffer[i], HEX);
    }
    return hexString;
}




byte* LoRaPayload::get_DataMsg() const {
    return dataMsg;
}

byte*  LoRaPayload::get_DataSignedMsg() const {
    return dataSignedMsg;
}

byte*  LoRaPayload::get_CombinedMsg() const {
    return combinedMsg;
}

String LoRaPayload::bytesToString(const byte* buffer, size_t bufferSize) {
    String result = "";
    
    for (size_t i = 0; i < bufferSize; ++i) {
        result += (char)buffer[i]; 
        result += ' ';             
    }

    return result; // Rimuove lo spazio finale
}

void MSGSend::sendMsg() {
        sendMsgECC();
}

void MSGSend::sendMsgECC() 
{
    log("ECC Encryption selected", 1);                                                            
    MSGPayload.createDataMsg(getIDDevice.get_UID(), RTC.getEpoch(), gnssHandler.generateLoRaMsg(), loadCell.get_LastWeightReading(), bytePack.packData());

    log("\ndataMsg HEX: " + MSGPayload.get_msgHex(), 1);

    String hashMsg;
    uint8_t hash_to_sign[32];
    eccProcessor.generateSHA256Hash(MSGPayload.get_msgHex(), hash_to_sign);
    log("dataMsg Hash (HEX): " + String(hashMsg), 1);
    log("dataMsg Size (HEX): " + String(hashMsg), 1);

    LoRaWANManager.sendMessage(hash_to_sign, sizeof(hash_to_sign));
}


LoRaPayload MSGPayload;
MSGSend msgService;
LoRaWANLib LoRaWANManager("70B3D57ED00093FB", "91923C75B75353C9A669A15B4E96C974", "3739323265378b09");