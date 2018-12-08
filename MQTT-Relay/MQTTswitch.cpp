#include "MQTTswitch.h"



MQTTswitch::MQTTswitch(char * name)
{
	this->name = name;

	String s = "/feeds/" + String(name);
	feed_sate = (char *)calloc(s.length() + 1, 1);
	strncpy(feed_sate, s.c_str(), s.length());

	s = "/feeds/" + String(name) + "/set";
	feed_set = (char *)calloc(s.length() + 1, 1);
	strncpy(feed_set, s.c_str(), s.length());

	s = "/feeds/" + String(name) + "/available";
	feed_available = (char *)calloc(s.length() + 1, 1);
	strncpy(feed_available, s.c_str(), s.length());


	Serial.printf("feed_sate=%s\n", feed_sate);
	Serial.printf("feed_set=%s\n", feed_set);
	Serial.printf("feed_available=%s\n", feed_available);

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
			digitalWrite(BUILTIN_LED, LOW);
			publish_state("ON");
		}
		else {
			digitalWrite(BUILTIN_LED, HIGH);
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
