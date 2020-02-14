/*
 * Copyright (c) 2020 Helmut Tschemernjak
 * 30826 Garbsen (Hannover) Germany
 * Licensed under the Apache License, Version 2.0);
 */

#ifdef __MBED__
 #include "main.h"
#elif ARDUINO
 #define FEATURE_SI7021
 #include <Wire.h>
#else
 #error "unkown platform"
typedef PinName int;
#endif

#ifdef FEATURE_SI7021

#include <HELIOS_Si7021.h>

#ifndef UNUSED
 #define UNUSED(x) (void)(x)
#endif


#define SI7021_MEASRH_HOLD_CMD		0xE5 // Measure Relative Humidity, Hold Master Mode
#define SI7021_MEASRH_NOHOLD_CMD	0xF5 // Measure Relative Humidity, No Hold Master Mode
#define SI7021_MEASTEMP_HOLD_CMD	0xE3 // Measure Temperature, Hold Master Mode
#define SI7021_MEASTEMP_NOHOLD_CMD	0xF3 // Measure Temperature, No Hold Master Mode
#define SI7021_READPREVTEMP_CMD		0xE0 // Read Temperature Value from Previous RH Measurement
#define SI7021_RESET_CMD			0xFE
#define SI7021_WRITERHT_REG_CMD		0xE6 // Write RH/T User Register 1
#define SI7021_READRHT_REG_CMD		0xE7 // Read RH/T User Register 1
#define SI7021_WRITEHEATER_REG_CMD	0x51 // Write Heater Control Register
#define SI7021_READHEATER_REG_CMD	0x11 // Read Heater Control Register
#define SI7021_ID1_CMD				0xFA0F // Read Electronic ID 1st Byte
#define SI7021_ID2_CMD				0xFCC9 // Read Electronic ID 2nd Byte
#define SI7021_FIRMVERS_CMD			0x84B8 // Read Firmware Revision

#define SI7021_REV_1				0xff
#define SI7021_REV_2				0x20



/**************************************************************************/

HELIOS_Si7021::HELIOS_Si7021(PinName sda, PinName scl)
{
	sernum_a = sernum_b = 0;
	_model = SI_7021;
	_revision = 0;
	_foundDevice = false;
	_initDone = false;
	
	_sda = sda;
	_scl = scl;
#ifdef __MBED__
	_i2c = NULL;
#endif
}


bool
HELIOS_Si7021::_init(void)
{
	if (_initDone)
		return true;
		
#ifdef __MBED__
	if (!_i2c)
		_i2c = new I2C(_sda, _scl);
#elif ARDUINO
	Wire.begin();
#else
#error "Unkown OS"
#endif

	reset();
	
	if (_readRegister8(SI7021_READRHT_REG_CMD) != 0x3A)
		return false;

	_initDone = true;
	_foundDevice = true;
	_readSerialNumber();
	_readRevision();
	return true;
}


HELIOS_Si7021::~HELIOS_Si7021(void)
{
#ifdef __MBED__
	if (_i2c)
		delete _i2c;
#endif
}


void HELIOS_Si7021::reset(void)
{
	_writeRegister8(SI7021_RESET_CMD);
	_waitMillis(50);
}

bool
HELIOS_Si7021::hasSensor(void)
{
	if (!_initDone)
		_init();
	return _foundDevice;
}


void
HELIOS_Si7021::_readSerialNumber(void)
{
	if (!_initDone)
		_init();
	if (!_foundDevice)
		return;

	_writeRegister8x2(SI7021_ID1_CMD >> 8, SI7021_ID1_CMD & 0xFF);
	_readBytes(_data, 8);

	sernum_a = _data[0];
	sernum_a <<= 8;
	sernum_a |= _data[2];
	sernum_a <<= 8;
	sernum_a |= _data[4];
	sernum_a <<= 8;
	sernum_a |= _data[6];
	
	
	_writeRegister8x2(SI7021_ID2_CMD >> 8, SI7021_ID2_CMD & 0xFF);
	_readBytes(_data, 8);

	sernum_b = _data[0];
	sernum_b <<= 8;
	sernum_b |= _data[2];
	sernum_b <<= 8;
	sernum_b |= _data[4];
	sernum_b <<= 8;
	sernum_b |= _data[6];
}


uint64_t
HELIOS_Si7021::getSerialNumber(void)
{
	return (uint64_t)sernum_a << 32 | (uint64_t)sernum_b;
}

void HELIOS_Si7021::_readRevision(void)
{
	if (!_initDone)
		_init();
	if (!_foundDevice)
		return;
		
	_revision = 0;

	_writeRegister8x2(SI7021_FIRMVERS_CMD >> 8, SI7021_FIRMVERS_CMD & 0xFF);
	_readBytes(_data, 8);
	
	if (_data[0] == SI7021_REV_1) {
          _revision = 1;
	} else if (_data[0] == SI7021_REV_2) {
          _revision = 2;
	}
}


float HELIOS_Si7021::readTemperature(void) {
	if (!_initDone)
		_init();
	if (!_foundDevice)
		return NAN;


	if (_readCmdBytesTimeout(SI7021_MEASTEMP_NOHOLD_CMD, _data, 3, 6) != 3)
		return NAN;

	float temperature = _data[0] << 8 | _data[1];
	temperature *= 175.72;
	temperature /= 65536;
	temperature -= 46.85;

	return temperature;
}


float HELIOS_Si7021::readHumidity(void)
{
	if (!_initDone)
		_init();
	if (!_foundDevice)
		return NAN;

	if (_readCmdBytesTimeout(SI7021_MEASRH_NOHOLD_CMD, _data, 3, 6) != 3)
		return NAN;

	float humidity = (_data[0] << 8 | _data[1]) * 125;
	humidity /= 65536;
	humidity -= 6;

	return humidity;
}


const char *HELIOS_Si7021::getModelName(void)
{
	if (!_initDone)
		_init();
	if (!_foundDevice)
		return "no device found";

	switch(_model) {
		case SI_Engineering_Samples:
			return "SI engineering samples";
		case SI_7013:
			return "Si7013";
		case SI_7020:
			return "Si7020";
		case SI_7021:
			return "Si7021";
		case SI_unkown:
		default:
			return "unknown";
	}
}


HELIOS_Si7021::sensorType HELIOS_Si7021::getModel(void)
{
	if (!_initDone)
		_init();
	return _model;
}

uint8_t
HELIOS_Si7021::_readRegister8(uint8_t reg) {
#ifdef  __MBED__
	_data[0] = reg;
	_i2c->write(_i2caddr, _data, 1);
	_i2c->read(_i2caddr, _data, 1);
	return _data[0];
#elif ARDUINO
  uint8_t value;
  Wire.beginTransmission(_i2caddr);
  Wire.write(reg);
  Wire.endTransmission(false);

  uint32_t start = millis(); // start timeout
  while(millis()-start < _TRANSACTION_TIMEOUT) {
    if (Wire.requestFrom(_i2caddr, 1) == 1) {
      value = Wire.read();
      return value;
    }
    _waitMillis(2);
  }
  return 0; // Error timeout
#else
#error "Unknown OS"
#endif
}

uint8_t
HELIOS_Si7021::_readBytes(char *buffer, int len)
{
#ifdef __MBED__
	_i2c->read(_i2caddr, buffer, len);
#elif ARDUINO
	bool gotData = false;
	uint32_t start = millis(); // start timeout
	while(millis()-start < _TRANSACTION_TIMEOUT) {
	  if (Wire.requestFrom(_i2caddr, len) == len) {
		gotData = true;
		break;
	  }
	  delay(2);
	}
	if (!gotData)
	  return 0; // error timeout
	for (int i = 0; i < len; i++)
		*buffer++ = Wire.read();
#else
#error "Unkown OS"
#endif
	return len;
}


uint8_t
HELIOS_Si7021::_readCmdBytesTimeout(uint8_t reg, char *buffer, int len, int timeout_ms)
{
_data[0] = reg;

#ifdef __MBED__
	_i2c->write(_i2caddr, _data, 1);
	Timer t;
	t.start();
	while(_i2c->read(_i2caddr, _data, len) != 0) {
		if (t.read_ms() > _TRANSACTION_TIMEOUT)
			return 0;
		_waitMillis(timeout_ms); // 1/2 typical sample processing time
	}
	return len;

#elif ARDUINO
	Wire.beginTransmission(_i2caddr);
	Wire.write(_data[0]);
	uint8_t err = Wire.endTransmission(false);
		 
#ifdef ARDUINO_ARCH_ESP32
	if(err && err != I2C_ERROR_CONTINUE) //ESP32 has to queue ReSTART operations.
#else
	if (err != 0)
#endif
		return 0; //error

	uint32_t start = millis(); // start timeout
	while(millis()-start < _TRANSACTION_TIMEOUT) {
		if (Wire.requestFrom(_i2caddr, 3) == 3) {
			for (int i = 0; i < 3; i++)
				_data[i] = Wire.read();
			return len;
		}
		delay(6); // 1/2 typical sample processing time
	}
	return 0; // Error timeout
#else
#error "Unkown OS"
#endif
}



void
HELIOS_Si7021::_writeRegister8(uint8_t reg)
{
#ifdef __MBED__
	_data[0] = reg;
	_i2c->write(_i2caddr, _data, 1);
#elif ARDUINO
	Wire.beginTransmission(_i2caddr);
	Wire.write(reg);
	Wire.endTransmission();
#else
#error "Unkown OS"
#endif
}

void
HELIOS_Si7021::_writeRegister8x2(uint8_t reg, uint8_t reg2)
{
#ifdef __MBED__
	_data[0] = SI7021_ID1_CMD >> 8;
	_data[1] = SI7021_ID1_CMD & 0xFF;
	_i2c->write(_i2caddr, _data, 2);
#elif ARDUINO
	Wire.beginTransmission(_i2caddr);
	Wire.write(reg);
	Wire.write(reg2);
	Wire.endTransmission();
#else
#error "Unkown OS"
#endif
}


void
HELIOS_Si7021::_waitMillis(int millis)
{
#ifdef __MBED__
	wait_ms(millis);
#else
	delay(millis);
#endif
}

#endif // FEATURE_SI7021
