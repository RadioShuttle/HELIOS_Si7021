
#define FEATURE_SI7021

#include "HELIOS_Si7021.h"


HELIOS_Si7021 si7021;


void setup() {
  Serial.begin(115200);

  Serial.println("Welcome to the HELIOS_Si7021 example");
  
  if (!si7021.begin()) {
    Serial.println("No Si7021 sensor found!");
    while (true)
      ;
  }
  
  Serial.println("Found " + String(si7021.getModelName()) + " Rev(" + String(si7021.getRevision()) + ")");
}

void loop() {
  Serial.print("Temperature: " + String(si7021.readTemperature(), 2) + "  ");
  Serial.print("Humidity: " + String(si7021.readHumidity(), 1) + "%");
  Serial.println("");
  delay(1000);
}
