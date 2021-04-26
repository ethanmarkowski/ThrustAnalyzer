// 
// 
// 

#include "Arduino.h"
#include "CurrentSensor.h"


CurrentSensor::CurrentSensor(const int & pin, const int & numReadings) : _pin(pin), _numReadings(numReadings), _index(0), _maxCurrent(0)
{
	// Initialize current readings buffer
	_readings = new float[_numReadings];
	for (int i = 0; i < _numReadings; ++i)
	{
		_readings[i] = 0;
	}
}

CurrentSensor::~CurrentSensor()
{
	delete[] _readings;
}

void CurrentSensor::Calibrate()
{
	// Calibrate current sensor based on 100 current readings taken over a 500 millisecond interval
	float buffer = 0;
	uint8_t numCalibrationReadings = 100;
	uint16_t calibrationTime = 500;

	for (int i = 0; i < numCalibrationReadings; ++i)
	{
		buffer += ((float) analogRead(_pin) - 512) * 5 / 1024 / 0.04;
		delay(calibrationTime / numCalibrationReadings);
	}

	_calibration = buffer / numCalibrationReadings;
}

void CurrentSensor::Read()
{
	// Read sensor and calculate current sensor value from analog reading
	float buffer = analogRead(_pin);
	_rawCurrent = (buffer - 512) * 5 / 1024 / 0.04 - _calibration;
	_readings[_index] = _rawCurrent;

	// Increment _index or reset to zero if the index is already set to the end of the _readings array
	(_index < _numReadings) ? ++_index : _index = 0;

	// Calculate smoothed current value
	buffer = 0;
	for (int i = 0; i < _numReadings; ++i)
	{
		buffer += _readings[i];
	}
	_smoothedCurrent = buffer / _numReadings;

	// Update _maxCurrent with raw current value
	if (_rawCurrent > _maxCurrent) { _maxCurrent = _rawCurrent; }
}

float CurrentSensor::GetValue()
{
	return _smoothedCurrent;

}

float CurrentSensor::GetRawValue()
{
	return _rawCurrent;
}

float CurrentSensor::GetMaxValue()
{
	return _maxCurrent;
}
