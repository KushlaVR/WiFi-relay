#include "ApiController.h"

SessionClass* ApiController::session = nullptr;
bool ApiController::useAuth = false;


ApiController::ApiController()
{
}


ApiController::~ApiController()
{
}

void ApiController::setup(bool useAuth)
{
	ApiController::useAuth = useAuth;
	webServer.on("/api/auth", handleAuth);
	webServer.on("/api/wifi", handleWifi);
	webServer.on("/api/wifisave", handleWifiSave);
	webServer.on("/api/mqtt", handleMQTT);
	webServer.on("/api/mqttsave", handleMQTTSave);
	webServer.on("/api/startup", handleStartup);
	webServer.on("/api/template", handleTemplate);
	webServer.on("/api/setup", handleSetup);
	webServer.on("/api/switches", HTTPMethod::HTTP_GET, handleGetSwitches);
	webServer.on("/api/switches", HTTPMethod::HTTP_POST, handleSetSwitches);
	webServer.on("/api/menu", handleMenu);
	webServer.on("/api/restart", handleRestart);
	webServer.on("/logout.html", handleLogout);
}

void ApiController::handleRestart()
{
	JsonString ret = JsonString();
	ret.beginObject();

	ret.AddValue("systime", Utils::FormatTime(webServer.timeZone->toLocal(now())));
	ret.AddValue("uptime", String(millis()));

	ret.endObject();

	webServer.jsonOk(&ret);

	while (true) { ; }
}

void ApiController::handleMQTT()
{
	Serial.println("MQTT get");

	JsonString ret = JsonString();
	ret.beginObject();

	ret.AddValue("systime", Utils::FormatTime(webServer.timeZone->toLocal(now())));
	ret.AddValue("uptime", String(millis()));
	mqttController.printConfig(&ret);
	ret.endObject();
	webServer.jsonOk(&ret);

}

void ApiController::handleMQTTSave()
{
	Serial.println("MQTT save");
	if (!webServer.hasArg("broker")) {
		Serial.println("broker require");
		webServer.handleNotFound();
	}
	if (!webServer.hasArg("port")) {
		Serial.println("port require");
		webServer.handleNotFound();
	}
	if (!webServer.hasArg("user")) {
		Serial.println("user require");
		webServer.handleNotFound();
	}
	if (!webServer.hasArg("key")) {
		Serial.println("key require");
		webServer.handleNotFound();
	}
	mqttController.saveConfig(webServer.arg("broker"), webServer.arg("port"), webServer.arg("user"), webServer.arg("key"));
	mqttController.loadConfig();
	webServer.Ok();
}

void ApiController::handleAuth()
{
	String user = "";
	String password = "";

	if (webServer.hasArg("user")) user = webServer.arg("user");
	if (webServer.hasArg("password")) password = webServer.arg("password");
	if (user.length() == 0) {
		WebUIController::handleNotFound();
		return;
	}
	user.toLowerCase();

	if (password.length() == 0) {
		WebUIController::handleNotFound();
		return;
	}

	if (user == "user" && password == "password") {
		String fName = "/log/" + user;
		String sessionID;
		if (SPIFFS.exists(fName)) {
			File f = SPIFFS.open(fName, "r");
			sessionID = f.readString();
			f.close();
		}
		else {
			ulong key = millis() * 349UL;
			File f = SPIFFS.open(fName, "w");
			f.print(key);
			f.flush();
			f.close();
			sessionID = String(key);
		}
		if (session == nullptr) session = new SessionClass();
		session->user = user;
		session->session = sessionID;

		webServer.sendHeader("Set-Cookie", "session=" + sessionID + "; path=/;");
		webServer.sendHeader("Set-Cookie", "user=" + user + "; path=/;");

		if (webServer.hasArg("json")) {
			JsonString ret = JsonString();
			ret.beginObject();
			ret.AddValue("user", "user");
			ret.AddValue("session", sessionID);
			ret.endObject();
			webServer.jsonOk(&ret);
			return;
		}
	}
	else {
		webServer.sendHeader("Set-Cookie", "session=;; path=/;");
		webServer.sendHeader("Set-Cookie", "user=;; path=/;");
	}
	webServer.sendHeader("Location", String("/"), true);
	webServer.send(302, "text/plain", "");

}

void ApiController::handleLogout() {
	webServer.sendHeader("Set-Cookie", "session=;; path=/;");
	webServer.sendHeader("Set-Cookie", "user=;; path=/;");
	webServer.sendHeader("Location", String("/"), true);
	webServer.send(302, "text/plain", "");
}

bool ApiController::isAuthorized()
{
	if (!ApiController::useAuth) return true;

	if (webServer.hasHeader("Cookie")) {
		Serial.print("Found cookie: ");
		String cookie = webServer.header("Cookie");
		Serial.println(cookie);
		int sIndex = cookie.indexOf("session=");
		int uIndex = cookie.indexOf("user=");
		if (sIndex != -1 && uIndex != -1) {
			int sEnd = cookie.indexOf(";", sIndex);
			if (sEnd <= 0) sEnd = cookie.indexOf(",", sIndex);
			if (sEnd <= 0) sEnd = cookie.length();


			int uEnd = cookie.indexOf(";", uIndex);
			if (uEnd <= 0) uEnd = cookie.indexOf(",", uIndex);
			if (uEnd <= 0) uEnd = cookie.length();

			String user = cookie.substring(uIndex + 5, uEnd);
			String sesionID = cookie.substring(sIndex + 8, sEnd);
			if (session == nullptr) {
				String fName = "/log/" + user;
				if (SPIFFS.exists(fName)) {
					File f = SPIFFS.open(fName, "r");
					String storedSesion = f.readString();
					f.close();
					Serial.printf("user=%s key=%s, s=%s", user.c_str(), sesionID.c_str(), storedSesion.c_str());
					if (storedSesion == sesionID) {
						//Serial.println("Authentification Successful");
						session = new SessionClass();
						session->user = user;
						session->session = sesionID;
						return true;
					}
				}
			}
			else {
				if (session->user == user && session->session == sesionID) {
					return true;
				}
			}
		}
	}
	Serial.println("Authentification Failed!");
	//	webServer.sendHeader("Set-Cookie", "session=");
	//	webServer.sendHeader("Set-Cookie", "user=");
	webServer.handleNotFound();
	return false;
}

void ApiController::handleWifi()
{
	if (!ApiController::isAuthorized()) return;

	Serial.println("scan start");
	int n = WiFi.scanNetworks();

	webServer.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
	webServer.sendHeader("Pragma", "no-cache");
	webServer.sendHeader("Expires", "-1");
	webServer.setContentLength(CONTENT_LENGTH_UNKNOWN);
	Serial.println("scan done");
	JsonString ret = JsonString();
	ret.beginObject();

	ret.AddValue("systime", Utils::FormatTime(webServer.timeZone->toLocal(now())));
	ret.AddValue("uptime", String(millis()));

	ret.AddValue("mac", WiFi.macAddress());
	ret.AddValue("localip", WiFi.localIP().toString());
	ret.AddValue("getway", WiFi.gatewayIP().toString());
	ret.AddValue("dnsip", WiFi.dnsIP().toString());

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
	webServer.jsonOk(&ret);
}

void ApiController::handleWifiSave()
{
	if (!ApiController::isAuthorized()) return;
	Serial.println("wifi save");
	wifiController.addConfig(webServer.arg("n"), webServer.arg("p"));
	String mode = webServer.arg("mode");
	if (mode == "server") {
		wifiController.firstRun = true;
	}
	else if (mode == "client") {
		wifiController.firstRun = false;
	}
	wifiController.saveConfig();

	webServer.sendHeader("Location", "/", true);
	webServer.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
	webServer.sendHeader("Pragma", "no-cache");
	webServer.sendHeader("Expires", "-1");
	webServer.send(302, "text/plain", "");    // Empty content inhibits Content-length header so we have to close the socket ourselves.
	webServer.client().stop(); // Stop is needed because we sent no content length

}

void ApiController::handleStartup()
{
	if (!ApiController::isAuthorized()) return;
	Serial.println("startup GET:");
	Output* proc = (Output*)outs.getFirst();
	while (proc != nullptr) {
		if (webServer.hasArg(proc->getName())) {
			String arg = webServer.arg(proc->getName());
			Serial.printf("Switch[%s] = %s\n", proc->getName().c_str(), arg.c_str());
			proc->saveStartup((arg) == "on");
		}
		proc = (Output*)proc->next;
	};
	webServer.Ok();
}

void ApiController::handleTemplate()
{
	String name = "";
	if (webServer.hasArg("name")) {
		name = webServer.arg("name");
		String path = "/html/v/_" + name + ".html";
		String minimized = webServer.getMinimizedPath(path);
		if (SPIFFS.exists(minimized)) path = minimized;
		Serial.printf("path=%s\n", path.c_str());
		if (SPIFFS.exists(path)) {
			//Serial.printf("exist=%s\n", path.c_str());
			char* contentType = webServer.getContentType(path);
			File f = SPIFFS.open(path, "r");
			webServer.sendFile(f, contentType, false);
			f.close();
			return;
		}
	}
	WebUIController::handleNotFound();
}

void ApiController::handleMenu()
{
	Serial.println("menu GET:");

	webServer.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
	webServer.sendHeader("Pragma", "no-cache");
	webServer.sendHeader("Expires", "-1");
	webServer.setContentLength(CONTENT_LENGTH_UNKNOWN);
	JsonString ret = JsonString();
	ret.beginObject();

	ret.AddValue("systime", Utils::FormatTime(webServer.timeZone->toLocal(now())));
	ret.AddValue("uptime", String(millis()));

	ret.beginArray("items");

	ret.beginObject();
	ret.AddValue("name", "Підє&#39;днання");
	ret.AddValue("href", "./wifi.html");
	ret.AddValue("target", "_self");
	ret.endObject();

	ret.beginObject();
	ret.AddValue("name", "Поновлення");
	ret.AddValue("href", "./update.html");
	ret.AddValue("target", "_self");
	ret.endObject();

	ret.beginObject();
	ret.AddValue("name", "Допомога");
	ret.AddValue("href", "./help.html");
	ret.AddValue("target", "_self");
	ret.endObject();

	ret.beginObject();
	ret.AddValue("name", "Logout");
	ret.AddValue("href", "./logout.html");
	ret.AddValue("target", "_self");
	ret.endObject();

	/*ret.beginObject();
	ret.AddValue("name", "GitHub");
	ret.AddValue("href", "https://github.com/KushlaVR/WiFi-relay");
	ret.AddValue("target", "_blank");
	ret.endObject();*/

	ret.endArray();
	ret.endObject();
	webServer.jsonOk(&ret);
}

void ApiController::handleSetup()
{
	if (!ApiController::isAuthorized()) return;
	String type = "";
	if (webServer.hasArg("delete")) {
		if (ApiController::handleDeleteTrigger()) return;
	}
	else
		if (webServer.hasArg("type")) {
			type = webServer.arg("type");
			if (type == "switch") {
				Serial.printf("setup: type=%s\n", type.c_str());
				if (ApiController::handleGetTriggersForSwitch()) return;
			}
			else {
				if (ApiController::handleUpdateTrigger(type)) return;
			}
		}
	WebUIController::handleNotFound();
}

bool ApiController::handleGetTriggersForSwitch()
{
	int index = webServer.arg("index").toInt();

	MQTTProcess* proc = (MQTTProcess*)mqttController.getFirst();
	int i = index - 1;
	while (i > 0) {
		if (proc->next != nullptr) {
			proc = (MQTTProcess*)proc->next;
			i--;
		}
		else {
			return false;
		}
	}

	JsonString ret = "";
	ret.beginObject();
	ret.AddValue("systime", Utils::FormatTime(webServer.timeZone->toLocal(now())));
	ret.AddValue("uptime", String(millis()));
	ret.beginArray("items");
	Trigger* t = Trigger::getFirstTrigger();
	while (t != nullptr) {
		if (proc->type == "switch") {
			MQTTSwitch* sw = (MQTTSwitch*)proc;
			if (t->proc == sw->out) {
				ret.beginObject();
				t->printInfo(&ret, true);
				ret.endObject();
			}
		}
		t = t->getNextTrigger();
	}
	ret.endArray();
	ret.endObject();

	Serial.printf("setup: ret=%s\n", ret.c_str());
	webServer.jsonOk(&ret);
	return true;
}

bool ApiController::handleUpdateTrigger(String type)
{
	Serial.printf("setup: type=%s\n", type.c_str());

	int index = -1;
	int uid = 0;
	if (webServer.hasArg("switch")) index = webServer.arg("switch").toInt();
	if (webServer.hasArg("uid")) uid = webServer.arg("uid").toInt();

	MQTTProcess* proc = (MQTTProcess*)mqttController.getFirst();
	int i = 1;
	while (proc != nullptr) {
		if (i == index) break;
		proc = (MQTTProcess*)proc->next;
		i++;
	};

	if (proc == nullptr) {
		return false;
	};
	if (proc->type != "switch") {
		return false;
	}

	Serial.printf("Process=%s\n", proc->name.c_str());
	MQTTSwitch* sw = (MQTTSwitch*)proc;

	Trigger* trigger = nullptr;

	if (uid == 0) {
		Serial.println("new trigger");
		trigger = CreateTriggerByType(type);
		if (trigger == nullptr) return false;
		trigger->proc = (Output*)sw->out;
		trigger->Register();
	}
	else {
		Serial.println("Try ti find trigger...");
		Trigger* t = Trigger::getFirstTrigger();
		while (t != nullptr && trigger == nullptr) {
			if (t->proc == sw->out) {
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

	trigger->fillFromWebServer(&webServer);

	Serial.println("save trigger");
	if (trigger->save()) {
		webServer.Ok();
		return true;
	}
	return false;
}

bool ApiController::handleDeleteTrigger()
{
	int index = -1;
	int uid = 0;
	if (webServer.hasArg("delete")) uid = webServer.arg("delete").toInt();
	if (uid == 0) return false;
	if (webServer.hasArg("switch")) index = webServer.arg("switch").toInt();

	MQTTProcess* proc = (MQTTProcess*)mqttController.getFirst();
	int i = 1;
	while (proc != nullptr) {
		if (i == index) break;
		proc = (MQTTProcess*)proc->next;
		i++;
	};

	if (proc == nullptr) {
		return false;
	};

	if (proc->type != "switch") {
		return false;
	}

	MQTTSwitch* sw = (MQTTSwitch*)proc;

	Serial.printf("Process=%s\n", proc->name.c_str());

	//Trigger * trigger = nullptr;

	Serial.println("Try ti find trigger...");
	Trigger* t = Trigger::getFirstTrigger();
	while (t != nullptr) {
		if (t->proc == sw->out) {
			if (t->uid == uid) {
				Serial.println("Try to unregister.");
				t->Unregister();
				Serial.println("Try to delete config.");
				t->DeleteConfig();
				t = nullptr;
				Serial.println("Trigger unregistered. OK!");
				webServer.Ok();
				return true;
			}
		}
		t = t->getNextTrigger();
	}
	return false;
}

void ApiController::handleGetSwitches()
{
	Serial.println("switches GET:");

	webServer.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
	webServer.sendHeader("Pragma", "no-cache");
	webServer.sendHeader("Expires", "-1");
	webServer.setContentLength(CONTENT_LENGTH_UNKNOWN);
	JsonString ret = JsonString();
	ret.beginObject();
	MQTTProcess* proc = (MQTTProcess*)mqttController.getFirst();

	ret.AddValue("systime", Utils::FormatTime(webServer.timeZone->toLocal(now())));
	ret.AddValue("uptime", String(millis()));

	ret.beginArray("items");
	while (proc != nullptr) {
		ret.beginObject();
		proc->printInfo(&ret);
		ret.endObject();
		proc = (MQTTProcess*)proc->next;
	};
	ret.endArray();
	ret.endObject();
	webServer.jsonOk(&ret);
}

void ApiController::handleSetSwitches()
{
	if (!ApiController::isAuthorized()) return;
	Serial.print("switches SET:");
	if (webServer.hasArg("index") && webServer.hasArg("state")) {

		int index = webServer.arg("index").toInt();

		MQTTProcess* proc = (MQTTProcess*)mqttController.getFirst();
		int i = 1;
		while (proc != nullptr) {
			if (i == index) break;
			proc = (MQTTProcess*)proc->next;
			i++;
		};

		if (proc == nullptr) {
			WebUIController::handleNotFound();
			return;
		}

		if (proc->type != "switch") {
			WebUIController::handleNotFound();
			return;
		}
		MQTTSwitch* sw = (MQTTSwitch*)proc;
		String state = webServer.arg("state");
		if (state == "on") {
			sw->setState(true);
		}
		else if (state == "x") {
			sw->setState(!sw->out->isOn());
		}
		else {
			sw->setState(false);
		}
		state = ((sw->out->isOn()) ? "on" : "off");
		Serial.print("index=");
		Serial.print(index);
		Serial.print("; state=");
		Serial.print(state);
		Serial.println();
		webServer.Ok("state", state);
		return;
	}
	WebUIController::handleNotFound();

}



ApiController apiController;