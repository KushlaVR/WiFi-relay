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
#include "Utils.h"
#include <TimeLib.h>


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

	ret.AddValue("systime", Utils::FormatTime(now()));
	ret.AddValue("uptime", String(millis()));

	ret.AddValue("mac", WiFi.macAddress());
	ret.AddValue("localip", WiFi.localIP().toString());
	ret.AddValue("getway", WiFi.gatewayIP().toString());
	ret.AddValue("dnsip",WiFi.dnsIP().toString());

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
	server.jsonOk(&ret);
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
		String minimized = server.getMinimizedPath(path);
		if (SPIFFS.exists(minimized)) path = minimized;
		Serial.printf("path=%s\n", path.c_str());
		if (SPIFFS.exists(path)) {
			//Serial.printf("exist=%s\n", path.c_str());
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
	if (server.hasArg("delete")) {
		if (ApiController::handleGetDeleteTrigger()) return;
	}
	else
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
			else if (type == "termo") {
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
	ret.AddValue("systime", Utils::FormatTime(now()));
	ret.AddValue("uptime", String(millis()));
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
	server.jsonOk(&ret);
	return true;
}

bool ApiController::handleGetDeleteTrigger()
{
	int index = -1;
	int uid = 0;
	if (server.hasArg("delete")) uid = server.arg("delete").toInt();
	if (uid == 0) return false;
	if (server.hasArg("switch")) index = server.arg("switch").toInt();

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

	Trigger * trigger = nullptr;

	Serial.println("Try ti find trigger...");
	Trigger * t = Trigger::getFirstTrigger();
	while (t != nullptr && trigger == nullptr) {
		if (t->proc == proc) {
			if (t->uid == uid) {
				t->Unregister();

				String num = String(t->uid);
				if (num.length() < 2) num = "0" + num;

				String fileName = "/config/" + proc->name + "/" + num + String(t->type) + ".json";
				if (SPIFFS.exists(fileName)) SPIFFS.remove(fileName);

				t = nullptr;
				Serial.println("Trigger unregistered. OK!");
				server.Ok();
				return true;
			}
		}
		t = t->getNextTrigger();
	}


	return false;
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

	Trigger * trigger = nullptr;

	if (uid == 0) {
		Serial.println("new trigger");
		if (type == "onoff")
			trigger = new OnOffTrigger();
		else if (type == "pwm") {
			trigger = new PWMTrigger();
		}
		else if (type == "termo") {
			trigger = new Termostat();
		}
		else {
			return false;
		}
		trigger->uid = Trigger::generateNewUid();
		trigger->proc = (MQTTswitch *)proc;
		trigger->Register();
	}
	else {
		Serial.println("Try ti find trigger...");
		Trigger * t = Trigger::getFirstTrigger();
		while (t != nullptr && trigger == nullptr) {
			if (t->proc == proc) {
				if (t->uid == uid) {
					if (!(String(t->type) == type)) return false;
					trigger = t;
					Serial.println("Found. OK!");
				}
			}
			t = t->getNextTrigger();
		}
	}

	if (trigger == nullptr) return false;

	Serial.println("Fill up trigger");

	if (server.hasArg("name")) {
		trigger->name = server.arg("name");
		Serial.printf("name=%s\n", trigger->name.c_str());
	}

	if (server.hasArg("days")) {
		trigger->days = server.arg("days").toInt();
		Serial.printf("days=%i\n", trigger->days);
	}


	if (type == "onoff") {
		OnOffTrigger * tr = (OnOffTrigger *)trigger;

		if (server.hasArg("time")) {
			tr->time = server.arg("time").toInt();
			Serial.printf("time=%i\n", tr->time);
		}

		String action = "";
		if (server.hasArg("action")) {
			action = server.arg("action");
			if (action == "true")
				tr->action = HIGH;
			else
				tr->action = LOW;
		}
	}
	else if (type == "pwm") {
		PWMTrigger * tr = (PWMTrigger *)trigger;

		if (server.hasArg("onlength")) {
			tr->onlength = server.arg("onlength").toInt();
			Serial.printf("onlength=%i\n", tr->onlength);
		}

		if (server.hasArg("offlength")) {
			tr->offlength = server.arg("offlength").toInt();
			Serial.printf("offlength=%i\n", tr->offlength);
		}

	}
	else if (type == "termo") {
		Termostat * tr = (Termostat *)trigger;

		if (server.hasArg("start")) {
			tr->start = server.arg("start").toInt();
			Serial.printf("start=%i\n", tr->start);
		}

		if (server.hasArg("end")) {
			tr->end = server.arg("end").toInt();
			Serial.printf("end=%i\n", tr->end);
		}

		if (server.hasArg("variable")) {
			tr->variable = server.arg("variable");
			Serial.printf("variable=%s\n", tr->variable.c_str());
		}

		if (server.hasArg("min")) {
			tr->min = server.arg("min").toInt();
			Serial.printf("min=%i\n", tr->min);
		}

		if (server.hasArg("max")) {
			tr->max = server.arg("max").toInt();
			Serial.printf("max=%i\n", tr->max);
		}

	}
	Serial.println("save trigger");
	if (trigger->save()) {
		server.Ok();
		return true;
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
	
	ret.AddValue("systime", Utils::FormatTime(now()));
	ret.AddValue("uptime", String(millis()));
	
	ret.beginArray("items");
	while (proc != nullptr) {
		ret.beginObject();
		proc->printInfo(&ret);
		ret.endObject();
		proc = proc->next;
	};
	ret.endArray();
	ret.endObject();
	server.jsonOk(&ret);
}

void ApiController::handleSetSwitches() {
	Serial.print("switches SET:");
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
		String state = server.arg("state");
		if (state == "on") {
			sw->setState(true);
		}
		else if (state == "x") {
			sw->setState(!sw->isOn());
		}
		else {
			sw->setState(false);
		}
		state = ((sw->isOn()) ? "on" : "off");
		Serial.print("index=");
		Serial.print(index);
		Serial.print("; state=");
		Serial.print(state);
		Serial.println();
		server.Ok("state", state);
		return;
	}
	WebPortal::handleNotFound();
}

