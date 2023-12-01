#include "HX711.h"
#include "..\loadCell\hx711_basic.h"
#include "..\utilities\utilities.h"

// HX711 circuit wiring
const int LOADCELL_DOUT_PIN = A2;
const int LOADCELL_SCK_PIN  = A3;

long scaleFactor = 43.34;     //default value for YZC-516C
float defaultOffset = 49133.0; //default value for YZC-516C

HX711 scale;

void HX711_module::begin() {
  scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
  scale.set_offset(defaultOffset);
  scale.set_scale(scaleFactor);
}

void HX711_module::calibrate(float weightKnown) 
{
  scale.power_up();
  log("tare without no weight in 2 seconds", 1);
  delay(2000);
  long b = scale.read_average();

  log("read average: \t\t" + String(b), 1);

  log("You have S to place 2kg weight", 1);
  delay(5000);
  long y = scale.read_average();

  long x = weightKnown;
  double m = (y-b)/x;

  log("|b: " + String(b) + "\n| x: " + String(x) + "\n| y: " + String(y) + "\n|m: " + String(m), 1);
  scale.set_offset(b);
  scale.set_scale(m);
  scale.power_down();
}

void HX711_module::printTare()
{
  log("offset: " + String(scale.get_offset()) + " | scale: " + String(scale.get_scale()) + " | units: " + String(scale.get_units()) + " | value: " + String(scale.get_value()), 1);
}

void HX711_module::setScale(float weightKnown)
{
    long offValue = scale.get_offset();
    if(offValue > 0.0)
    {
      log("Setting scale...",1);
      log("offValue: " + String(offValue), 1);

      long rawValue = scale.get_value();  // Valore grezzo restituito dalla bilancia
      rawValue = rawValue;// + offValue;
      log("rawValue: " + String(rawValue), 1);
      scaleFactor = (rawValue-offValue)/weightKnown;
      log("scaleFactor= " + String(rawValue) + " - " + String(offValue) + " = " + String(scaleFactor) + "\n              -------------------\n" + String(weightKnown), 2);
      scale.set_scale(scaleFactor);
      log("Scale set successfully", 1);
    }else log("You need set offSet first.\nPress 'x' to set it without weight.", 1);
}

void HX711_module::setOffset()
{
  log("Please make sure the load cell is empty and not under load.", 1);
  log("Tare will begin in...", 1);
  
  // Attendere l'input da parte dell'utente o utilizzare un meccanismo di trigger
  // Quando l'utente preme un tasto o si verifica il trigger, eseguire la tara.
  for(uint8_t i = 0; i < 5; i++)
  {
    log(String(i), 1);
    delay(1000);
  }
  scale.tare();  // Esegui la tara
  
  log("Offset calibration complete.", 1);
  //HX711_module::read();
}

void HX711_module::read() {
  bool hx711_state = scale.is_ready();
  log("hx state: " + String(hx711_state), 1);
  if (scale.is_ready()) {
    long reading = scale.read();
    log("HX711 reading: ", 1);
    log(String(reading), 1);
  } else {
    log("HX711 not found.", 1);
  }
}

void HX711_module::readAverage(uint8_t nSamples) {

  if (scale.is_ready()) {
    long reading = scale.read_average();
    log("HX711 reading: " + String(reading), 1);
  } else {
    log("HX711 not found.", 1);
  }
}

void HX711_module::getWeight(byte times)
{ 
  if (scale.is_ready()) {
    log("Reading weight...", 1);
    float weightVal = scale.get_units(times);
    log("weight: " + String(weightVal) + "g",1);
  }else {
  log("LoadCell not found.", 1);
  }
}

HX711_module loadCellADC;