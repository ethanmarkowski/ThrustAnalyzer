// AnalogButtons.h

#ifndef _Button_h
#define _Button_h

#include <Arduino.h>

enum class Buttons { RIGHT = 50, UP = 300, DOWN = 500, LEFT = 700, START = 900, NONE };

class AnalogButtons
{
private:
	enum class States { OPEN, PRESSED };

	const uint8_t _pin;
	States _state;
	Buttons _button;

public:
	AnalogButtons(const uint8_t &pin);
	Buttons GetInput();
};

#endif