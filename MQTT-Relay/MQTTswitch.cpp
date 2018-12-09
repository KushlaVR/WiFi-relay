#include "MQTTswitch.h"



MQTTswitch::MQTTswitch(String feed, String name, uint8_t pin)
{
	this->name = name;
	this->type = "switch";
	this->pin = pin;
	pinMode(pin, OUTPUT);
	String s = "/switches/" + String(feed) + "/" + String(name);
	feed_sate = (char *)calloc(s.length() + 1, 1);
	strncpy(feed_sate, s.c_str(), s.length());

	s = "/switches/" + String(feed) + "/" + String(name) + "/set";
	feed_set = (char *)calloc(s.length() + 1, 1);
	strncpy(feed_set, s.c_str(), s.length());

	s = "/switches/" + String(feed) + "/" + String(name) + "/available";
	feed_available = (char *)calloc(s.length() + 1, 1);
	strncpy(feed_available, s.c_str(), s.length());

	Serial.printf("feed_sate=%s\n", feed_sate);
	Serial.printf("feed_set=%s\n", feed_set);
	Serial.printf("feed_available=%s\n", feed_available);

}

MQTTswitch::MQTTswitch(char * feed, char * name, uint8_t pin) :MQTTswitch(String(feed), String(name), pin)
{
}


MQTTswitch::~MQTTswitch()
{
}

bool MQTTswitch::process(Adafruit_MQTT_Subscribe * subscription)
{
	if (subscription == onoffbutton_set) {
		String value = String((char *)onoffbutton_set->lastread);
		Serial.print(F("Got: "));
		Serial.println(value);
		if (value == "ON") {
			digitalWrite(pin, LOW);
			publish_state("ON");
		}
		else {
			digitalWrite(pin, HIGH);
			publish_state("OFF");
		}
		return true;
	}


	return false;
}

void MQTTswitch::Register(Adafruit_MQTT_Client * connection)
{
	onoffbutton_state = new Adafruit_MQTT_Publish(connection, (const char *)feed_sate);
	onoffbutton_set = new Adafruit_MQTT_Subscribe(connection, (const char *)feed_set);
	onoffbutton_available = new Adafruit_MQTT_Publish(connection, (const char *)feed_available);
	// Setup MQTT subscription for onoff feed.
	connection->subscribe(onoffbutton_set);

}

void MQTTswitch::printInfo(JsonString * ret)
{
	MQTTprocess::printInfo(ret);
	ret->AddValue("state", state);
}

bool MQTTswitch::schedule()
{
	ulong m = millis();
	if ((m - lastReport) > 5000UL) {
		if (publish_available()) lastReport = m;
	}
}



bool MQTTswitch::publish_available() {
	// Now we can publish stuff!
	if (!onoffbutton_available->publish("online")) {
		Serial.println(F("available - Failed"));
	}
	else {
		Serial.println(F("available - send!"));
		return true;
	}
	return false;
}


bool MQTTswitch::publish_state(const char * state) {
	this->state = String(state);
	// Now we can publish stuff!
	if (!onoffbutton_state->publish(state)) {
		Serial.println(F("state - Failed"));
		return true;
	}
	else {
		Serial.println(F("state - send!"));
	}
	return false;
}
