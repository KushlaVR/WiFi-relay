#pragma once
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"
#include "Json.h"
#include "MQTTProcess.h"
#include "MQTTController.h"
#include "Output.h"

class MQTTSwitch :public MQTTProcess
{
	ulong lastReport = 0;
	ulong repotPeriod = 300000UL;
	char * feed_sate;
	char * feed_set;
	char * feed_available;

	bool publishedState = false;

	Adafruit_MQTT_Publish * onoffbutton_state;
	Adafruit_MQTT_Subscribe * onoffbutton_set;
	Adafruit_MQTT_Publish * onoffbutton_available;


	bool publish_available();
	bool publish_state(const char * state);

public:
	//String variable;
	Output * out;

	MQTTSwitch(String feed, Output * out);
	~MQTTSwitch();

	
	MQTTController * getController() { return (MQTTController *)__collection; };
	void Register(Adafruit_MQTT_Client * connection);
	void printInfo(JsonString * ret);
	bool loop(Adafruit_MQTT_Subscribe * subscribtion);
	void schedule();
	void setState(bool newState);

};

