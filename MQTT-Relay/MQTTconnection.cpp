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

		File f = SPIFFS.open("/mqtt.json", "r");
		f.seek(0);
		JsonString json = JsonString(f.readString());
		
		String b = json.getValue("broker");
		broker = (char *)calloc(b.length() + 1, 1);
		b.toCharArray(broker, b.length()+1);
		if (b.length()>0) {
			String p = json.getValue("port");
			port = p.toInt();
			
			String u = json.getValue("user");
			user = (char *)calloc(u.length() + 1, 1);
			u.toCharArray(user, u.length()+1);

			String k = json.getValue("key");
			key = (char *)calloc(k.length()+1, 1);
			k.toCharArray(key, k.length());

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



void MQTTconnection::Register(MQTTprocess * device) {

	Serial.print("Register: ");
	Serial.println(device->name);

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
	device->Register(connection);
	Serial.println("OK;");

}
