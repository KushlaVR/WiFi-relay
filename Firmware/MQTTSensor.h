#pragma once
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"
#include "Json.h"
#include "MQTTProcess.h"
#include "MQTTController.h"
#include "Variable.h"

class MQTTSensor : public MQTTProcess
{
private:
	String variable;
	String visual;
	char * feed_sate;
	float pulished = NAN;


	ulong lastReport = 0;
	ulong repotPeriod = 300000UL;
	Adafruit_MQTT_Publish * sensor_state;

public:
	
	MQTTSensor(char * feed, char * name, char * variable, char * visual) :MQTTSensor(String(feed), String(name), String(variable), String(visual)) {};
	MQTTSensor(String feed, String name, String variable, String visual);
	~MQTTSensor();

	MQTTController * getController() { return (MQTTController *)__collection; };
	void Register(Adafruit_MQTT_Client * connection);
	void schedule();
	void printInfo(JsonString * ret);


	bool publish_value(double d);
};

