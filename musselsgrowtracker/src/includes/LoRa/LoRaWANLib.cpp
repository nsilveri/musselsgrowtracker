#include "Arduino.h"
#include "LoRaWANLib.h"
#include "..\utilities\utilities.h"

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
//LoRaWAN.getDevEui(devEuiStr, sizeof(devEuiStr));

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
		// join OTAA 1.0
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
		// join ABP
		LoRaWAN.setDevAddr(devAddr);
		LoRaWAN.setNwkSKey(nwkSKey);
		LoRaWAN.setAppSKey(appSKey);
		if( LoRaWAN.joinABP() == 0 )
			Serial.println("Err: ABP join failed");
		else
			Serial.println("LoRaWAN activated through ABP");
	}

    // Configurazione LoRaWAN basata sulla regione (rimuovi commenti per la tua regione)
    // ...
    /*
    // LoRaWAN.setDutyCycle(false);
    Serial.println(31);
    LoRaWAN.begin(EU868);
    //LoRaWAN.setAntennaGain(2.0);
    //EU868
    Serial.println(30);
    LoRaWAN.joinOTAA(this->_appEui, this->_appKey, this->_devEui);
    //LoRaWAN.joinABP();

    log("JOIN( )", 1);
    */
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




void LoRaWANLib::sendMessage(const byte* payload, size_t payloadSize) {
    log("MSG sendMessage: " + MSGPayload.bytesToHexString(payload, payloadSize), 1);

    bool joinState = LoRaWAN.joined();
    bool busyState = LoRaWAN.busy(); 


    log("Pre-send data LoRa states: -> Join:" + String(joinState) + " - Busy:" + String(busyState), 2);

    if(!joinState && autoRejoin)
    {
        log("Rejoining OTAA...", 1);
        LoRaWAN.rejoinOTAA();
    }

    if (LoRaWAN.joined()) {
        bool sent = false;
        
        log("SENDING MESSAGE: ", 1);

        while (!sent) {
            if (!LoRaWAN.busy()) {

                // Create a hexadecimal string from the payload
                String payloadStr = "";
                //for (size_t i = 0; i < payloadSize; i++) {
                //    char hex[3];
                //    sprintf(hex, "%02X", payload[i]);
                //    payloadStr += hex;
                //}
                
                payloadStr = MSGPayload.bytesToHexString(payload, payloadSize);

                log("PAYLOAD: '" + payloadStr + "'", 1);
                log("PAYLOAD_SIZE: '" + String(payloadSize) + "'", 1);

                LoRaWAN.beginPacket();
                LoRaWAN.write(payload, payloadSize);
                LoRaWAN.endPacket();

                sent = true;  // Exit the loop once the message is sent
            } else {
                // Wait for a short period before retrying the send
                log("wait", 1);
                delay(500);
            }
        }
    } else {
        log("LoRa not joined!", 1);
    }
}

//NNSXS.E366ZCJCDOYKDKH5K5J7VBIH456HBYAD6M4R73Y.QXWBSJLNW63TCE3EOBDCTRB4TEJPTN2X2FHSTZJWG7VNQZT7DLYQ
//LoRaWANLib LoRaWANManager("mussels-grow-tracker", "NNSXS.E366ZCJCDOYKDKH5K5J7VBIH456HBYAD6M4R73Y.QXWBSJLNW63TCE3EOBDCTRB4TEJPTN2X2FHSTZJWG7VNQZT7DLYQ", "c0ee40ffff296197");
LoRaWANLib LoRaWANManager("70B3D57ED00093FB", "91923C75B75353C9A669A15B4E96C974", "3739323265378b09");