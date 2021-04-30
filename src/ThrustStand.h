// ThrustStand.h

#ifndef _ThrustStand_h
#define _ThrustStand_h

#include <Arduino.h>
#include <HX711.h>
#include "ISensor.h"

class ThrustStand : protected HX711, public ISensor
{
private:
	const uint8_t _doutPin;
	const uint8_t _sckPin;
	const float _scaleFactor;
	const uint8_t _numReadings;
	uint8_t _index;
	float *_readings;
	float _rawValue;
	float _smoothedValue;
	float _maxValue;
	float _minValue;

public:
	ThrustStand(const uint8_t &doutPin, const uint8_t &sckPin, const float &scaleFactor, const uint8_t &numReadings);
	~ThrustStand();
	void Calibrate();
	void Update();
	float GetValue() const;
	float GetRawValue() const;
	float GetMaxValue() const;
	float GetMinValue() const;
};

#endif