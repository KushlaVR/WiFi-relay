#pragma once
#if defined(ARDUINO) && ARDUINO >= 100
#include "arduino.h"
#else
#include "WProgram.h"
#endif

#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"
#include "MQTTprocess.h"
#include "MQTTconnection.h"
#include "Variable.h"

class MQTTSensor: public MQTTprocess
{
private:
	char * feed_sate;
	float pulished = NAN;
	
	ulong lastReport = 0;
	ulong repotPeriod = 300000UL;
	Adafruit_MQTT_Publish * sensor_state;

public:
	String visual = "tsens";
	MQTTSensor(char * feed, char * name, char * variable, char * visual) :MQTTSensor(String(feed), String(name), String(variable), String(visual)) {};
	MQTTSensor(String feed, String name, String variable, String visual);
	~MQTTSensor();

	void Register(Adafruit_MQTT_Client * connection);
	bool schedule();
	void printInfo(JsonString * ret);


	bool publish_value(double d);
};

