// ISensor.h

#ifndef _ISensor_h
#define _ISensor_h


#include <arduino.h>

class ISensor
{
public:

	// Calibrate sensor
	virtual void Calibrate() = 0;

	// Check if sensor has been calibrated
	virtual bool GetIsCalibrated() const = 0;

	// Read a new sensor value
	virtual void Update() = 0;

	// Return the most recent sensor value with data smoothing
	virtual float GetValue() const = 0;

	// Return the most recent raw sensor value
	virtual float GetRawValue() const = 0;

	// Return the maximum raw value recorded
	virtual float GetMaxValue() const = 0;

	// Return the minimum raw value recorded
	virtual float GetMinValue() const = 0;

	// Return the upper safeguard value
	virtual float GetUpperSafeguard() const = 0;

	// Set the upper safeguard value
	virtual void SetUpperSafeguard(const float &upperSafeguard) = 0;

	// Return the lower safeguard value
	virtual float GetLowerSafeguard() const = 0;

	// Set the lower safeguard value
	virtual void SetLowerSafeguard(const float &lowerSafeguard) = 0;
};

#endif