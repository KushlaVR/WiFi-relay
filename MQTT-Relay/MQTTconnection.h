#pragma once

#include <FS.h>
#include <ESP8266WiFi.h>
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"
#include "MQTTprocess.h"
#include "Json.h"

class MQTTconnection
{
	ulong lastConnect;
	ulong connectInterval;
	char * broker;
	uint16_t port;
	char * user;
	char * key;
	WiFiClient client;
	MQTTprocess * firstProcess;

public:
	MQTTconnection();
	~MQTTconnection();

	Adafruit_MQTT_Client * connection;// (&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_KEY);
	void setup();
	bool loop();
	void process();

	Adafruit_MQTT_Subscribe * readSubscription(int16_t timeout = (int16_t)0);

	void Register(MQTTprocess * device);
};

