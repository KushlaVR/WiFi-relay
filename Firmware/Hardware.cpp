#include "Hardware.h"
#include "Output.h"
#include "Sensor.h"

OneWire * wire;


int Hardware::pinLevel(String value)
{
	if (value == "1") return HIGH;
	return LOW;
}

int Hardware::pinNumber(String pinName)
{
	Serial.printf("parse pin number %s\n", pinName.c_str());

	if (pinName == "A0") return A0;
	if (pinName == "D0") return D0;
	if (pinName == "D1") return D1;
	if (pinName == "D2") return D2;
	if (pinName == "D3") return D3;
	if (pinName == "D4") return D4;
	if (pinName == "D5") return D5;
	if (pinName == "D6") return D6;
	if (pinName == "D7") return D7;
	if (pinName == "D8") return D8;
	if (pinName == "LED") return LED_BUILTIN;
	if (pinName == "LED_BUILTIN") return LED_BUILTIN;
	return LED_BUILTIN;
}

void Hardware::begin()
{
	if (SPIFFS.exists("/cfg/hardware.json")) {
		Serial.println("Hardware loading...");
		File f = SPIFFS.open("/cfg/hardware.json", "r");
		JsonString s = JsonString(f.readString());

		int outStart = s.getValuePos("outputs", 0);
		while (outStart >= 0) {
			Output * out = outs.add(s.getValue("name", outStart), pinNumber(s.getValue("pin", outStart)), pinLevel(s.getValue("on", outStart)), pinLevel(s.getValue("off", outStart)));
			outStart = s.indexOf("}", outStart);
			outStart = s.indexOf("name", outStart);
		}

		int sensorStart = s.getValuePos("sensors", 0);
		while (sensorStart >= 0) {
			String type = s.getValue("type", sensorStart);
			String pin = s.getValue("pin", sensorStart);

			if (type == "DHT22") {
				sensors.add(new DHT_22(pinNumber(pin), DHT22));
			}
			else if (type == "DHT21") {
				sensors.add(new DHT_22(pinNumber(pin), DHT21));
			}
			else if (type == "DHT11") {
				sensors.add(new DHT_22(pinNumber(pin), DHT11));
			}
			else if (type == "DS18X20") {
				wire = new OneWire(pinNumber(pin));
				DS18X20::findAll(wire);
			}
			sensorStart = s.indexOf("}", sensorStart);
			sensorStart = s.indexOf("type", sensorStart);
		}

	}
}
