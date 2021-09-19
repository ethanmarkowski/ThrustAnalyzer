// Throttle.cpp

#include "Throttle.h"

Throttle::Throttle(const uint8_t &escPin, const uint8_t &potPin) :
	_escPin(escPin), _potPin(potPin), _isArmed(false), _mode(ThrottleHelper::ThrottleModes::AUTO), _isTestStarted(false), _isTestCompleted(false), _autoRunTime(5000), _autoThrottleNumSteps(5), _autoMaxThrottle(1.0), _idlePulse(1000), _minPulse(1030), _maxPulse(2000), _throttle(0) {}

void Throttle::Arm()
{
	attach(_escPin);
	writeMicroseconds(_minPulse);
	delay(1500);
	_isArmed = true;
	_isTestStarted = false;
}

void Throttle::Disarm()
{
	writeMicroseconds(_minPulse);
	detach();
	_isArmed = false;
}

bool Throttle::GetArmStatus() const { return _isArmed; }

bool Throttle::GetCompletionStatus() const { return _isTestCompleted; }

ThrottleHelper::ThrottleModes Throttle::GetMode() const { return _mode; }

const String Throttle::GetModeString() const { return ThrottleHelper::modeString[(int)_mode]; }

void Throttle::SetMode(const ThrottleHelper::ThrottleModes &mode) { _mode = mode; }

void Throttle::IncrementMode() 
{ 
	switch (_mode)
	{
		case ThrottleHelper::ThrottleModes::POTINPUT: _mode = ThrottleHelper::ThrottleModes::AUTO; break;
		case ThrottleHelper::ThrottleModes::AUTO: break;
	}
}

void Throttle::DecrementMode()
{
	switch (_mode)
	{
		case ThrottleHelper::ThrottleModes::POTINPUT: break;
		case ThrottleHelper::ThrottleModes::AUTO: _mode = ThrottleHelper::ThrottleModes::POTINPUT; break;
	}
}

uint32_t Throttle::GetAutoRunTime() const { return _autoRunTime; }

void Throttle::SetAutoRunTime(const uint32_t &autoRunTime) { _autoRunTime = constrain(autoRunTime, 5000, 120000); }

uint8_t Throttle::GetAutoThrottleNumSteps() const { return _autoThrottleNumSteps; }

void Throttle::SetAutoThrottleNumSteps(const uint8_t &autoThrottleNumSteps) { _autoThrottleNumSteps = constrain(autoThrottleNumSteps, 1, 20); }

float Throttle::GetAutoMaxThrottle() const { return _autoMaxThrottle; }

void Throttle::SetAutoMaxThrottle(const float &autoMaxThrottle) { _autoMaxThrottle = constrain(autoMaxThrottle, 0.01, 1.0); }

float Throttle::_PotInputToThrottle() const { return (float)analogRead(_potPin) / 1024; }

float Throttle::_AutoThrottle()
{
	// Calculate duration for each throttle step with 3 seconds remaining for the final step
	uint32_t stepDuration = (_autoRunTime - 3000) / (_autoThrottleNumSteps - 1);

	// Calculate throttle step interval
	float throttleInterval = _autoMaxThrottle / _autoThrottleNumSteps;

	if (!_isTestStarted)
	{
		_startTime = millis();
		_isTestStarted = true;
	}

	uint32_t currentTime = millis();

	float throttle = constrain(((currentTime - _startTime) / stepDuration + 1) * throttleInterval, 0, _autoMaxThrottle);

	// Throttle setting should be overwritten to 0.0 if the test has concluded
	if (currentTime - _startTime >= _autoRunTime)
	{ 
		throttle = 0.0;
		_isTestCompleted = true;
	}

	return throttle;
}

uint16_t Throttle::_ThrottleToPulse() const { return constrain(_throttle * (_maxPulse - _minPulse) + _minPulse, _minPulse, _maxPulse); }

void Throttle::Run()
{
	// Update throttle setting
	switch (_mode)
	{
		case ThrottleHelper::ThrottleModes::POTINPUT: _throttle = _PotInputToThrottle(); break;

		case ThrottleHelper::ThrottleModes::AUTO: _throttle = _AutoThrottle(); break;
	}

	// Send PWM signal to ESC
	writeMicroseconds(_ThrottleToPulse());
}

float Throttle::GetThrottle() const { return _throttle; }