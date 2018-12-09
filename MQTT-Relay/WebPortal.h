#pragma once
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266SSDP.h>
#include <DNSServer.h>
#include <ESP8266mDNS.h>
#include <FS.h>
#include "Json.h"

/* Set these to your desired softAP credentials. They are not configurable at runtime */
#ifndef APSSID
#define APSSID "KushlaVR"
#define APPSK  "12345678"
#endif

class WebPortal :public ESP8266WebServer
{
private:

	// DNS server
	const byte DNS_PORT = 53;
	DNSServer dnsServer;



	/** Should I connect to WLAN asap? */
	boolean connect;

	/** Last time I tried to connect to WLAN */
	unsigned long lastConnectTry = 0;

	/** Current WLAN status */
	unsigned int status = WL_IDLE_STATUS;

	void connectWifi();

	void sendFile(File file, char* contenttype, bool addGzHeader);
	char * getContentType(String filename);
public:
	WebPortal();
	~WebPortal();

	/* Don't set this wifi credentials. They are configurated at runtime and stored on EEPROM */
	String ssid;
	String password;

	const char *softAP_ssid = APSSID;
	const char *softAP_password = APPSK;



	/* Soft AP network parameters */
	IPAddress apIP = IPAddress(192, 168, 4, 1);
	IPAddress netMsk = IPAddress(255, 255, 255, 0);

	/* hostname for mDNS. Should work at least on windows. Try http://relay.local */
	char * myHostname;// = "relay";


	void setup();
	void ssdp(const char* deviceName = "ESP8266");
	void loop();




	/** Is this an IP? */
	static boolean isIp(String str);

	/** IP to String? */
	static String toStringIp(IPAddress ip);

	static boolean captivePortal();
	static void handleRoot();
	static void handleWifi();
	static void handleWifiSave();
	static void handleNotFound();
	static bool handleFileRead(String path);
};

extern WebPortal server;


