#pragma once

#include "TimeZone.h"
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <FS.h>
#include "Json.h"
#include "Output.h"
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"
#include "MQTTController.h"
#include "MQTTProcess.h"
#include "MQTTswitch.h"
#include "WebUIController.h"
#include <vector>

class SessionClass {

public:
	String client;
	String user;
	String session;
};


class ApiController
{
	static SessionClass * session;
public:
	ApiController();
	~ApiController();

	void setup();

	static void handleRestart();
	static void handleMQTT();
	static void handleMQTTSave();
	static void handleAuth();
	static void handleLogout();
	static bool isAuthorized();
	static void handleWifi();
	static void handleWifiSave();
	static void handleStartup();
	static void handleTemplate();
	static void handleMenu();
	static void handleSetup();
	static bool handleGetTriggersForSwitch();
	static bool handleUpdateTrigger(String type);
	static bool handleDeleteTrigger();

	static void handleGetSwitches();
	static void handleSetSwitches();
};

extern ApiController apiController;