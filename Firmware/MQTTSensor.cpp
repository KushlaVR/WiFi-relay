#include "MQTTSensor.h"




MQTTSensor::MQTTSensor(String feed, String name, String variable, String visual)
{
	this->name = name;
	this->type = "sensor";
	this->visual = visual;
	this->variable = variable;
	String s = "/sensors/" + String(feed) + "/" + String(name);
	feed_sate = (char *)calloc(s.length() + 1, 1);
	strncpy(feed_sate, s.c_str(), s.length());

	Serial.printf("feed_sate=%s\n", feed_sate);
}

MQTTSensor::~MQTTSensor()
{
}

void MQTTSensor::Register(Adafruit_MQTT_Client * connection)
{
	sensor_state = new Adafruit_MQTT_Publish(connection, (const char *)feed_sate);
}

void MQTTSensor::schedule()
{
	float d = Variable::getValue(variable);
	if (isnan(d)) return;
	ulong m = millis();
	if ((lastReport == 0) || ((m - lastReport) > repotPeriod) || (d != pulished)) {
		//Serial.printf("d=%f\n", d);
		//Serial.printf("published=%f\n", pulished);
		double f = d;
		if (publish_value(f)) {
			lastReport = m;
			pulished = d;
		};
	}

	return;
}

void MQTTSensor::printInfo(JsonString * ret)
{
	MQTTProcess::printInfo(ret);
	ret->AddValue("visual", visual);
	float d = Variable::getValue(variable);
	ret->AddValue("value", String(d));
}

bool MQTTSensor::publish_value(double d)
{
	if (getController()->isServerOnline()) {
		// Now we can publish stuff!
		if (!sensor_state->publish(d)) {
			Serial.println(F("publish_value - Failed"));
		}
		else {
			Serial.printf("publish_value %f - send!\n", d);
			return true;
		}
	}
	return false;
}
