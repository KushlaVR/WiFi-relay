#include "WiFiController.h"



WiFiController::WiFiController()
{
	AP = new StationSettings();
}


WiFiController::~WiFiController()
{
}

void WiFiController::setup(String configFIle)
{
	this->configFile = configFIle;
	WiFi.begin();
	WiFi.disconnect();
	if (SPIFFS.exists(configFIle)) {
		File f = SPIFFS.open(configFIle, "r");
		JsonString json = JsonString(f.readString());
		String s = json.getValue("ssid");
		AP->prefix = s;
		if (s.length() > 0) {
			AP->ssid = AP->prefix + getMAC();
		}
		s = json.getValue("mode");
		if (s == "server") firstRun = true;
		AP->key = json.getValue("key");
		if (AP->key.length() == 0) {
			//When APkey is empty - AP key = MAC address
			AP->key = getMAC();
		}
		s = json.getValue("ip");
		if (s.length() > 0) {
			apIP.fromString(s);
		}
		s = json.getValue("mask");
		if (s.length() > 0) {
			netMsk.fromString(s);
		}
		int  i = json.getValuePos("wifi", 0);
		if (i > 0) {
			//read all known wifi stations
			int index = 0;
			while (index < 3) {
				i = json.getValuePos("ssid", i) - 8;
				if (i < 0) break;
				knownNetworks[index].ssid = json.getValue("ssid", i);
				if (knownNetworks[index].ssid == "") break;
				knownNetworks[index].key = json.getValue("key", i);
				Serial.printf("ssid %i:%s\n", index, knownNetworks[index].ssid.c_str());
				i += 5;
				index++;
			}
			if (index > 0)
				setMode(Mode::client);
			else
				setMode(Mode::server);
		}
		f.close();
	}
	else {
		firstRun = true;
		setMode(Mode::server);
	}
}

bool WiFiController::loop()
{
	if (WiFi.status() != status) {
		status = WiFi.status();
		Serial.print("Status: ");
		Serial.println(statusToString(status));

		if (WiFi.status() == wl_status_t::WL_CONNECTED) {
			Serial.println("");
			Serial.print("Connected to ");
			Serial.println(knownNetworks[currentStation].ssid);
			Serial.print("IP address: ");
			Serial.println(WiFi.localIP());
		}
	}
	if (mode == Mode::server) {
		dnsServer.processNextRequest();
		return true;//Serv clients
	}
	else if (mode == Mode::client) {
		if ((WiFi.status() == wl_status_t::WL_CONNECTED)) {
			connectionTryBeforChangeModeToAP = MAX_TRY_COUNT;
			sucessStation = currentStation;
			return true;//Serv clients
		}
		else /*if (WiFi.status() == wl_status_t::WL_DISCONNECTED)*/ {

			//Пробуємо_підєднатися до першої станції
			if (currentStation == -1 || lasConnectTry == 0 || ((millis() - lasConnectTry) > 20000)) {
				if (sucessStation >= 0)
				{
					currentStation = sucessStation; sucessStation = -1;
				}
				else
					currentStation++;

				if (currentStation == NETWORK_HISTORY_ITEMS) {
					currentStation = 0;
					lasConnectTry = millis();
					connectionTryBeforChangeModeToAP--;
					if (connectionTryBeforChangeModeToAP == 0) {
						connectionTryBeforChangeModeToAP = MAX_TRY_COUNT;
						setMode(Mode::server);
					}
				}
				Serial.print("Current station: ");
				Serial.println(knownNetworks[currentStation].ssid);
				int networksFound = WiFi.scanNetworks();
				Serial.printf("%d network(s) found\n", networksFound);
				String ssid;
				for (int i = 0; i < networksFound; i++)
				{
					ssid = WiFi.SSID(i);
					//Serial.printf("%d: %s, Ch:%d (%ddBm) %s\n", i + 1, WiFi.SSID(i).c_str(), WiFi.channel(i), WiFi.RSSI(i), WiFi.encryptionType(i) == ENC_TYPE_NONE ? "open" : "");
					if (knownNetworks[currentStation].ssid == ssid) {
						WiFi.disconnect();
						WiFi.hostname(AP->ssid);
						WiFi.begin(knownNetworks[currentStation].ssid.c_str(), knownNetworks[currentStation].key.c_str());
						Serial.print("Connect to : ");
						Serial.println(knownNetworks[currentStation].ssid.c_str());
						lasConnectTry = millis();
						return false;
					}
				}
			}
		}
	}
	return false;//works Offline
}

Mode WiFiController::getMode()
{
	return mode;
}

void WiFiController::setMode(Mode mode)
{
	if (this->mode == mode) return;
	WiFi.disconnect();
	this->mode = mode;
	switch (this->mode)
	{
	case Mode::none:
		dnsServer.stop();
		break;
	case Mode::server:
		Serial.println("Server mode");
		setupAP();
		break;
	case Mode::client:
		dnsServer.stop();
		Serial.println("Client mode");
		if (firstRun) setupAP();
		break;
	default:
		break;
	}
}

void WiFiController::setupAP()
{
	Serial.println("Configuring access point...");
	WiFi.softAPConfig(apIP, apIP, netMsk);

	//Store soft AP params in fixed memory
	softAP_name = (char *)calloc(AP->ssid.length() + 1, 1);
	strncpy(softAP_name, AP->ssid.c_str(), AP->ssid.length());

	softAP_password = (char *)calloc(AP->key.length() + 1, 1);
	strncpy(softAP_password, AP->key.c_str(), AP->key.length());

	WiFi.softAP(softAP_name, softAP_password);

	delay(500); // Without delay I've seen the IP address blank
	Serial.print("AP IP address: ");
	Serial.println(WiFi.softAPIP());

	/* Setup the DNS server redirecting all the domains to the apIP */
	dnsServer.setErrorReplyCode(DNSReplyCode::NoError);
	dnsServer.start(DNS_PORT, "*", apIP);
}

String WiFiController::getMAC()
{
	String mac = WiFi.macAddress(); mac.replace(":", "");
	return mac;
}

String WiFiController::getAPName()
{
	return AP->ssid;
}

String WiFiController::statusToString(uint8_t _status)
{
	switch (_status)
	{
	case WL_NO_SHIELD:
		return "No shield";
	case WL_IDLE_STATUS:
		return "IDLE";
	case WL_NO_SSID_AVAIL:
		return "SSID avail";
	case WL_SCAN_COMPLETED:
		return "Scan completed";
	case WL_CONNECTED:
		return "Connected";
	case WL_CONNECT_FAILED:
		return "Connect failed";
	case WL_CONNECTION_LOST:
		return "Connection lost";
	case WL_DISCONNECTED:
		return "Disconneced";
	default:
		break;
	}
	return String();
}

void WiFiController::addConfig(String ssid, String password)
{
	int indexToWrite = sucessStation;
	if (indexToWrite == -1) indexToWrite = currentStation;
	indexToWrite++;
	if (indexToWrite == NETWORK_HISTORY_ITEMS) indexToWrite = 0;
	knownNetworks[indexToWrite].ssid = ssid;
	knownNetworks[indexToWrite].key = password;
}

void WiFiController::saveConfig()
{
	JsonString cfg = "";
	cfg.beginObject();
	if (firstRun)
		cfg.AddValue("mode", "server");
	cfg.AddValue("ssid", AP->prefix);
	cfg.AddValue("key", AP->key);
	cfg.AddValue("ip", WebUIController::ipToString(apIP));
	cfg.AddValue("mask", WebUIController::ipToString(netMsk));

	cfg.beginArray("wifi");

	for (int i = 0; i < NETWORK_HISTORY_ITEMS; i++) {
		if (knownNetworks[i].ssid.length() > 0) {
			cfg.beginObject();
			cfg.AddValue("ssid", knownNetworks[i].ssid);
			cfg.AddValue("key", knownNetworks[i].key);
			cfg.endObject();
		}
	}

	cfg.endArray();
	cfg.endObject();


	File f = SPIFFS.open(this->configFile, "w");
	f.seek(0);
	f.print(cfg);
	f.flush();
	f.close();
}


WiFiController wifiController;
