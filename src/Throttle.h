// Throttle.h

#ifndef _Throttle_h
#define _Throttle_h

#include <Arduino.h>
#include <Servo.h>

class Throttle : protected Servo
{
private:
	const uint8_t _escPin;
	const uint8_t _potPin;
	bool _isArmed;
	int8_t _mode;
	bool _isTestStarted;
	uint32_t _startTime;
	uint32_t _autoRunTime;
	uint8_t _autoThrottleNumSteps;
	float _autoMaxThrottle;
	const uint16_t _idlePulse;
	const uint16_t _minPulse;
	const uint16_t _maxPulse;
	float _throttle;

	float _PotInputToThrottle() const;
	float _AutoThrottle();
	uint16_t _ThrottleToPulse() const;

public:
	enum Modes { POTINPUT, AUTO, NUM_MODES = AUTO };

	Throttle(const uint8_t &escPin, const uint8_t &potPin);
	void Arm();
	void Disarm();
	bool GetArmStatus() const;
	int8_t GetMode() const;
	void SetMode(const int8_t &mode);
	uint32_t GetAutoRunTime() const;
	void SetAutoRunTime(const uint32_t &autoRunTime);
	void SetAutoThrottleNumSteps(const uint8_t &autoThrottleNumSteps);
	float GetAutoMaxThrottle() const;
	void SetAutoMaxThrottle(const float &autoMaxThrottle);
	bool GetThrottle() const;
	void Run();
};

#endif

