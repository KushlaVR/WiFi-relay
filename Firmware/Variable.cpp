#include "Variable.h"

Collection variables = Collection();


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
	Variable * item = (Variable *)variables.getFirst();
	Variable * last = nullptr;
	while (item != nullptr) {
		if (item->name == name) {
			item->floatValue = value;
			Serial.println("OK");
			return;
		}
		last = item;
		item = (Variable *)item->next;
	}
	item = new Variable();
	item->name = name;
	item->floatValue = value;
	if (last == nullptr)
		variables.add(item);
	else
		last->next = item;
	Serial.println("Added new.");
}

float Variable::getValue(String name)
{
	if (variables.getFirst() == nullptr) return NAN;
	Variable * item = (Variable *)variables.getFirst();
	while (item != nullptr) {
		if (item->name == name) {
			return item->floatValue;
		}
		item = (Variable *)item->next;
	}
	Serial.print("Variable not found:");
	Serial.println(name);
	return NAN;
}
