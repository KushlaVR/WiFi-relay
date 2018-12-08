#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"
#include "MQTTconnection.h"



MQTTconnection::MQTTconnection()
{
}


MQTTconnection::~MQTTconnection()
{
}


void MQTTconnection::setup() {
	if (SPIFFS.exists("/mqtt.json")) {
		Serial.println(F("read ./mqtt.json"));

		DynamicJsonBuffer  jsonBuffer(200);
		File f = SPIFFS.open("/mqtt.json", "r");
		f.seek(0);
		JsonObject& root = jsonBuffer.parseObject(f.readString());
		if (root.containsKey("broker")) {
			const char * b = root["broker"];
			//Serial.println(strlen(b));
			broker = (char *)calloc(1, strlen(b) + 1);
			strncpy(broker, b, strlen(b));

			const char * p = root["port"];
			port = String(p).toInt();

			const char * u = root["user"];
			user = (char *)calloc(1, strlen(u) + 1);
			if (strlen(u)) strncpy(user, u, strlen(u));

			const char * k = root["key"];
			key = (char *)calloc(1, strlen(k) + 1);
			if (strlen(k)) strncpy(key, k, strlen(k));

			Serial.print("broker:"); Serial.println(broker);
			Serial.print("port:"); Serial.println(port);
			Serial.print("user:"); Serial.println(user);
			Serial.print("key:"); Serial.println(key);

			if (strlen(user) == 0) {
				Serial.println("create mqtt");
				connection = new Adafruit_MQTT_Client(&client, (const char *)broker, port);
			}
			else
			{
				connection = new Adafruit_MQTT_Client(&client, (const char *)broker, port, (const char *)user, (const char *)key);
			}
			Serial.println("OK!");
		}
		f.close();
		lastConnect = millis();
		connectInterval = 0UL;
	}
}

bool MQTTconnection::loop() {
	if (WiFi.status() != WL_CONNECTED) return false;
	if ((millis() - lastConnect) > connectInterval) {
		// Function to connect and reconnect as necessary to the MQTT server.
		// Should be called in the loop function and it will take care if connecting.
		if (connection == nullptr) return false;
		int8_t ret;
		// Stop if already connected.
		if (connection->connected()) return true;
		Serial.print("Connecting to MQTT... ");
		if ((ret = connection->connect()) != 0) { // connect will return 0 for connected
			Serial.print("Can't connecto to MQTT broker:");
			Serial.println(connection->connectErrorString(ret));
			Serial.println("Next try in 5 seconds...");
			connection->disconnect();
			connectInterval = 5000UL;
			return false;
		}
		Serial.println("MQTT Connected!");
		connectInterval = 0UL;
		lastConnect = millis();
		return true;
	}
	return false;
}

void MQTTconnection::process()
{
	MQTTprocess * dev;
	//process subscritions
	Adafruit_MQTT_Subscribe *subscription;
	while ((subscription = connection->readSubscription())) {
		dev = firstProcess;
		while (dev != nullptr) {
			dev->process(subscription);
			dev = dev->next;
		}
	}
	//process scheduled tasks
	dev = firstProcess;
	while (dev != nullptr) {
		dev->schedule();
		dev = dev->next;
	}
}

Adafruit_MQTT_Subscribe * MQTTconnection::readSubscription(int16_t timeout) {
	return connection->readSubscription(timeout);
}


void MQTTconnection::Register(MQTTprocess * device) {
	if (firstProcess == nullptr) {
		firstProcess = device;
		device->next = nullptr;
	}
	else {
		MQTTprocess * dev = firstProcess;
		while (dev != nullptr) {
			if (dev->next == nullptr) {
				dev->next = device;
				device->next = nullptr;
				dev = nullptr;
			}
			else
				dev = dev->next;
		}
	}
	Serial.print("registered");
	device->Register(connection);

}
