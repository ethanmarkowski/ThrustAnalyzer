// CurrentSensor.h

#ifndef _CURRENTSENSOR_h
#define _CURRENTSENSOR_h

#include "arduino.h"
#include "BaseSensor.h"

class CurrentSensor : public BaseSensor
{
private:
	float AnalogToValue() const;

public:
	CurrentSensor(const int &pin, const int &numReadings);
};

#endif

