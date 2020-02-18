#pragma once
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"
#include "Json.h"
#include "Collection.h"


class MQTTProcess : public Item
{
public:
	MQTTProcess();
	~MQTTProcess();

	String name;
	String type = "process";

	int getSort() { return Index + 1; };

	virtual void printInfo(JsonString * ret);
	virtual void Register(Adafruit_MQTT_Client * connection) {};
	virtual bool loop(Adafruit_MQTT_Subscribe * subscribtion) { return false; };
	virtual void schedule() {};

};

