#pragma once
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <FS.h>
#include "Json.h"
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"
#include "MQTTconnection.h"
#include "MQTTprocess.h"
#include "MQTTswitch.h"
#include "WebPortal.h"

class ApiController
{

public:
	ApiController();
	~ApiController();

	void setup();

	static MQTTconnection * connection;
	static void handleMQTT();
	static void handleMQTTSave();
	static void handleWifi();
	static void handleWifiSave();
	static void handleStartup();
	static void handleTemplate();
	static void handleSetup();
	static void handleMenu();
	static bool handleGetSwitchSetup();
	static bool handleGetDeleteTrigger();
	static bool handleSetTrigger(String type);

	static void handleGetSwitches();
	static void handleSetSwitches();
};

