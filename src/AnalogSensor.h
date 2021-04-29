// AnalogSensor.h

#ifndef _AnalogSensor_h
#define _AnalogSensor_h


#include "arduino.h"
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
	float* _readings;
	float _rawValue;
	float _smoothedValue;
	float _maxValue;
	float _minValue;

	AnalogSensor();
	float AnalogToValue() const;

public:
	AnalogSensor(const uint8_t &pin, const bool &bidirectional, const float &sensitivity, const uint8_t &numReadings);
	~AnalogSensor();
	void Calibrate();
	void Update();
	float GetValue() const;
	float GetRawValue() const;
	float GetMaxValue() const;
	float GetMinValue() const;
};

#endif

