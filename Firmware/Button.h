// Button.h

#ifndef _BUTTON_h
#define _BUTTON_h

#if defined(ARDUINO) && ARDUINO >= 100
#include "arduino.h"
#else
#include "WProgram.h"
#endif

class ButtonBase {
protected:
	unsigned long pressedTime;
	unsigned long releasedTime;
	unsigned long lastImpuls;
	void(*press)(void * arg);
	void(*hold)(void * arg);
	void(*release)(void * arg);

	virtual void doPress();
	virtual void doHold();
	virtual void doRelease();
public:

	void * argument = nullptr;

	//Тривалість дребіжання контактів
	int pressDebounce = 100;
	int releaseDebounce = 100;
	bool isEnabled = true;
	bool state = false;
	//Час до фіксації кнопки
	unsigned long holdInterval = 2000;
	virtual bool isPressed();
	virtual bool isReleased();
	virtual void handle();
};


class Button :public ButtonBase {
private:

public:
	int pin;
	int condition = 1;
	Button(int pin, void(*press)(void * arg));
	Button(int pin, void(*press)(void * arg), void(*hold)(void * arg));
	Button(int pin, void(*press)(void * arg), void(*hold)(void * arg), void(*release)(void * arg));
	virtual bool isPressed();
	virtual bool isReleased();
};

#endif

