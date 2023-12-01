#include "HX711.h"

// HX711 circuit wiring
const int LOADCELL_DOUT_PIN = 15;
const int LOADCELL_SCK_PIN = 14;
#define MOSFET_PIN 9

HX711 scale;

bool MOSFET_STATE = true;

void setup() {
  Serial.begin(57600);
  scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
  pinMode(MOSFET_PIN, OUTPUT);
  digitalWrite(MOSFET_PIN, MOSFET_STATE);
}

void loop() {

  if (scale.is_ready()) {
    long reading = scale.read();
    Serial.print("HX711 reading: ");
    Serial.println(reading);
  } else {
    Serial.println("HX711 not found.");
  }

  delay(1000);
  
}
