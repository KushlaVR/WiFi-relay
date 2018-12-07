/***************************************************
Adafruit MQTT Library ESP8266 Example

Must use ESP8266 Arduino from:
https://github.com/esp8266/Arduino

Works great with Adafruit's Huzzah ESP board & Feather
----> https://www.adafruit.com/product/2471
----> https://www.adafruit.com/products/2821

Adafruit invests time and resources providing this open source code,
please support Adafruit and open-source hardware by purchasing
products from Adafruit!

Written by Tony DiCola for Adafruit Industries.
MIT license, all text above must be included in any redistribution
****************************************************/
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <DNSServer.h>
#include <ESP8266mDNS.h>
#include <EEPROM.h>
#include <FS.h>
#include <ArduinoJson.h>
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"
#include "WebPortal.h"


/************************* Adafruit.io Setup *********************************/

//#define AIO_SERVER      "192.168.5.100"
//#define AIO_SERVERPORT  1883                   // use 8883 for SSL
//#define AIO_USERNAME    ""//"...your AIO username (see https://accounts.adafruit.com)..."
//#define AIO_KEY         ""//"...your AIO key..."

/************ Global State (you don't need to change this!) ******************/

// Create an ESP8266 WiFiClient class to connect to the MQTT server.
WiFiClient client;
// or... use WiFiFlientSecure for SSL
//WiFiClientSecure client;

// Setup the MQTT client class by passing in the WiFi client and MQTT server and login details.
Adafruit_MQTT_Client * mqtt = nullptr;// (&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_KEY);

/****************************** Feeds ***************************************/

// Setup a feed called 'photocell' for publishing.
// Notice MQTT paths for AIO follow the form: <username>/feeds/<feedname>
Adafruit_MQTT_Publish * photocell;// = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/photocell");

// Setup a feed called 'onoff' for subscribing to changes.
/*
- platform: mqtt
name: "Bedroom Switch"
state_topic: "/feeds/onoff"
command_topic: "/feeds/onoff/set"
availability_topic: "/feeds/onoff/available"
payload_on: "ON"
payload_off: "OFF"
state_on: "ON"
state_off: "OFF"
optimistic: false
qos: 0
retain: true
*/
Adafruit_MQTT_Publish * onoffbutton_state;// = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/onoff");
Adafruit_MQTT_Subscribe * onoffbutton_set;// = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/feeds/onoff/set");
Adafruit_MQTT_Publish * onoffbutton_available;// = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/onoff/available");


char * broker;
const char * port;
const char * user;
const char * key;

/*************************** Sketch Code ************************************/

// Bug workaround for Arduino 1.6.6, it seems to need a function declaration
// for some reason (only affects ESP8266, likely an arduino-builder bug).
void MQTT_connect();
void MQTT_ReadConfig();


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
	MQTT_ReadConfig();

	Serial.print("broke:"); Serial.println(broker);
	Serial.print("user:"); Serial.println(user);
	Serial.print("key:"); Serial.println(key);

	photocell = new Adafruit_MQTT_Publish(mqtt, "/feeds/photocell");
	onoffbutton_state = new Adafruit_MQTT_Publish(mqtt, "/feeds/onoff");
	onoffbutton_set = new Adafruit_MQTT_Subscribe(mqtt, "/feeds/onoff/set");
	onoffbutton_available = new Adafruit_MQTT_Publish(mqtt, "/feeds/onoff/available");
	// Setup MQTT subscription for onoff feed.
	mqtt->subscribe(onoffbutton_set);
}

uint32_t x = 0;
unsigned long lastInfo = 0;

void loop() {
	server.loop();
	if (WiFi.status() == WL_CONNECTED) {
		// Ensure the connection to the MQTT server is alive (this will make the first
		// connection and automatically reconnect when disconnected).  See the MQTT_connect
		// function definition further below.
		MQTT_connect();

		// this is our 'wait for incoming subscription packets' busy subloop
		// try to spend your time here

		Adafruit_MQTT_Subscribe *subscription;
		while ((subscription = mqtt->readSubscription(1))) {
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
		}
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

			publish_available();
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

void publish(Adafruit_MQTT_Publish * topic, const char * state) {
	// Now we can publish stuff!
	if (!topic->publish(state)) {
		Serial.println(F("Failed"));
	}
	else {
		Serial.println(F("available - send!"));
	}
}

void publish_available() {
	// Now we can publish stuff!
	if (!onoffbutton_available->publish("online")) {
		Serial.println(F("available - Failed"));
	}
	else {
		Serial.println(F("available - send!"));
	}
}


void publish_state(const char * state) {
	// Now we can publish stuff!
	if (!onoffbutton_state->publish(state)) {
		Serial.println(F("state - Failed"));
	}
	else {
		Serial.println(F("state - send!"));
	}
}



void MQTT_ReadConfig() {
	if (SPIFFS.exists("/mqtt.json")) {
		Serial.println(F("read ./mqtt.json"));
		
		DynamicJsonBuffer  jsonBuffer(200);
		File f = SPIFFS.open("/mqtt.json", "r");
		f.seek(0);
		JsonObject& root = jsonBuffer.parseObject(f.readString());
		if (root.containsKey("broker")) {
			const char * b = root["broker"];
			Serial.println(strlen(b));
			broker = (char *)calloc(1,strlen(b)+1);
			strncpy(broker, b, strlen(b));
			
			port = root["port"];
			user = root["user"];
			key = root["key"];
			
			uint16_t uPort = String(port).toInt();			
			Serial.print("broker:"); Serial.println(broker);
			Serial.print("port:"); Serial.println(uPort);
			Serial.print("user:"); Serial.println(user);
			Serial.print("key:"); Serial.println(key);

			if (strlen(user) == 0) {
				Serial.println("create mqtt");
				mqtt = new Adafruit_MQTT_Client(&client, (const char *)broker, uPort);
			}
			else
			{
				mqtt = new Adafruit_MQTT_Client(&client, broker, uPort, user, key);
			}
			Serial.println("OK!");
		}
		f.close();
	}
}


// Function to connect and reconnect as necessary to the MQTT server.
// Should be called in the loop function and it will take care if connecting.
void MQTT_connect() {
	if (mqtt == nullptr) return;
	int8_t ret;

	// Stop if already connected.
	if (mqtt->connected()) {
		return;
	}

	Serial.print("Connecting to MQTT... ");

	uint8_t retries = 3;
	while ((ret = mqtt->connect()) != 0) { // connect will return 0 for connected
		Serial.println(mqtt->connectErrorString(ret));
		Serial.println("Retrying MQTT connection in 5 seconds...");
		mqtt->disconnect();
		delay(5000);  // wait 5 seconds
		retries--;
		if (retries == 0) {
			// basically die and wait for WDT to reset me
			while (1);
		}
	}
	Serial.println("MQTT Connected!");
}