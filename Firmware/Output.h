#pragma once
#include <FS.h>
#include "Collection.h"
#include "Json.h"

class Output : public Item
{
	String configFile;
	String name;
	int pin;
	int onState;
	int offState;
	bool _on;

public:
	Output(String name, int pin, int onState, int offState);
	~Output();

	String getName() { return name; }
	bool isOn() { return _on; }
	void setup(String configDir);
	void saveStartup(bool state);
	void loop();
	void setState(bool isOn);
	int getSort() { return Index + 1; };
	void printInfo(JsonString * json);
};


class Outputs : public Collection {
public:
	String configDir;
	Outputs();
	~Outputs() {};
	Output * get(int index);
	Output * add(String name, int pin, int onState, int offState);
	void setup(String configDir);
	void loop();
};


extern Outputs outs;
