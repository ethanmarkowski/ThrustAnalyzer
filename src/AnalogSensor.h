// AnalogSensor.h

#ifndef _AnalogSensor_h
#define _AnalogSensor_h


#include <arduino.h>
#include "ISensor.h"

class AnalogSensor : public ISensor
{
private:
	const uint8_t _pin;
	const bool _bidirectional;
	const float _sensitivity;
	float _calibration;
	const uint8_t _numReadings;
	uint8_t _index;
	float *_readings;
	float _rawValue;
	float _smoothedValue;
	float _maxValue;
	float _minValue;
	int8_t _upperLimit;
	int8_t _lowerLimit;
	int8_t _upperSafeguard;
	int8_t _lowerSafeguard;

	AnalogSensor();
	float AnalogToValue() const;

public:
	AnalogSensor(const uint8_t &pin, const bool &bidirectional, const float &sensitivity, const uint8_t &numReadings, const int8_t &upperLimit, const int8_t &lowerLimit);
	AnalogSensor(const uint8_t &pin, const bool &bidirectional, const float &sensitivity, const uint8_t &numReadings);
	AnalogSensor(const uint8_t &pin, const bool &bidirectional, const float &sensitivity);
	~AnalogSensor();
	void Calibrate();
	void Update();
	float GetValue() const;
	float GetRawValue() const;
	float GetMaxValue() const;
	float GetMinValue() const;
	int8_t GetUpperSafeguard() const;
	void SetUpperSafeguard(const int8_t &upperSafeguard);
	int8_t GetLowerSafeguard() const;
	void SetLowerSafeguard(const int8_t &lowerSafeguard);
};

#endif