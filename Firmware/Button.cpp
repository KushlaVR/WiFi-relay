// 
// 
// 

#include "Button.h"


void ButtonBase::doPress()
{
	if (press != nullptr) press(argument);
}

void ButtonBase::doHold()
{
	if (hold != nullptr) hold(argument);
}

void ButtonBase::doRelease()
{
	if (release != nullptr) release(argument);
}

bool ButtonBase::isPressed()
{
	return false;
}

bool ButtonBase::isReleased()
{
	return false;
}

void ButtonBase::handle()
{
	if (!isEnabled) return;

	bool newState = isPressed();
	if (newState)
		lastImpuls = millis();


	if (pressedTime == 0 && (millis() - releasedTime) > pressDebounce) {//Кнопку відпустилт досатньо давно
		if (millis() - lastImpuls < pressDebounce) {//Щойно зафіксовано імпульс на піні
			pressedTime = millis();//Вважаємо що кнопка нажата
			releasedTime = 0;
			state = true;
			doPress();
		}
	}
	else if (releasedTime == 0 && (millis() - pressedTime) > releaseDebounce) {//Кнопка не відпущена, і з моменту нажа
		if ((millis() - pressedTime) > holdInterval) {//Кнопка нажата достатньо, щоб вважати її тако що утримується
			pressedTime = millis();
			doHold();
		}
		if (millis() - lastImpuls > pressDebounce) {//Кнопку відпустили іімпуль більше не фіксується
			pressedTime = 0;
			releasedTime = millis();
			state = false;
			doRelease();
		}
	}
}



Button::Button(int pin, void(*press)(void * arg))
{
	this->pin = pin;
	pinMode(pin, INPUT);
	this->press = press;
}
Button::Button(int pin, void(*press)(void * arg), void(*hold)(void * arg))
{
	this->pin = pin;
	pinMode(pin, INPUT);
	this->press = press;
	this->hold = hold;
}
Button::Button(int pin, void(*press)(void * arg), void(*hold)(void * arg), void(*release)(void * arg))
{
	this->pin = pin;
	pinMode(pin, INPUT_PULLUP);
	this->press = press;
	this->hold = hold;
	this->release = release;
}

bool Button::isPressed()
{
	return (digitalRead(pin) == condition);
}

bool Button::isReleased()
{
	return !(digitalRead(pin) == condition);
}





