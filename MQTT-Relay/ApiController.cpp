#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <FS.h>
#include "Json.h"
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"
#include "MQTTprocess.h"
#include "Trigger.h"
#include "ApiController.h"


ApiController::ApiController()
{
}


ApiController::~ApiController()
{
}


void ApiController::setup()
{
	server.on("/api/wifi", handleWifi);
	server.on("/api/wifisave", handleWifiSave);
	server.on("/api/template", handleTemplate);
	server.on("/api/setup", handleSetup);
	server.on("/api/switches", HTTPMethod::HTTP_GET, handleGetSwitches);
	server.on("/api/switches", HTTPMethod::HTTP_POST, handleSetSwitches);
}

/** Wifi config page handler */
void ApiController::handleWifi() {

	Serial.println("scan start");
	int n = WiFi.scanNetworks();

	server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
	server.sendHeader("Pragma", "no-cache");
	server.sendHeader("Expires", "-1");
	server.setContentLength(CONTENT_LENGTH_UNKNOWN);
	Serial.println("scan done");
	JsonString ret = JsonString();
	ret.beginObject();
	if (n > 0) {
		ret.beginArray("ssid");
		for (int i = 0; i < n; i++) {
			ret.beginObject();
			ret.AddValue("name", WiFi.SSID(i));
			ret.AddValue("encryption", String(WiFi.encryptionType(i)));
			ret.AddValue("rssi", String(WiFi.RSSI(i)));
			ret.endObject();
		}
		ret.endArray();
	};
	ret.endObject();
	server.send(200, "application/json", ret);
}

/** Handle the WLAN save form and redirect to WLAN config page again */
void ApiController::handleWifiSave() {
	Serial.println("wifi save");
	server.ssid = server.arg("n");
	server.password = server.arg("p");
	server.sendHeader("Location", "/", true);
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
}


void ApiController::handleTemplate() {
	String name = "";
	if (server.hasArg("name")) {
		name = server.arg("name");
		String path = "/html/content/_" + name + ".html";
		Serial.printf("path=%s\n", path.c_str());

		if (SPIFFS.exists(path)) {
			Serial.printf("exist=%s\n", path.c_str());
			char* contentType = server.getContentType(path);
			File f = SPIFFS.open(path, "r");
			server.sendFile(f, contentType, false);
			f.close();
			return;
		}
	}
	WebPortal::handleNotFound();
}


void ApiController::handleSetup() {
	String type = "";
	if (server.hasArg("type")) {
		type = server.arg("type");
		if (type == "switch") {
			Serial.printf("setup: type=%s\n", type.c_str());
			if (ApiController::handleGetSwitchSetup()) return;
		}
		else if (type == "onoff") {
			Serial.printf("setup: type=%s\n", type.c_str());
			if (ApiController::handleSetTrigger(type)) return;
		}
		else if (type == "pwm") {
			Serial.printf("setup: type=%s\n", type.c_str());
			if (ApiController::handleSetTrigger(type)) return;
		}
	}
	WebPortal::handleNotFound();
}

bool ApiController::handleGetSwitchSetup()
{
	int index = server.arg("index").toInt();

	MQTTprocess * proc = mqtt_connection.getFirstProcess();
	int i = index - 1;
	while (i > 0) {
		if (proc->next != nullptr) {
			proc = proc->next;
			i--;
		}
		else {
			return false;
		}
	}

	JsonString ret = "";
	ret.beginObject();
	ret.beginArray("items");
	Trigger * t = Trigger::getFirstTrigger();
	while (t != nullptr) {
		if (t->proc == proc) {
			ret.beginObject();
			t->printInfo(&ret, true);
			ret.endObject();
		}
		t = t->getNextTrigger();
	}
	ret.endArray();
	ret.endObject();

	Serial.printf("setup: ret=%s\n", ret.c_str());
	server.send(200, "application/json", ret);
	return true;
}

bool ApiController::handleSetTrigger(String type)
{
	int index = -1;
	int uid = 0;
	if (server.hasArg("switch")) index = server.arg("switch").toInt();
	if (server.hasArg("uid")) uid = server.arg("uid").toInt();

	MQTTprocess * proc = mqtt_connection.getFirstProcess();
	int i = 1;
	while (proc != nullptr) {
		if (i == index) break;
		proc = proc->next;
		i++;
	};

	if (proc == nullptr) {
		return false;
	};

	Serial.printf("Process=%s\n", proc->name.c_str());

	if (type == "onoff") {

		OnOffTrigger * tr = nullptr;

		if (uid == 0) {
			Serial.println("new trigger");
			//new trigger
			tr = new OnOffTrigger();
			tr->proc = (MQTTswitch *)proc;
			tr->Register();
		}
		else {
			Serial.println("Try ti find trigger...");
			Trigger * t = Trigger::getFirstTrigger();
			while (t != nullptr && tr == nullptr) {
				if (t->proc == proc) {
					if (t->uid == uid) {
						if (!(String(t->type) == "onoff")) return false;
						tr = (OnOffTrigger *)t;
						Serial.println("Found. OK!");
					}
				}
				t = t->getNextTrigger();
			}
		}
		
		Serial.println("Fill up trigger");
		if (server.hasArg("name")) tr->name = server.arg("name");
		Serial.printf("name=%s\n",tr->name.c_str());
		Serial.printf("days=%s\n", server.arg("days").c_str());
		if (server.hasArg("days")) tr->days = server.arg("days").toInt();
		Serial.printf("days=%i\n", tr->days);
		if (server.hasArg("time")) tr->time = server.arg("time").toInt();
		Serial.printf("time=%i\n", tr->time);

		String action = "";
		if (server.hasArg("action")) action = server.arg("action");
		if (action == "true")
			tr->action = HIGH;
		else
			tr->action = LOW;
		if (tr->save()) {
			JsonString ret = JsonString();
			ret.beginObject();
			ret.AddValue("status", "OK");
			ret.endObject();
			server.send(200, "application/json", ret);
		}
	}
	else if (type == "pwm") {

	}
	return false;
}



void ApiController::handleGetSwitches() {
	Serial.println("switches GET:");

	server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
	server.sendHeader("Pragma", "no-cache");
	server.sendHeader("Expires", "-1");
	server.setContentLength(CONTENT_LENGTH_UNKNOWN);
	JsonString ret = JsonString();
	ret.beginObject();
	MQTTprocess * proc = mqtt_connection.getFirstProcess();
	ret.beginArray("items");
	while (proc != nullptr) {
		ret.beginObject();
		proc->printInfo(&ret);
		ret.endObject();
		proc = proc->next;
	};
	ret.endArray();
	ret.endObject();
	server.send(200, "application/json", ret);
}

void ApiController::handleSetSwitches() {
	Serial.println("switches GET:");
	if (server.hasArg("index") && server.hasArg("state")) {

		int index = server.arg("index").toInt();

		MQTTprocess * proc = mqtt_connection.getFirstProcess();
		int i = 1;
		while (proc != nullptr) {
			if (i == index) break;
			proc = proc->next;
			i++;
		};

		if (proc == nullptr) {
			WebPortal::handleNotFound();
			return;
		}

		if (proc->type != "switch") {
			WebPortal::handleNotFound();
			return;
		}
		MQTTswitch * sw = (MQTTswitch *)proc;
		sw->setState(server.arg("state") == "on");
		server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
		server.sendHeader("Pragma", "no-cache");
		server.sendHeader("Expires", "-1");
		server.setContentLength(CONTENT_LENGTH_UNKNOWN);
		JsonString ret = JsonString();
		ret.beginObject();
		ret.AddValue("status", "OK");
		ret.endObject();
		server.send(200, "application/json", ret);
		return;
	}
	WebPortal::handleNotFound();
}

