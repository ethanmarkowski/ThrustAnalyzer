// Throttle.h

#ifndef _Throttle_h
#define _Throttle_h

#include <Arduino.h>
#include <Servo.h>

namespace ThrottleHelper
{
	enum class ThrottleModes : uint8_t { POTINPUT, AUTO };
	const String modeString[] = { "Manual throttle", "Auto throttle" };
}

class Throttle : protected Servo
{
private:
	const uint8_t _escPin;
	const uint8_t _potPin;
	bool _isArmed;
	ThrottleHelper::ThrottleModes _mode;
	bool _isTestStarted;
	bool _isTestCompleted;
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
	Throttle(const uint8_t &escPin, const uint8_t &potPin);
	void Arm();
	void Disarm();
	bool GetArmStatus() const;
	bool GetCompletionStatus() const;
	ThrottleHelper::ThrottleModes GetMode() const;
	const String GetModeString() const;
	void SetMode(const ThrottleHelper::ThrottleModes &mode);
	void IncrementMode();
	void DecrementMode();
	uint32_t GetAutoRunTime() const;
	void SetAutoRunTime(const uint32_t &autoRunTime);
	uint8_t GetAutoThrottleNumSteps() const;
	void SetAutoThrottleNumSteps(const uint8_t &autoThrottleNumSteps);
	float GetAutoMaxThrottle() const;
	void SetAutoMaxThrottle(const float &autoMaxThrottle);
	void Run();
	float GetThrottle() const;
};

#endif