// CurrentSensor.cpp

#include "Arduino.h"
#include "CurrentSensor.h"

CurrentSensor::CurrentSensor(const int &pin, const int &numReadings) : BaseSensor(pin, numReadings) {};

float CurrentSensor::AnalogToValue() const
{
	// Convert analog reading from sensor to raw current value
	return ((float)analogRead(_pin) - 512) * 5 / 1024 / 0.04;
}