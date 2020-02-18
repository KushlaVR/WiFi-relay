#pragma once
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266SSDP.h>
#include <DNSServer.h>
#include <ESP8266mDNS.h>
#include <FS.h>
#include "Json.h"
#include "TimeZone.h"
#include "WiFiController.h"

class UpdateController
{
public:
	UpdateController();
	~UpdateController();
	void setup();
	void loop();

	uint16_t httpsPort = 443;
	String defaultHost = "raw.githubusercontent.com";
	String defaultURL = "/KushlaVR/WiFi-relay/master/MQTT-Relay/data";
	String host = "raw.githubusercontent.com";

	//const char * fingerprint = "CC AA 48 48 66 46 0E 91 53 2C 9C 7C 23 2A B1 74 4D 29 9D 33";//TOD: read from settings file (can see in browser certificate info)


	static void handleUpdate();
	static void handleUpgrade();
	static void updateFiles(String url);
	static void updateFile(String url, String file);
	static void loadURLtoFile(BearSSL::WiFiClientSecure *client, const char *host, const uint16_t port, const char *path, String toFile);


};

extern UpdateController updateController;

