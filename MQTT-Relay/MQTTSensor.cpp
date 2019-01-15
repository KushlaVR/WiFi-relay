
#include "MQTTSensor.h"


MQTTSensor::MQTTSensor(String feed, String name, String variable)
{
	this->name = name;
	this->type = "sensor";
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

bool MQTTSensor::schedule()
{
	float d = Variable::getValue(variable);
	if (isnan(d)) return false;
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

	return false;
}

void MQTTSensor::printInfo(JsonString * ret)
{
	MQTTprocess::printInfo(ret);
	ret->AddValue("visual", "tsens");
	float d = Variable::getValue(variable);
	ret->AddValue("value", String(d));
}

bool MQTTSensor::publish_value(double d)
{
	if (mqtt_connection.serverOnline) {
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
