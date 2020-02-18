#include "MQTTSwitch.h"



bool MQTTSwitch::publish_available()
{
	if (getController()->isServerOnline()) {
		// Now we can publish stuff!
		if (!onoffbutton_available->publish("online")) {
			Serial.println(F("available - Failed"));
		}
		else {
			Serial.println(F("available - send!"));
			return true;
		}
	}
	return false;
}

bool MQTTSwitch::publish_state(const char * state)
{
	if (getController()->isServerOnline()) {
		// Now we can publish stuff!
		if (!onoffbutton_state->publish(state)) {
			Serial.printf("Publish %s state - Failse!\n", state);
			return true;
		}
		else {
			Serial.printf("Publised %s state - send!\n", state);
		}
	}
	return false;
}


MQTTSwitch::MQTTSwitch(String feed, Output * out)
{
	this->out = out;
	this->name = out->getName();
	this->type = "switch";
	this->publishedState = out->isOn();
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

MQTTSwitch::~MQTTSwitch()
{
}

void MQTTSwitch::Register(Adafruit_MQTT_Client * connection)
{
	onoffbutton_state = new Adafruit_MQTT_Publish(connection, (const char *)feed_sate);
	onoffbutton_set = new Adafruit_MQTT_Subscribe(connection, (const char *)feed_set);
	onoffbutton_available = new Adafruit_MQTT_Publish(connection, (const char *)feed_available);
	// Setup MQTT subscription for onoff feed.
	connection->subscribe(onoffbutton_set);
}

void MQTTSwitch::printInfo(JsonString * ret)
{
	MQTTProcess::printInfo(ret);
	ret->AddValue("state", (out->isOn() ? "ON" : "OFF"));
	ret->AddValue("index", String(getSort()));
	ret->AddValue("visual", "switch");
}

bool MQTTSwitch::loop(Adafruit_MQTT_Subscribe * subscribtion)
{
	if (subscribtion == onoffbutton_set) {
		String value = String((char *)onoffbutton_set->lastread);
		Serial.print(F("Got: "));
		Serial.println(value);
		setState(value == "ON");
		return true;
	}
	return false;
}

void MQTTSwitch::schedule()
{
	//Report that switch is online to MQTT server
	ulong m = millis();
	if (lastReport == 0 || (m - lastReport) > repotPeriod) {
		if (publish_available()) lastReport = m;
	}
	if (publishedState != out->isOn()) {
		setState(out->isOn());
	}
}

void MQTTSwitch::setState(bool newState)
{
	if (publishedState == newState) return;
	out->setState(newState);
	publishedState = newState;
	if (newState)
		publish_state("ON");
	else
		publish_state("OFF");
}
