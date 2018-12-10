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

MQTTconnection mqtt_connection = MQTTconnection();
/****************************** Feeds ***************************************/
// Setup a feed called 'photocell' for publishing.
// Notice MQTT paths for AIO follow the form: <username>/feeds/<feedname>
Adafruit_MQTT_Publish * photocell;// = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/photocell");

void handleGetSwitches();
void handleSetSwitches();

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

	server.on("/api/switches", HTTPMethod::HTTP_GET, handleGetSwitches);
	server.on("/api/switches", HTTPMethod::HTTP_POST, handleSetSwitches);

}


void handleGetSwitches() {
	Serial.println("switches GET:");

	server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
	server.sendHeader("Pragma", "no-cache");
	server.sendHeader("Expires", "-1");
	server.setContentLength(CONTENT_LENGTH_UNKNOWN);
	JsonString ret = JsonString();
	ret.beginObject();
	MQTTprocess * proc = mqtt_connection.getFirstProcess();
	ret.beginArray("items");
	while (proc != nullptr) {
		ret.beginObject();
		proc->printInfo(&ret);
		ret.endObject();
		proc = proc->next;
	};
	ret.endArray();
	ret.endObject();
	server.send(200, "application/json", ret);
}

void handleSetSwitches() {
	Serial.println("switches GET:");
	if (server.hasArg("index") && server.hasArg("state")) {
		
		int index = server.arg("index").toInt();

		MQTTprocess * proc = mqtt_connection.getFirstProcess();
		int i = 1;
		while (proc != nullptr) {
			if (i == index) break;
			proc = proc->next;
			i++;
		};

		if (proc == nullptr) {
			WebPortal::handleNotFound();
			return;
		}

		if (proc->type != "switch") {
			WebPortal::handleNotFound();
			return;
		}
		MQTTswitch * sw = (MQTTswitch *)proc;
		sw->setState(server.arg("state")=="on");
		server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
		server.sendHeader("Pragma", "no-cache");
		server.sendHeader("Expires", "-1");
		server.setContentLength(CONTENT_LENGTH_UNKNOWN);
		JsonString ret = JsonString();
		ret.beginObject();
		ret.AddValue("status", "OK");
		ret.endObject();
		server.send(200, "application/json", ret);
		return;
	}
	WebPortal::handleNotFound();
}


uint32_t x = 0;
unsigned long lastInfo = 0;

void loop() {
	server.loop();
	if (mqtt_connection.loop()) {
		mqtt_connection.process();

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


