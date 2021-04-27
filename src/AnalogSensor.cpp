// AnalogSensor.cpp

#include "AnalogSensor.h"

AnalogSensor::AnalogSensor(const int &pin, const bool &bidirectional, const float &sensitivity, const int &numReadings) : 
	_pin(pin), _bidirectional(bidirectional), _sensitivity(sensitivity), _numReadings(numReadings), _index(0), _maxValue(0), _minValue(3.4e38)
{
	// Initialize current readings buffer
	_readings = new float[_numReadings];
	for (int i = 0; i < _numReadings; ++i)
	{
		_readings[i] = 0;
	}
}

AnalogSensor::~AnalogSensor()
{
	delete[] _readings;
}

float AnalogSensor::AnalogToValue() const
{
	// Convert analog reading from sensor to raw current value
	return ((float)analogRead(_pin) - (512 * _bidirectional)) * 5 / 1024 / _sensitivity;
}

void AnalogSensor::Calibrate()
{
	// Calibrate current sensor based on 100 current readings taken over a 500 millisecond interval
	float buffer = 0;
	uint8_t numCalibrationReadings = 100;
	uint16_t calibrationTime = 500;

	for (int i = 0; i < numCalibrationReadings; ++i)
	{
		buffer += AnalogToValue();
		delay(calibrationTime / numCalibrationReadings);
	}

	_calibration = buffer / numCalibrationReadings;
}

void AnalogSensor::Read()
{
	// Read sensor and calculate current sensor value from analog reading
	_rawValue = AnalogToValue() - _calibration;
	_readings[_index] = _rawValue;

	// Increment _index or reset to zero if the index is already set to the end of the _readings array
	(_index < _numReadings) ? ++_index : _index = 0;

	// Calculate smoothed current value
	float buffer = 0;
	for (int i = 0; i < _numReadings; ++i)
	{
		buffer += _readings[i];
	}

	_smoothedValue = buffer / _numReadings;

	// Update _maxValue with raw value
	if (_rawValue > _maxValue) { _maxValue = _rawValue; }

	// Update _minValue with raw value
	if (_rawValue < _minValue) { _minValue = _rawValue; }
}

float AnalogSensor::GetValue() const
{
	return _smoothedValue;
}

float AnalogSensor::GetRawValue() const
{
	return _rawValue;
}

float AnalogSensor::GetMaxValue() const
{
	return _maxValue;
}

float AnalogSensor::GetMinValue() const
{
	return _minValue;
}
