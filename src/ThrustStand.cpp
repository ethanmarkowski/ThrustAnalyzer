// ThrustStand.cpp

#include "ThrustStand.h"

ThrustStand::ThrustStand(const uint8_t &doutPin, const uint8_t &sckPin, const float &scaleFactor, const uint8_t &numReadings, const int8_t &upperLimit, const int8_t &lowerLimit) :
	_doutPin(doutPin), _sckPin(sckPin), _scaleFactor(scaleFactor), _isCalibrated(false), _numReadings(numReadings), _index(0), _upperLimit(upperLimit), _lowerLimit(lowerLimit), _upperSafeguard(upperLimit), _lowerSafeguard(lowerLimit)
{
	// Pin assignments for underlying HX711 object
	begin(_doutPin, _sckPin);

	// Set conversion factor from raw ADC readings to actual values
	set_scale(_scaleFactor);

	// Initialize current readings buffer
	_readings = new float[_numReadings];
	for (uint8_t i = 0; i < _numReadings; ++i)
	{
		_readings[i] = 0;
	}
}

ThrustStand::ThrustStand(const uint8_t &doutPin, const uint8_t &sckPin, const float &scaleFactor, const uint8_t &numReadings) :
	ThrustStand(doutPin, sckPin, scaleFactor, numReadings, 127, -127) {}

ThrustStand::~ThrustStand() { delete[] _readings; }

void ThrustStand::Calibrate()
{
	tare();
	_isCalibrated = true;
}

bool ThrustStand::GetIsCalibrated() const { return _isCalibrated; }

void ThrustStand::Update()
{
	// Read sensor and calculate value from analog reading
	_rawValue = get_units(1);
	_readings[_index] = _rawValue;

	// Increment _index or reset to zero if the index is already set to the end of the _readings array
	(_index < _numReadings - 1) ? ++_index : _index = 0;

	// Calculate smoothed value
	float buffer = 0;
	for (uint8_t i = 0; i < _numReadings; ++i)
	{
		buffer += _readings[i];
	}

	_smoothedValue = buffer / _numReadings;

	// Update _maxValue with raw value
	if (_rawValue > _maxValue) { _maxValue = _rawValue; }

	// Update _minValue with raw value
	if (_rawValue < _minValue) { _minValue = _rawValue; }
}

float ThrustStand::GetValue() const
{
	return _smoothedValue;
}

float ThrustStand::GetRawValue() const
{
	return _rawValue;
}

float ThrustStand::GetMaxValue() const
{
	return _maxValue;
}

float ThrustStand::GetMinValue() const
{
	return _minValue;
}

float ThrustStand::GetUpperSafeguard() const { return _upperSafeguard; }

void ThrustStand::SetUpperSafeguard(const float &upperSafeguard) { _upperSafeguard = constrain(upperSafeguard, _upperLimit, _lowerLimit); }

float ThrustStand::GetLowerSafeguard() const { return _lowerSafeguard; }

void ThrustStand::SetLowerSafeguard(const float &lowerSafeguard) { _lowerSafeguard = constrain(lowerSafeguard, _upperLimit, _lowerLimit); }