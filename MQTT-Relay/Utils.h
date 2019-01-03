#pragma once
#if defined(ARDUINO) && ARDUINO >= 100
#include "arduino.h"
#else
#include "WProgram.h"
#endif
#include <TimeLib.h>

class Utils
{
public:
	Utils();
	~Utils();

	static String FormatTime(time_t time);
};

