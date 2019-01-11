#include "Variable.h"

static Variable * _firstVariable = nullptr;

Variable::Variable()
{
	next = nullptr;
}


Variable::~Variable()
{
}

void Variable::setValue(String name, float value)
{
	Serial.printf("  Valiable[%s]=%f;", name.c_str(), value);
	Variable * item = _firstVariable;
	Variable * last = nullptr;
	while (item != nullptr) {
		if (item->name == name) {
			item->floatValue = value;
			Serial.println("OK");
			return;
		}
		last = item;
		item = item->next;
	}
	item = new Variable();
	item->name = name;
	item->floatValue = value;
	if (last == nullptr)
		_firstVariable = item;
	else
		last->next = item;
	Serial.println("Added new.");
}

float Variable::getValue(String name)
{
	if (_firstVariable == nullptr) return NAN;
	Variable * item = _firstVariable;
	while (item != nullptr) {
		if (item->name == name) {
			return item->floatValue;
		}
		item = item->next;
	}
	Serial.print("Variable not found:");
	Serial.println(name);
	return NAN;
}
