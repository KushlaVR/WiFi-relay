#pragma once
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"


class MQTTprocess
{
public:
	MQTTprocess();
	~MQTTprocess();

	virtual bool process(Adafruit_MQTT_Subscribe * subscription) {};
	virtual bool schedule() {};
	virtual void Register(Adafruit_MQTT_Client * connection) {};
	MQTTprocess * next;
};

