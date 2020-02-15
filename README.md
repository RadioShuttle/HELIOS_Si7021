# HELIOS_Si7021

A common driver supporting the Si7021 temperature and humidity sensor for Arduino and Mbed OS. I started with the Adafruit Si7021 driver for Arduino and spent significant development time on this. Later on, I started over to create a Si7021 driver for Mbed OS. Now I combined the Mbed OS and Arduino versions into a single source.The advantage of the HELIOS_Si7021 driver is that it requires less memory, initializes much faster, allows custom I2C pins, and is fully supported.

Helmut Tschemernjak
www.radioshuttle.de

## Supported platforms
- Mbed OS
- Arduino (ESP32 has been tested)


## TODOs
- Add checksum support for temperature and humidity data

##  Credits
This driver has initially been written by the RadioShuttle engineers (www.radioshuttle.de). Many thanks to everyone who helped bringing this project forward.
