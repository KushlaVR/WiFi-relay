#pragma once
#if defined(ARDUINO) && ARDUINO >= 100
#include "arduino.h"
#else
#include "WProgram.h"
#endif
#include <TimeLib.h>
#include "Utils.h"

class SerialController
{
	int cmdPos = 0;
	char cmd[256];

public:
	SerialController();
	~SerialController();

	void loop();

	void cmdTime(String cmd);
};

