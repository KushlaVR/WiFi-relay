#pragma once
#include "Arduino.h"
#include "Collection.h"

class Variable: public Item
{
public:
	Variable();
	~Variable();

	String name = "";

	float floatValue = NAN;

	static void setValue(String name, float value);
	static float getValue(String name);
};
