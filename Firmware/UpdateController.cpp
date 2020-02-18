#include "UpdateController.h"



UpdateController::UpdateController()
{
}


UpdateController::~UpdateController()
{
}

void UpdateController::setup()
{
	webServer.on("/update", handleUpdate);
	webServer.on("/upgrade", handleUpgrade);
}

void UpdateController::loop()
{
}

void UpdateController::handleUpdate()
{
	Serial.println("Update files");

	String url = "";
	if (webServer.hasArg("url")) {
		url = webServer.arg("url");
		if (webServer.hasArg("file")) {
			webServer.Ok();
			webServer.client().stop();
			updateFile(url, webServer.arg("file"));
			return;
		}
	}
	else {
		if (SPIFFS.exists("/html/files.txt")) {
			File f = SPIFFS.open("/html/files.txt", "r");
			updateController.host = f.readStringUntil('\n');
			updateController.host.trim();
			url = f.readStringUntil('\n');
			url.trim();
			f.close();
		}
	}
	if (updateController.host.length() == 0 || updateController.host.startsWith("/")) {
		updateController.host = updateController.defaultHost;
		url = updateController.defaultURL;
	}

	Serial.print("HOST=");
	Serial.println(updateController.host);
	Serial.print("URL=");
	Serial.println(url);

	if (url.length() > 0) {
		webServer.Ok();
		webServer.client().stop();
		Serial.printf("url=%s", url.c_str());
		updateFiles(url);
	}
	else {
		WebUIController::handleNotFound();
		return;
	}
}

void UpdateController::handleUpgrade()
{
	Serial.println("Upgrade firmware");
	UpdateController::updateFile(updateController.defaultURL, "/firmware.bin");
	if (SPIFFS.exists("/firmware.bin")) {
		File file = SPIFFS.open("/firmware.bin", "r");

		uint32_t maxSketchSpace = (ESP.getFreeSketchSpace() - 0x1000) & 0xFFFFF000;
		if (!Update.begin(maxSketchSpace, U_FLASH)) { //start with max available size
			Update.printError(Serial);
			Serial.println("ERROR");
		}

		Serial.println("Updating");
		while (file.available()) {
			uint8_t ibuffer[128];
			file.read((uint8_t *)ibuffer, 128);
			//Serial.println((char *)ibuffer);
			Update.write(ibuffer, sizeof(ibuffer));
		}
		Serial.print(Update.end(true));
		Serial.println("Done!");
		file.close();
		//SPIFFS.remove("/firmware.bin");
		Serial.println("Finished");
		while (1) {};
		//system_upgrade_reboot();
	}
	else {
		Serial.println("No firmware available...");
	}
}

void UpdateController::updateFiles(String url)
{
	updateFile(url, "/html/files.txt");
	if (SPIFFS.exists("/html/files.txt")) {
		File f = SPIFFS.open("/html/files.txt", "r");
		updateController.host = f.readStringUntil('\n');
		updateController.host.trim();
		url = f.readStringUntil('\n');
		url.trim();
		if (updateController.host.length() == 0 || updateController.host.startsWith("/")) {
			updateController.host = updateController.defaultHost;
			url = updateController.defaultURL;
		}
		String fName = f.readStringUntil('\n');
		while (fName.length() > 0) {
			updateFile(url, fName);
			fName = f.readStringUntil('\n');
		}
		f.close();
	}
}

void UpdateController::updateFile(String url, String file)
{
	BearSSL::WiFiClientSecure client;
	client.setInsecure();
	UpdateController::loadURLtoFile(&client, updateController.host.c_str(), updateController.httpsPort, (url + file).c_str(), file);
}

void UpdateController::loadURLtoFile(BearSSL::WiFiClientSecure * client, const char * host, const uint16_t port, const char * path, String toFile)
{
	Serial.print("Loadnig: ");
	Serial.println(toFile);
	//Serial.println(path);
	//Serial.printf("Trying: %s:443...", host);
	//Serial.printf("path:\n", path);
	client->connect(host, port);
	if (!client->connected()) {
		Serial.printf("*** Can't connect. ***\n-------\n");
		return;
	}
	Serial.printf("Connected!\n-------\n");
	client->write("GET ");
	client->write(path);
	client->write(" HTTP/1.0\r\nHost: ");
	client->write(host);
	client->write("\r\nUser-Agent: ESP8266\r\n");
	client->write("\r\n");

	if (client->connected()) {
		Serial.println("client->connected");
		while (client->connected()) {
			String line = client->readStringUntil('\n');
			if (line.startsWith("HTTP/")) {
				if (!line.substring(9).startsWith("200")) {
					client->stop();
					Serial.println(line);
					return;
				}
			}
			//Serial.println(line);
			if (line == "\r") {
				Serial.println("headers received");
				break;
			}
		}

		File f = SPIFFS.open(toFile, "w");
		while (client->connected()) {
			uint8_t tmp[32];
			memset(tmp, 0, 32);
			int rlen = client->read((uint8_t*)tmp, sizeof(tmp) - 1);
			yield();
			if (rlen < 0) {
				break;
			}
			if (rlen > 0) {
				f.write(tmp, rlen);
				Serial.print(".");
			}
		}
		f.flush();
		f.close();
		Serial.println("Done");
	}
	client->stop();
	Serial.println("client->stopped");

}


UpdateController updateController;