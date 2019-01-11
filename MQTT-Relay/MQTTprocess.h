#pragma once
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"
#include "Json.h"


class MQTTprocess
{
public:
	MQTTprocess();
	~MQTTprocess();

	String name;
	String type = "process";
	String variable;

	virtual bool process(Adafruit_MQTT_Subscribe * subscription) {};
	virtual bool schedule() {};
	virtual void Register(Adafruit_MQTT_Client * connection) {};
	virtual void printInfo(JsonString * ret);
	MQTTprocess * next;
};

