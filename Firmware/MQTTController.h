#pragma once
#include <FS.h>
#include <ESP8266WiFi.h>
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"
#include "MQTTProcess.h"
#include "Json.h"

class MQTTController : public Collection
{

	char * configFile = "/cfg/mqtt.json";

	char * broker;
	uint16_t port;
	char * user;
	char * key;

	WiFiClient client;
	Adafruit_MQTT_Client * connection;// (&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_KEY);


	int numberOfTry = 0;
	bool serverOnline = false;
	ulong lastConnect;
	ulong connectInterval;

public:
	MQTTController();
	~MQTTController();




	void setup();

	bool Available();

	void loop();

	MQTTProcess * Register(MQTTProcess * device);

	bool isServerOnline() { return serverOnline; }

	void printConfig(JsonString * json);
	void saveConfig(String broker, String port, String user, String key);
	void loadConfig();
};

extern MQTTController mqttController;
