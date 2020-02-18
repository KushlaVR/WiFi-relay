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


	if (pressedTime == 0 && (millis() - releasedTime) > pressDebounce) {//������ ��������� �������� �����
		if (millis() - lastImpuls < pressDebounce) {//����� ����������� ������� �� ��
			pressedTime = millis();//������� �� ������ ������
			releasedTime = 0;
			state = true;
			doPress();
		}
	}
	else if (releasedTime == 0 && (millis() - pressedTime) > releaseDebounce) {//������ �� ��������, � � ������� ����
		if ((millis() - pressedTime) > holdInterval) {//������ ������ ���������, ��� ������� �� ���� �� ����������
			pressedTime = millis();
			doHold();
		}
		if (millis() - lastImpuls > pressDebounce) {//������ ��������� ������� ����� �� ���������
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





