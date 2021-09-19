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
	bool _isCalibrated;
	const uint8_t _numReadings;
	uint8_t _index;
	float *_readings;
	float _rawValue;
	float _smoothedValue;
	float _maxValue;
	float _minValue;
	int8_t _upperLimit;
	int8_t _lowerLimit;
	float _upperSafeguard;
	float _lowerSafeguard;

public:
	ThrustStand(const uint8_t &doutPin, const uint8_t &sckPin, const float &scaleFactor, const uint8_t &numReadings, const int8_t &upperLimit, const int8_t &lowerLimit);
	ThrustStand(const uint8_t &doutPin, const uint8_t &sckPin, const float &scaleFactor, const uint8_t &numReadings);
	~ThrustStand();
	void Calibrate();
	bool GetIsCalibrated() const;
	void Update();
	float GetValue() const;
	float GetRawValue() const;
	float GetMaxValue() const;
	float GetMinValue() const;
	float GetUpperSafeguard() const;
	void SetUpperSafeguard(const float &upperSafeguard);
	float GetLowerSafeguard() const;
	void SetLowerSafeguard(const float &lowerSafeguard);
};

#endif