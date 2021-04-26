// CurrentSensor.h

#ifndef _CURRENTSENSOR_h
#define _CURRENTSENSOR_h

#include "arduino.h"

class CurrentSensor
{
private:
	const uint8_t _pin;
	const uint8_t _numReadings;
	uint8_t _index;
	float * _readings;
	float _calibration;
	float _rawCurrent;
	float _smoothedCurrent;
	float _maxCurrent;

public:
	CurrentSensor(const int & pin, const int & numReadings);
	~CurrentSensor();
	void Calibrate();
	void Read();
	float GetValue();
	float GetRawValue();
	float GetMaxValue();
};

#endif

