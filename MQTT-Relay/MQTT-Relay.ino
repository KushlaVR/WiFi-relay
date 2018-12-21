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
#include "Trigger.h"

/****************************** Feeds ***************************************/
// Setup a feed called 'photocell' for publishing.
// Notice MQTT paths for AIO follow the form: <username>/feeds/<feedname>
Adafruit_MQTT_Publish * photocell;// = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/photocell");



NTPreciver NTP;
ApiController api;

void setup() {
	setTime(18, 0, 0, 17, 12, 2018);
	setSyncInterval(1);

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

	MQTTswitch * out1 = (MQTTswitch *)mqtt_connection.Register(new MQTTswitch(String(server.myHostname), "out1", D5));
	MQTTswitch * out2 = (MQTTswitch *)mqtt_connection.Register(new MQTTswitch(String(server.myHostname), "out2", D6));
	MQTTswitch * out3 = (MQTTswitch *)mqtt_connection.Register(new MQTTswitch(String(server.myHostname), "out3", D7));
	MQTTswitch * led = (MQTTswitch *)mqtt_connection.Register(new MQTTswitch(String(server.myHostname), "led", LED_BUILTIN));
	
	led->onPinValue = LOW;
	led->offPinValue = HIGH;

	Trigger::loadConfig(out1);
	Trigger::loadConfig(out2);
	Trigger::loadConfig(out3);
	Trigger::loadConfig(led);
	/*
	OnOffTrigger * t;

	t = new OnOffTrigger();
	t->proc = led;
	t->action = HIGH;
	t->time = 18 * 60 + 1;//18:01
	t->Register();

	t = new OnOffTrigger();
	t->proc = led;
	t->action = LOW;
	t->time = 18 * 60 + 2;//18:02
	t->Register();
	/**/

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

		if ((millis() - lastInfo) > 10000) {
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
	time_t t = now();
	Trigger::processNext(&t);
}
