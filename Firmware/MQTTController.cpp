#include "MQTTController.h"



MQTTController::MQTTController()
{

}


MQTTController::~MQTTController()
{

}

void MQTTController::setup()
{
	loadConfig();

	if (strlen(broker) > 0) {
		if (strlen(user) == 0) {
			Serial.println("create mqtt");
			connection = new Adafruit_MQTT_Client(&client, (const char *)broker, port);
		}
		else
		{
			connection = new Adafruit_MQTT_Client(&client, (const char *)broker, port, (const char *)user, (const char *)key);
		}
		lastConnect = millis();
		connectInterval = 0UL;
	}
	else {
		connection = nullptr;
	}
}

bool MQTTController::Available()
{
	serverOnline = false;
	if (connection == nullptr) return false;
	if (WiFi.status() != WL_CONNECTED) return false;
	if ((millis() - lastConnect) > connectInterval) {
		// Function to connect and reconnect as necessary to the MQTT server.
		// Should be called in the loop function and it will take care if connecting.
		int8_t ret;
		// Stop if already connected.
		if (connection->connected()) {
			numberOfTry = 0;
			serverOnline = true;
			return true;
		};
		Serial.print("Connecting to MQTT... ");
		if ((ret = connection->connect()) != 0) { // connect will return 0 for connected
			numberOfTry++;
			if (numberOfTry > 120) numberOfTry = 120;
			Serial.print("Can't connecto to MQTT broker:");
			Serial.println(connection->connectErrorString(ret));
			Serial.printf("Next try in %i seconds...\n", numberOfTry * 10);
			connection->disconnect();
			connectInterval = 1000UL * numberOfTry * 10UL;
			lastConnect = millis();
			return false;
		}
		Serial.println("MQTT Connected!");
		connectInterval = 0UL;
		lastConnect = millis();
		serverOnline = true;
		return true;
	}
	return false;
}

void MQTTController::loop()
{
	MQTTProcess * dev;
	//Do scheduled tasks
	dev = (MQTTProcess *)getFirst();
	while (dev != nullptr) {
		dev->schedule();
		dev = (MQTTProcess *)(dev->next);
	}

	//if there is no mqtt connection - exit
	if (connection == nullptr) return;
	Adafruit_MQTT_Subscribe *subscription;
	while ((subscription = connection->readSubscription())) {
		dev = (MQTTProcess *)getFirst();
		while (dev != nullptr) {
			dev->loop(subscription);
			dev = (MQTTProcess *)(dev->next);
		}
	}

}

MQTTProcess * MQTTController::Register(MQTTProcess * device)
{
	Serial.print("Register: ");
	Serial.println(device->name);
	add(device);
	if (connection != nullptr) device->Register(connection);
	Serial.println("OK;");
	return device;
}

void MQTTController::printConfig(JsonString * json)
{
	json->AddValue("broker", String(broker));
	json->AddValue("port", String(port));
	json->AddValue("user", String(user));
	json->AddValue("key", String(key));
}

void MQTTController::saveConfig(String broker, String port, String user, String key)
{
	JsonString ret = "";
	ret.beginObject();
	ret.AddValue("broker", broker);
	ret.AddValue("port", port);
	ret.AddValue("user", user);
	ret.AddValue("password", key);
	ret.endObject();

	String path = "/mqtt.json";
	File file = SPIFFS.open(path, "w");
	file.print(ret);
	file.flush();
	file.close();
}

void MQTTController::loadConfig()
{
	if (SPIFFS.exists(configFile)) {
		Serial.println(F("read ./cfg/mqtt.json"));

		File f = SPIFFS.open(configFile, "r");
		f.seek(0);
		JsonString json = JsonString(f.readString());

		String b = json.getValue("server");
		broker = (char *)calloc(b.length() + 1, 1);
		b.toCharArray(broker, b.length() + 1);
		if (b.length() > 0) {
			String p = json.getValue("port");
			port = p.toInt();

			String u = json.getValue("user");
			user = (char *)calloc(u.length() + 1, 1);
			u.toCharArray(user, u.length() + 1);

			String k = json.getValue("password");
			key = (char *)calloc(k.length() + 1, 1);
			k.toCharArray(key, k.length());

			Serial.print("broker:"); Serial.println(broker);
			Serial.print("port:"); Serial.println(port);
			Serial.print("user:"); Serial.println(user);
			Serial.print("key:"); Serial.println(key);

			Serial.println("OK!");
		}
		f.close();
	}
}


MQTTController mqttController;
