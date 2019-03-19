#pragma once
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266SSDP.h>
#include <DNSServer.h>
#include <ESP8266mDNS.h>
#include <FS.h>
#include "Json.h"
#include "Blinker.h"

/* Set these to your desired softAP credentials. They are not configurable at runtime */
#ifndef APPSK
#define APPSK  "12345678"
#endif

class WebPortal :public ESP8266WebServer
{
private:

	//Blinker * debugLed;

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

public:
	WebPortal();
	~WebPortal();

	/* Don't set this wifi credentials. They are configurated at runtime and stored on EEPROM */
	String ssid;
	String password;


	/* Soft AP network parameters */
	IPAddress apIP = IPAddress(192, 168, 4, 1);
	IPAddress netMsk = IPAddress(255, 255, 255, 0);

	/* hostname for mDNS. Should work at least on windows. Try http://relayXXXXXXX.local */
	char * myHostname;
	char *softAP_password = APPSK;


	void setup();
	void ssdp(const char* deviceName = "ESP8266");
	void loop();


	void sendFile(File file, char* contenttype, bool addGzHeader);
	char * getContentType(String filename);


	/** Is this an IP? */
	static boolean isIp(String str);

	/** IP to String? */
	static String toStringIp(IPAddress ip);

	static boolean captivePortal();
	static void handleRoot();
	static void handleNotFound();
	static void handleUpdate();
	static void handleUpgrade();
	static void updateFiles(String url);
	static void updateFile(String url, String file);
	static void loadURLtoFile(BearSSL::WiFiClientSecure *client, const char *host, const uint16_t port, const char *path, String toFile);

	static bool handleFileRead(String path, bool html = true);
	static String getMinimizedPath(String path);
	static bool replaceMin(String ext, String* path);
	static void jsonOk(JsonString * json);
	static void Ok();
	static void Ok(String name, String value);

};

extern WebPortal server;


