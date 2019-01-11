#pragma once
#if defined(ARDUINO) && ARDUINO >= 100
#include "arduino.h"
#else
#include "WProgram.h"
#endif
class Variable
{
public:
	Variable();
	~Variable();

	Variable * next;

	String name = "";

	float floatValue = NAN;

	static void setValue(String name, float value);
	static float getValue(String name);
};

