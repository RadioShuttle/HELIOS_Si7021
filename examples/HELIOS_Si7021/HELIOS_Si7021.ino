
#define FEATURE_SI7021

#include "HELIOS_Si7021.h"


#if defined(ARDUINO_SAMD_ZERO) || defined(ARDUINO_ARCH_SAMD)
#define MYSERIAL SerialUSB
#else
#define MYSERIAL Serial
#endif


HELIOS_Si7021 si7021;


void setup() {
  MYSERIAL.begin(115200);
  while (!MYSERIAL)
    ; // wait for serial port to connect. Needed for native USB port only
    
  MYSERIAL.println("Welcome to the HELIOS_Si7021 example");
  
  if (!si7021.hasSensor()) {
    MYSERIAL.println("No Si7021 sensor found!");
    while (true)
      ;
  }
  
  MYSERIAL.println("Found " + String(si7021.getModelName()) + " Rev(" + String(si7021.getRevision()) + ")");
  uint64_t serno = si7021.getSerialNumber();
  MYSERIAL.print("Serial number: ");
  MYSERIAL.print((uint32_t)(serno >> 32) ,HEX);
  MYSERIAL.print(":");
  MYSERIAL.println((uint32_t)(serno & 0xffffffff) ,HEX);
}

void loop() {
  MYSERIAL.print("Temperature: " + String(si7021.readTemperature(), 2) + "  ");
  MYSERIAL.print("Humidity: " + String(si7021.readHumidity(), 1) + "%");
  MYSERIAL.println("");
  delay(1000);
}
