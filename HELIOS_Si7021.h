/*
 * Copyright (c) 2020 Helmut Tschemernjak
 * 30826 Garbsen (Hannover) Germany
 * Licensed under the Apache License, Version 2.0);
 */

#ifdef ARDUINO
 #include "Arduino.h"
 #ifndef PinName
  typedef int PinName;
  #ifndef NC
    #define NC -1
  #endif
 #endif
#endif

#ifndef __HELIOS_Si7021_H__
#define __HELIOS_Si7021_H__

class HELIOS_Si7021 {
public:
	HELIOS_Si7021(PinName sda = NC, PinName scl = NC);
	~HELIOS_Si7021();
	
	/*
	 * When skipDeviceInit is called after the Constructor before any other
	 * calls the inital device reset will be skipped, to avoid the 50 ms
	 * reset delay. This allows the ESP32 MCU after deepsleep wakeups to skip
	 * the device init, which was already done during the cold boot.
	 */
	void skipDeviceInit() { _skipDeviceInit = true; };

	/*
	 * check if the sensor is avilable
	 */
	bool hasSensor(void);

	/*
	 * Issue a reset, gets automatically done on first access.
	 */
	void reset(void);
	
	/*
	 * reads and returns the sensor serial number
	 */
	uint64_t getSerialNumber(void);
	
	/*
	 * reads and returns the sensor revision
	 */
	int getRevision(void);
	
	float readTemperature(void);
	float readHumidity(void);
	
	enum sensorType {
		SI_Engineering_Samples,
		SI_7013,
		SI_7020,
		SI_7021,
		SI_unkown,
	};
	const char *getModelName(void);
	sensorType getModel(void);
	
private:
	char _data[8];
#ifdef __MBED__
    I2C *_i2c;
	const static uint8_t _i2caddr = 0x40 << 1; // convert from 7 to 8 bit.
#else
	const static int8_t _i2caddr = 0x40;
#endif

	bool _foundDevice;
	bool _initDone;
	bool _skipDeviceInit;
	PinName _sda, _scl;
	/*
	 * initializes the sensor and reads it version
	 * return true if the device has been found.
	 * In case API access gets done before calling begin(),
	 * begin will be called automatically.
	 */
	bool _init(void);
	uint8_t _readRegister8(uint8_t reg);
	uint8_t _readBytes(char *buffer, int len);
	uint8_t _readCmdBytesTimeout(uint8_t reg, char *buffer, int len, int timeout_ms);
	void _writeRegister8(uint8_t reg);
	void _writeRegister8x2(uint8_t reg, uint8_t reg2);
	void _waitMillis(int millis);

	const static int _TRANSACTION_TIMEOUT = 100; // Wire NAK/Busy timeout in ms
};

/**************************************************************************/

#endif // __HELIOS_Si7021_H__

