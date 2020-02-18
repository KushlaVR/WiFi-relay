/*
 Name:		Firmware_V1.ino
 Created:	4/11/2019 10:45:30 PM
 Author:	Віталік
*/
#include "definitions.h"


NTPreciver NTP;
Hardware * hardware;


// the setup function runs once when you press reset or power the board
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


	/*1. Reading configs
	//
	//    In SPIFFS we cannot edit file. To edit file we nead to rewrite all data.
	//    On low level it is equel to create new file in new flash address and mark old file as deleted...
	//    So it is better to store configuration in separate files
	//
	//    There is also allowed "/" sign in name
	//    So we can create pseudo folders in data catalog
	//
	//    Maximum filename len = 32 chars (include first "/" char and \0 char at then end of file name)
	//
	//    SPIFFS allow to append files without relocating flash memory, so we can also make a logging system.
	//
	//    /cfg/wifi.json
	//    /cfg/mqtt.json
	//    /cfg/out1/start.json
	//    /cfg/out2/start.json
	//    /cfg/out3/start.json
	//    /cfg/led/start.json
	//    /log/year/month/day/out1.txt
	//    /log/year/month/day/out2.txt
	//    /log/year/month/day/out3.txt
	*/

	hardware = new Hardware();
	hardware->begin();

	//TODO: read outputs from config...
	//Output * out1 = outs.add("out1", D5, HIGH, LOW);
	//Output * out2 = outs.add("out2", D6, HIGH, LOW);
	//Output * out3 = outs.add("out3", D7, HIGH, LOW);
	//Output * led = outs.add("led", BUILTIN_LED, LOW, HIGH);

	//sensors.add(new DHT_22(D2, DHT22));
	//DS18X20::findAll(wire);

	outs.setup("/cfg/");

	NTP.setup("/cfg/ntp.json");

	/*
	Wifi connection:
	  When wifi.json is not found start AP with default config
	  else
		if known network is available - connect
	  if network is not available more then 2 minutes
		try to connect to next known network.
		if fail - strt AP

	*/
	wifiController.setup("/cfg/wifi.json");

	/*
	MQTT
	*/
	mqttController.setup();

	Output * out = (Output *)outs.getFirst();
	while (out != nullptr) {
		mqttController.Register(new MQTTSwitch(String(wifiController.getAPName()), out));
		Trigger::loadConfig(out);
		out = (Output *)out->next;
	}

	Sensor * sns = (Sensor *)sensors.getFirst();
	while (sns != nullptr) {
		if (sns->type == "dht") {
			mqttController.Register(new MQTTSensor(String(wifiController.getAPName()), "temperature" + String(sns->Index), "t" + String(sns->Index), "tsens"));
			mqttController.Register(new MQTTSensor(String(wifiController.getAPName()), "humidity" + String(sns->Index), "h" + String(sns->Index), "hsens"));
		}
		else if (sns->type == "ds18x20") {
			mqttController.Register(new MQTTSensor(String(wifiController.getAPName()), "temperature" + String(sns->Index), "t" + String(sns->Index), "tsens"));
		}
		sns = (Sensor *)sns->next;
	}

	//mqttController.Register(new MQTTSwitch(String(wifiController.getAPName()), out1));
	//mqttController.Register(new MQTTSwitch(String(wifiController.getAPName()), out2));
	//mqttController.Register(new MQTTSwitch(String(wifiController.getAPName()), out3));
	//mqttController.Register(new MQTTSwitch(String(wifiController.getAPName()), led));
	//mqttController.Register(new MQTTSensor(String(wifiController.getAPName()), "temperature", "t1", "tsens"));
	//mqttController.Register(new MQTTSensor(String(wifiController.getAPName()), "humidity", "h1", "hsens"));

	//Trigger::loadConfig(out1);
	//Trigger::loadConfig(out2);
	//Trigger::loadConfig(out3);
	//Trigger::loadConfig(led);

	Trigger::Sort();

	webServer.setup(NTP.timeZone);
	apiController.setup();
}

// the loop function runs over and over again until power down or reset
void loop() {
	if (wifiController.loop()) {
		webServer.loop();
		//work Online
		if (mqttController.Available())
			mqttController.loop();
		NTP.loop();
	}
	else {
		webServer.loop();
	}
	outs.loop();
	sensors.loop();
	time_t t = NTP.timeZone->toLocal(now());
	Trigger::processNext(&t);
}
