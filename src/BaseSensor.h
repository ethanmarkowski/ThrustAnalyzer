// BaseSensor.h

#ifndef _BaseSensor_h
#define _BaseSensor_h


#include "arduino.h"
#include "ISensor.h"

class BaseSensor : public ISensor
{
private:
	BaseSensor();

protected:
	const uint8_t _pin;
	const uint8_t _numReadings;
	uint8_t _index;
	float *_readings;
	float _calibration;
	float _rawValue;
	float _smoothedValue;
	float _maxValue;
	float _minValue;

	BaseSensor(const int &pin, const int &numReadings);
	~BaseSensor();
	virtual float AnalogToValue() const = 0;

public:
	void Calibrate();
	void Read();
	float GetValue() const;
	float GetRawValue() const;
	float GetMaxValue() const;
	float GetMinValue() const;
};


#endif

