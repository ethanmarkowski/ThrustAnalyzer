// AnalogButtons.cpp

#include "AnalogButtons.h"

AnalogButtons::AnalogButtons(const uint8_t &pin) : _pin(pin), _state(States::OPEN) {}

Buttons AnalogButtons::GetInput()
{
	uint16_t input = analogRead(_pin);

	switch (_state)
	{
	case States::OPEN: if (input < (int)Buttons::SELECT)
	{
		_state = States::PRESSED;

		if (input < (int)Buttons::RIGHT) { _button = Buttons::RIGHT; }
		else if (input < (int)Buttons::UP) { _button = Buttons::UP; }
		else if (input < (int)Buttons::DOWN) { _button = Buttons::DOWN; }
		else if (input < (int)Buttons::LEFT) { _button = Buttons::LEFT; }
		else if (input < (int)Buttons::SELECT) { _button = Buttons::SELECT; }
	}

		break;

	case States::PRESSED: if (input > (int)Buttons::SELECT) { _state = States::OPEN; return _button; }
	}

	return Buttons::NONE;
}