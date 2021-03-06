#pragma once

#include <Time.h>
#include <OneWire.h>
#include "DHT_Async.h"
#include "Variable.h"

class Sensor
{
private:
	Sensor * next;
public:


	Sensor();
	~Sensor();

	int uid = 0;
	char * type = nullptr;
	bool type_s = false;
	String variable;
	unsigned long lastLoop = 0;
	unsigned long interval = 5000UL;

	void Register();
	virtual bool loop(unsigned long m);

	static void processNext();
	static int generateNewUid();
};


class DS18X20 : public Sensor {


public:
	DS18X20();
	~DS18X20() {};

	float celsius = 0;

	byte addr[8];
	byte data[12];
	int stage = 0;

	bool loop(unsigned long m);

	static bool findAll();
};

class DHT_22 : public Sensor {

	float termoCompesation = 1;// 0.8461;
	float termoTranslation = 0;

public:
	DHT_22() {};
	~DHT_22() {};

	float t = 0;
	float h = 0;

	bool loop(unsigned long m);
	static bool findAll();

};