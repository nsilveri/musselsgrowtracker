
HX711 scale;

bool MOSFET_STATE = true;
long scaleFactor = 43.34;     //default value for YZC-516C
float defaultOffset = 49133.0; //default value for YZC-516C

void loadCell::begin() {
  scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
  scale.set_offset(defaultOffset);
  scale.get_scale(scaleFactor);
  //pinMode(MOSFET_PIN, OUTPUT);
  //digitalWrite(MOSFET_PIN, MOSFET_STATE);
}

void loadCell::read() {

  if (scale.is_ready()) {
    long reading = scale.read();
    Serial.print("HX711 reading: ");
    Serial.println(reading);
  } else {
    Serial.println("HX711 not found.");
  }

  delay(1000);
  
}