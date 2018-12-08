#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266SSDP.h>
#include <DNSServer.h>
#include <ESP8266mDNS.h>
#include <EEPROM.h>
#include <FS.h>
#include <ArduinoJson.h>
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"
//#include "MQTTdevice.h"
#include "MQTTconnection.h"
#include "MQTTprocess.h"
#include "MQTTswitch.h"
#include "WebPortal.h"

MQTTconnection mqtt_connection = MQTTconnection();
/****************************** Feeds ***************************************/

// Setup a feed called 'photocell' for publishing.
// Notice MQTT paths for AIO follow the form: <username>/feeds/<feedname>
Adafruit_MQTT_Publish * photocell;// = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/photocell");

MQTTswitch * ledSwitch;


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

	ledSwitch = new MQTTswitch("onoff");
	mqtt_connection.Register(ledSwitch);

	photocell = new Adafruit_MQTT_Publish(mqtt_connection.connection, "/feeds/photocell");

}

uint32_t x = 0;
unsigned long lastInfo = 0;

void loop() {
	server.loop();
	if (mqtt_connection.loop()) {
		mqtt_connection.process();

		/*Adafruit_MQTT_Subscribe *subscription;
		while ((subscription = mqtt_connection.readSubscription())) {
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
			}
		}*/
		if ((millis() - lastInfo) > 5000) {
			lastInfo = millis();
			// Now we can publish stuff!
			Serial.print(F("\nSending photocell val "));
			Serial.print(x);
			Serial.print("...");
			if (!photocell->publish(x++)) {
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
//
//void publish(Adafruit_MQTT_Publish * topic, const char * state) {
//	// Now we can publish stuff!
//	if (!topic->publish(state)) {
//		Serial.println(F("Failed"));
//	}
//	else {
//		Serial.println(F("available - send!"));
//	}
//}





