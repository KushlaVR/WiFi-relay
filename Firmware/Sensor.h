#pragma once
#include "Json.h"
#include "DHT_Async.h"
#include <OneWire.h>
#include "Variable.h"
#include "Collection.h"
#include "Button.h"

#define MAX_BUFFER_LEN 3


class Sensor : public Item
{
public:
	Sensor();
	~Sensor();

	String type = "Sensor";
	unsigned long lastRead = 0;
	unsigned long interval = 60000;

	virtual void loop();

};

class DHT_22 : public Sensor {


	float temp[MAX_BUFFER_LEN];
	float hum[MAX_BUFFER_LEN];
	int bufferLength = 0;
	int bufferIndex = 0;

	AsyncDHT * dht = nullptr;


	float termoCompesation = 1;// 0.8461;
	float termoTranslation = 0;

public:
	DHT_22(uint8_t pin, uint8_t type);
	~DHT_22() {};

	float t = 0;
	float h = 0;

	virtual void loop();
};


class ButtonSensor : public Sensor {

	Button * btn = nullptr;

public:
	ButtonSensor(uint8_t pin);
	ButtonSensor(uint8_t pin, int pressedState);
	virtual void loop();
	void pressed();
};


class DS18X20 : public Sensor {
private:
	OneWire * wire;

public:
	DS18X20(OneWire * wire);
	~DS18X20() {};

	float celsius = 0;

	byte addr[8];
	byte data[12];
	int stage = 0;
	String type;
	bool type_s;
	ulong Start_ms = 0;
	virtual void loop();

	static bool findAll(OneWire * wire);
};

class Sensors : public Collection {
public:
	String configDir;
	Sensors() {};
	~Sensors() {};
	Sensor * get(int index);
	//Output * add(String name, int pin, int onState, int offState);
	void setup(String configDir);
	virtual void loop();
};

extern Sensors sensors;
