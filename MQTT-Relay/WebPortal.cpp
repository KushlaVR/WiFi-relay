#include "WebPortal.h"
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266SSDP.h>
#include <DNSServer.h>
#include <ESP8266mDNS.h>
#include <FS.h>



String WebPortal::toStringIp(IPAddress ip)
{
	String res = "";
	for (int i = 0; i < 3; i++) {
		res += String((ip >> (8 * i)) & 0xFF) + ".";
	}
	res += String(((ip >> 8 * 3)) & 0xFF);
	return res;
}


boolean WebPortal::isIp(String str)
{
	for (size_t i = 0; i < str.length(); i++) {
		int c = str.charAt(i);
		if (c != '.' && (c < '0' || c > '9')) {
			return false;
		}
	}
	return true;
}



WebPortal::WebPortal() :ESP8266WebServer()
{

}

WebPortal::~WebPortal()
{
}




/*
This example serves a "hello world" on a WLAN and a SoftAP at the same time.
The SoftAP allow you to configure WLAN parameters at run time. They are not setup in the sketch but saved on EEPROM.
Connect your computer or cell phone to wifi network ESP_ap with password 12345678. A popup may appear and it allow you to go to WLAN config. If it does not then navigate to http://192.168.4.1/wifi and config it there.
Then wait for the module to connect to your wifi and take note of the WLAN IP it got. Then you can disconnect from ESP_ap and return to your regular WLAN.
Now the ESP8266 is in your network. You can reach it through http://192.168.x.x/ (the IP you took note of) or maybe at http://esp8266.local too.
This is a captive portal because through the softAP it will redirect any http request to http://192.168.4.1/
*/


void WebPortal::setup() {
	//Generate unique device name based on MAC address of WIFI adapter
	String mac = "relay" + WiFi.macAddress(); mac.replace(":", "");
	myHostname = (char *)calloc(mac.length() + 1, 1);
	strncpy(myHostname, mac.c_str(), mac.length());

	Serial.print("Host name: http://"); Serial.print(myHostname); Serial.println(".local");
	
	//Make device visible in for Microsoft Windows Network
	ssdp(myHostname);
	//read wifi config
	if (SPIFFS.exists("/wifi.json")) {
		DynamicJsonBuffer  jsonBuffer(200);
		File f = SPIFFS.open("/wifi.json", "r");
		f.seek(0);
		JsonObject& root = jsonBuffer.parseObject(f.readString());
		if (root.containsKey("wlan_id")) {
			const char * wlan_id = root["wlan_id"];
			const char * wlan_pwd = root["wlan_pwd"];
			server.ssid = String(wlan_id).c_str();
			server.password = String(wlan_pwd).c_str();
		}
	}
	else {
		Serial.println("WiFi config not found!");
		Serial.println("Start portal");
		WiFi.disconnect();

		Serial.println();
		Serial.println("Configuring access point...");
		/* You can remove the password parameter if you want the AP to be open. */
		WiFi.softAPConfig(apIP, apIP, netMsk);
		WiFi.softAP(softAP_ssid, softAP_password);
		delay(500); // Without delay I've seen the IP address blank
		Serial.print("AP IP address: ");
		Serial.println(WiFi.softAPIP());

		/* Setup the DNS server redirecting all the domains to the apIP */
		dnsServer.setErrorReplyCode(DNSReplyCode::NoError);
		dnsServer.start(DNS_PORT, "*", apIP);
	}

	/* Setup web pages: root, wifi config pages, SO captive portal detectors and not found. */
	on("/", handleRoot);
	on("/wifi", handleWifi);
	on("/wifisave", handleWifiSave);
	on("/generate_204", handleRoot);  //Android captive portal. Maybe not needed. Might be handled by notFound handler.
	on("/fwlink", handleRoot);  //Microsoft captive portal. Maybe not needed. Might be handled by notFound handler.
	onNotFound(handleNotFound);
	begin(); // Web server start
	Serial.println("HTTP server started");

	connect = ssid.length() > 0; // Request WLAN connect if there is a SSID
}

void WebPortal::connectWifi() {
	Serial.println("Connecting as wifi client...");
	WiFi.disconnect();
	WiFi.begin(ssid.c_str(), password.c_str());
	int connRes = WiFi.waitForConnectResult();
	Serial.print("connRes: ");
	Serial.println(connRes);
}

void WebPortal::ssdp(const char* deviceName) {
	//SSDP makes device visible on windows network
	on("/description.xml", HTTP_GET, [&]() {
		SSDP.schema(server.client());
	});
	SSDP.setSchemaURL("description.xml");
	SSDP.setHTTPPort(80);
	SSDP.setName(deviceName);
	SSDP.setModelName("ESP8266 based wifi relay");

	SSDP.setURL("/");
	SSDP.begin();
	SSDP.setDeviceType("upnp:rootdevice");
} //setup





/** Handle root or redirect to captive portal */
void WebPortal::handleRoot() {
	if (captivePortal()) { // If caprive portal redirect instead of displaying the page.
		return;
	}
	/*server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
	server.sendHeader("Pragma", "no-cache");
	server.sendHeader("Expires", "-1");
	server.setContentLength(CONTENT_LENGTH_UNKNOWN);
	server.send(200, "text/html", ""); // Empty content inhibits Content-length header so we have to close the socket ourselves.
	server.sendContent(
		"<html><head></head><body>"
		"<h1>HELLO WORLD!!</h1>"
	);
	if (server.client().localIP() == server.apIP) {
		server.sendContent(String("<p>You are connected through the soft AP: ") + server.softAP_ssid + "</p>");
	}
	else {
		server.sendContent(String("<p>You are connected through the wifi network: ") + server.ssid + "</p>");
	}
	server.sendContent(
		"<p>You may want to <a href='/wifi'>config the wifi connection</a>.</p>"
		"</body></html>"
	);
	server.client().stop(); // Stop is needed because we sent no content length*/

	handleFileRead("/index.html");

}

bool WebPortal::handleFileRead(String path) {
	/*if (path.equals("/home.html")) {
		if (config.homePage.length() > 0) path = config.homePage;
		else if (config.hasDHT) path = "/home_dht.htm";
	}*/
	if (path.endsWith("/")) path += "index.html";
	if (path.equals("/favicon.ico")) path = "icon.svg";
	char* contentType = server.getContentType(path);
	//String minimized = portal.getMinimizedPath(path);
	//String minimizedGz = server + ".gz";
	//bool gz = false;
	//if (SPIFFS.exists(minimizedGz)) { path = minimizedGz; gz = true; }
	//else if (SPIFFS.exists(minimized)) path = minimized;
	//else {
	//	minimized = path + ".gz";
	//	if (SPIFFS.exists(minimized)) { path = minimized; gz = true; }
	//}
	String s = "/html" + path;
	Serial.println("path=" + s);
	if (SPIFFS.exists(s)) {
		File file = SPIFFS.open(s, "r");
		server.sendFile(file, contentType, false);
		file.close();
		//		console.println("Read file from spiffs: " + path);
		return true;
	}
	Serial.println("Not found!!! " + path);
	return false;
}

char* WebPortal::getContentType(String filename) {
	if (hasArg("download")) return "application/octet-stream";
	else if (filename.endsWith(".htm")) return "text/html";
	else if (filename.endsWith(".html")) return "text/html";
	else if (filename.endsWith(".css")) return "text/css";
	else if (filename.endsWith(".js")) return "application/javascript";
	else if (filename.endsWith(".png")) return "image/png";
	else if (filename.endsWith(".gif")) return "image/gif";
	else if (filename.endsWith(".jpg")) return "image/jpeg";
	else if (filename.endsWith(".ico")) return "image/x-icon";
	else if (filename.endsWith(".xml")) return "text/xml";
	else if (filename.endsWith(".svg")) return "image/svg+xml";
	else if (filename.endsWith(".pdf")) return "application/x-pdf";
	else if (filename.endsWith(".zip")) return "application/x-zip";
	else if (filename.endsWith(".gz")) return "application/x-gzip";
	return "text/plain";
}

void WebPortal::sendFile(File file, char* contenttype, bool addGzHeader) {
	String header;
	size_t size_to_send = file.size();
	Serial.println("file=" + String(file.name()) + " filesize=" + String(size_to_send));
	if (addGzHeader) {
		sendHeader("Content-Encoding", "gzip"); // console.print("Header1 ");
		sendHeader("Cache-Control", "max-age=31536000", true);
	}
	_prepareHeader(header, 200, contenttype, size_to_send);
	sendContent(header);
	//	console.print("Header2 ");

	const size_t unit_size = 1024;
	char dataDownloadBuf[unit_size];
	while (size_to_send) {
		size_t will_send = (size_to_send < unit_size) ? size_to_send : unit_size;
		//	console.print("Read ");
		file.readBytes(&dataDownloadBuf[0], will_send);
		//	console.print("Send ");
		size_t sent = _currentClient.write(&dataDownloadBuf[0], will_send);
		//	console.println("Next ");
		if (sent == 0) {
			break;
		}
		size_to_send -= sent;
	}
}

/** Redirect to captive portal if we got a request for another domain. Return true in that case so the page handler do not try to handle the request again. */
boolean WebPortal::captivePortal() {
	if (!WebPortal::isIp(server.hostHeader()) && server.hostHeader() != (String(server.myHostname) + ".local")) {
		Serial.println("Request redirected to captive portal");
		server.sendHeader("Location", String("http://") + WebPortal::toStringIp(server.client().localIP()), true);
		server.send(302, "text/plain", "");   // Empty content inhibits Content-length header so we have to close the socket ourselves.
		server.client().stop(); // Stop is needed because we sent no content length
		return true;
	}
	return false;
}

/** Wifi config page handler */
void WebPortal::handleWifi() {
	server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
	server.sendHeader("Pragma", "no-cache");
	server.sendHeader("Expires", "-1");
	server.setContentLength(CONTENT_LENGTH_UNKNOWN);
	server.send(200, "text/html", ""); // Empty content inhibits Content-length header so we have to close the socket ourselves.
	server.sendContent(
		"<html><head>"
		"<style>"
		"</style>"
		"</head>"
		"<body>"
		"<h1>Wifi config</h1>"
	);
	if (server.client().localIP() == server.apIP) {
		server.sendContent(String("<p>You are connected through the soft AP: ") + server.softAP_ssid + "</p>");
	}
	else {
		server.sendContent(String("<p>You are connected through the wifi network: ") + server.ssid + "</p>");
	}
	server.sendContent(
		"\r\n<br />"
		"<table><tr><th align='left'>SoftAP config</th></tr>"
	);
	server.sendContent(String() + "<tr><td>SSID " + String(server.softAP_ssid) + "</td></tr>");
	server.sendContent(String() + "<tr><td>IP " + WebPortal::toStringIp(WiFi.softAPIP()) + "</td></tr>");
	server.sendContent(
		"</table>"
		"\r\n<br />"
		"<table><tr><th align='left'>WLAN config</th></tr>"
	);
	server.sendContent(String() + "<tr><td>SSID " + String(server.ssid) + "</td></tr>");
	server.sendContent(String() + "<tr><td>IP " + WebPortal::toStringIp(WiFi.localIP()) + "</td></tr>");
	server.sendContent(
		"</table>"
		"\r\n<br />"
		"<table><tr><th align='left'>WLAN list (refresh if any missing)</th></tr>"
	);
	Serial.println("scan start");
	int n = WiFi.scanNetworks();
	Serial.println("scan done");
	if (n > 0) {
		for (int i = 0; i < n; i++) {
			server.sendContent(String() + "\r\n<tr><td>SSID " + WiFi.SSID(i) + String((WiFi.encryptionType(i) == ENC_TYPE_NONE) ? " " : " *") + " (" + WiFi.RSSI(i) + ")</td></tr>");
		}
	}
	else {
		server.sendContent(String() + "<tr><td>No WLAN found</td></tr>");
	}
	server.sendContent(
		"</table>"
		"\r\n<br /><form method='POST' action='wifisave'><h4>Connect to network:</h4>"
		"<input type='text' placeholder='network' name='n'/>"
		"<br /><input type='password' placeholder='password' name='p'/>"
		"<br /><input type='submit' value='Connect/Disconnect'/></form>"
		"<p>You may want to <a href='/'>return to the home page</a>.</p>"
		"</body></html>"
	);
	server.client().stop(); // Stop is needed because we sent no content length
}

/** Handle the WLAN save form and redirect to WLAN config page again */
void WebPortal::handleWifiSave() {
	Serial.println("wifi save");
	server.ssid = server.arg("n");
	server.password = server.arg("p");
	server.sendHeader("Location", "wifi", true);
	server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
	server.sendHeader("Pragma", "no-cache");
	server.sendHeader("Expires", "-1");
	server.send(302, "text/plain", "");    // Empty content inhibits Content-length header so we have to close the socket ourselves.
	server.client().stop(); // Stop is needed because we sent no content length

	if (server.ssid.length() == 0) {
		SPIFFS.remove("/wifi.json");
		Serial.println("wifi config removed");
	}
	else {
		File f = SPIFFS.open("/wifi.json", "w");
		f.seek(0);
		f.print("{\"wlan_id\":\"");
		f.print(server.ssid);
		f.print("\",\"wlan_pwd\":\"");
		f.print(server.password);
		f.println("\"}");
		f.flush();
		f.close();

		Serial.print("{\"wlan_id\":\"");
		Serial.print(server.ssid);
		Serial.print("\",\"wlan_pwd\":\"");
		Serial.print(server.password);
		Serial.println("\"}");
		Serial.println("OK!");
		Serial.println("wifi config saved.");


	}

	//saveCredentials();
	//server.connect = strlen(ssid) > 0; // Request WLAN connect with new credentials if there is a SSID

}


void WebPortal::handleNotFound()
{
	if (captivePortal()) { // If caprive portal redirect instead of displaying the error page.
		return;
	}
	if (handleFileRead(server.uri())) {
		return;
	}
	String message = "File Not Found\n\n";
	message += "URI: ";
	message += server.uri();
	message += "\nMethod: ";
	message += (server.method() == HTTP_GET) ? "GET" : "POST";
	message += "\nArguments: ";
	message += server.args();
	message += "\n";

	for (uint8_t i = 0; i < server.args(); i++) {
		message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
	}
	server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
	server.sendHeader("Pragma", "no-cache");
	server.sendHeader("Expires", "-1");
	server.send(404, "text/plain", message);
}


void WebPortal::loop() {
	if (connect) {
		Serial.println("Connect requested");
		connect = false;
		connectWifi();
		lastConnectTry = millis();
	}
	{
		unsigned int s = WiFi.status();
		if (s == 0 && millis() > (lastConnectTry + 60000)) {
			/* If WLAN disconnected and idle try to connect */
			/* Don't set retry time too low as retry interfere the softAP operation */
			connect = true;
		}
		if (status != s) { // WLAN status change
			Serial.print("Status: ");
			Serial.println(s);
			status = s;
			if (s == WL_CONNECTED) {
				/* Just connected to WLAN */
				Serial.println("");
				Serial.print("Connected to ");
				Serial.println(ssid);
				Serial.print("IP address: ");
				Serial.println(WiFi.localIP());

				// Setup MDNS responder
				if (!MDNS.begin(myHostname)) {
					Serial.println("Error setting up MDNS responder!");
				}
				else {
					Serial.println("mDNS responder started");
					// Add service to MDNS-SD
					MDNS.addService("http", "tcp", 80);
				}
			}
			else if (s == WL_NO_SSID_AVAIL) {
				WiFi.disconnect();
			}
		}
	}
	// Do work:
	//DNS
	dnsServer.processNextRequest();
	//HTTP
	handleClient();
}


WebPortal server;