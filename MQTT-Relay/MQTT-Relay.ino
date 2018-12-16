#include <ESP8266HTTPClient.h>
#include <ESP8266httpUpdate.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266SSDP.h>
#include <DNSServer.h>
#include <ESP8266mDNS.h>
#include <EEPROM.h>
#include <FS.h>
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"
#include "MQTTconnection.h"
#include "MQTTprocess.h"
#include "MQTTswitch.h"
#include "WebPortal.h"
#include "Json.h"
#include "NTPreciver.h"
#include "ApiController.h"
#include <TimeLib.h>

/****************************** Feeds ***************************************/
// Setup a feed called 'photocell' for publishing.
// Notice MQTT paths for AIO follow the form: <username>/feeds/<feedname>
Adafruit_MQTT_Publish * photocell;// = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/photocell");



NTPreciver NTP;
ApiController api;

void setup() {
	Serial.begin(115200);
	pinMode(BUILTIN_LED, OUTPUT);
	digitalWrite(BUILTIN_LED, HIGH);
	if (!SPIFFS.begin()) {
		Serial.println(F("No file system!"));
		Serial.println(F("Fomating..."));
		if (SPIFFS.format())
			Serial.println(F("OK"));
		else {
			Serial.println(F("Fail.... rebooting..."));
			while (true);
		}
	}
	else {
		Serial.println(F("File system OK!"));
	}
	Serial.println(F("KushlaVR@gmail.com \nMQTT switch\n\n"));

	server.setup();
	mqtt_connection.setup();

	mqtt_connection.Register(new MQTTswitch(String(server.myHostname), "out1", D5));
	mqtt_connection.Register(new MQTTswitch(String(server.myHostname), "out2", D6));
	mqtt_connection.Register(new MQTTswitch(String(server.myHostname), "out3", D7));
	mqtt_connection.Register(new MQTTswitch(String(server.myHostname), "led", LED_BUILTIN));

	photocell = new Adafruit_MQTT_Publish(mqtt_connection.connection, "/feeds/photocell");

	api.setup();
	NTP.setup();

}


uint32_t x = 0;
unsigned long lastInfo = 0;

void loop() {
	server.loop();
	NTP.loop();
	if (mqtt_connection.loop()) {
		mqtt_connection.process();

		if ((millis() - lastInfo) > 5000) {
			lastInfo = millis();
			// Now we can publish stuff!


			time_t t = now();
			String s;
			if (timeStatus() == timeSet)
				s = String(year(t)) + "." + String(month(t)) + "." + String(day(t)) + " " +
				String(hour(t)) + ":" + String(minute(t)) + ":" + String(second(t));
			else
				s = "time not set. millis=" + String(millis());

			Serial.print(F("\nSending photocell val "));
			Serial.print(s);
			Serial.print("...");
			if (!photocell->publish(s.c_str())) {
				Serial.println(F("Failed"));
			}
			else {
				Serial.println(F("OK!"));
			}
		}


		// ping the server to keep the mqtt connection alive
		// NOT required if you are publishing once every KEEPALIVE seconds
		/*
		if(! mqtt.ping()) {
		mqtt.disconnect();
		}
		*/
	}
}
