#include "Output.h"



Output::Output(String name, int pin, int onState, int offState)
{
	this->name = name;
	this->pin = pin;
	this->onState = onState;
	this->offState = offState;
	_on = false;
	pinMode(pin, OUTPUT);
	digitalWrite(pin, offState);
	Serial.printf("Out pin=%i, name=%s, on=%i, off=%i\n", pin, name.c_str(), onState, offState);
}


Output::~Output()
{
}

void Output::setup(String configDir)
{
	this->configFile = configDir + name + "/start.json";
	if (!SPIFFS.exists(configFile)) {
		Serial.print("Not found "); Serial.println(configFile);
		return;
	}
	Serial.print("Reading config "); Serial.println(configFile);
	File f = SPIFFS.open(configFile, "r");
	JsonString json = JsonString(f.readString());
	setState(json.getValue("state") == "1");
	f.close();
}

void Output::saveStartup(bool state)
{
	JsonString cfg = "";
	cfg.beginObject();
	if (state)
		cfg.AddValue("state", "1");
	else
		cfg.AddValue("state", "0");
	cfg.endObject();
	File f = SPIFFS.open(configFile, "w");
	f.print(cfg);
	f.close();
}

void Output::loop()
{
	//handle some MQTT topic publishing 
}

void Output::setState(bool isOn)
{
	if (_on == isOn) return;
	if (isOn) {
		digitalWrite(pin, onState);
	}
	else {
		digitalWrite(pin, offState);
	}
	_on = isOn;
}

void Output::printInfo(JsonString * json)
{
	json->AddValue("name", name);
	json->AddValue("type", "switch");
	json->AddValue("state", (isOn() ? "ON" : "OFF"));
	json->AddValue("index", String(this->getSort()));
	json->AddValue("visual", "switch");
}

Outputs::Outputs()
{
}

Output * Outputs::get(int index)
{
	return (Output *)(((Collection *)this)->get(index));
}

Output * Outputs::add(String name, int pin, int onState, int offState)
{
	return (Output *)((Collection *)this)->add((Item *)new Output(name, pin, onState, offState));
}

void Outputs::setup(String configDir)
{
	this->configDir = configDir;
	Serial.println("Reading config");
	Output * out = (Output *)getFirst();
	while (out != nullptr) {
		out->setup(this->configDir);
		out = (Output *)(out->next);
	}
}

void Outputs::loop()
{
	Output * out = (Output *)getFirst();
	while (out != nullptr) {
		out->loop();
		out = (Output *)(out->next);
	}
}


Outputs outs;
