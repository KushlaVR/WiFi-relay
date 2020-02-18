#pragma once
#include "definitions.h"

enum Mode {
	none = 0,
	server = 1,
	client = 2
};

#define NETWORK_HISTORY_ITEMS 3
#define MAX_TRY_COUNT 10

class StationSettings {
public:
	StationSettings() {};
	~StationSettings() {};
	String prefix;
	String ssid;
	String key;
};



class WiFiController
{
	//How many times try to connect to WIFI router befor switch to AP mode
	int connectionTryBeforChangeModeToAP = MAX_TRY_COUNT;
	ulong lasConnectTry = 0;
	int sucessStation = -1;
	int currentStation = -1;
	StationSettings knownNetworks[NETWORK_HISTORY_ITEMS];

	char * softAP_name;
	char *softAP_password;

	StationSettings * AP;

	const byte DNS_PORT = 53;
	DNSServer dnsServer;

	/* Soft AP network parameters */
	IPAddress apIP = IPAddress(192, 168, 4, 1);
	IPAddress netMsk = IPAddress(255, 255, 255, 0);

	Mode mode = Mode::none;
	String configFile;
	//ESP8266WebServer * webUI;

	uint8_t status = wl_status_t::WL_IDLE_STATUS;

public:
	bool firstRun = false;

	WiFiController();
	~WiFiController();

	void setup(String configFIle);
	bool loop();

	Mode getMode();
	void setMode(Mode mode);
	void setupAP();

	String getMAC();
	String getAPName();
	String statusToString(uint8_t _status);

	void addConfig(String ssid, String password);
	void saveConfig();
};

extern WiFiController wifiController;
