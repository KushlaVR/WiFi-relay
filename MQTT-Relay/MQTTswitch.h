#pragma once
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"
#include "Json.h"
#include "MQTTprocess.h"

class MQTTswitch : public MQTTprocess
{
	// Setup a feed called 'onoff' for subscribing to changes.
	/*
	- platform: mqtt
	name: "Bedroom Switch"
	state_topic: "/feeds/onoff"
	command_topic: "/feeds/onoff/set"
	availability_topic: "/feeds/onoff/available"
	payload_on: "ON"
	payload_off: "OFF"
	state_on: "ON"
	state_off: "OFF"
	optimistic: false
	qos: 0
	retain: true
	*/
	ulong lastReport = 0;

	char * feed_sate;
	char * feed_set;
	char * feed_available;

	Adafruit_MQTT_Publish * onoffbutton_state;
	Adafruit_MQTT_Subscribe * onoffbutton_set;
	Adafruit_MQTT_Publish * onoffbutton_available;

	uint8_t pin;
	String state = "OFF";

	bool publish_available();
	bool publish_state(const char * state);
public:
	MQTTswitch(String feed, String name, uint8_t pin);
	MQTTswitch(char * feed, char * name, uint8_t pin);
	~MQTTswitch();

	bool process(Adafruit_MQTT_Subscribe * subscription);
	void Register(Adafruit_MQTT_Client * connection);
	void printInfo(JsonString * ret);
	bool schedule();
};;

