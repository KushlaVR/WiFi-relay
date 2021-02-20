#include "Sensor.h"

Sensor::Sensor()
{
}

Sensor::~Sensor()
{
}

void Sensor::loop()
{
}

Sensor* Sensors::get(int index)
{
	return (Sensor*)(((Collection*)this)->get(index));
}

void Sensors::setup(String configDir)
{
}

void Sensors::loop()
{
	Sensor* sensor = (Sensor*)getFirst();
	while (sensor != nullptr) {
		if (sensor->lastRead == 0 || (millis() - sensor->lastRead) > sensor->interval)
			sensor->loop();
		sensor = (Sensor*)(sensor->next);
	}
}

DHT_22::DHT_22(uint8_t pin, uint8_t type)
{
	this->type = "dht";
	dht = new AsyncDHT();
	dht->begin(pin, type);
	for (int i = 0; i < MAX_BUFFER_LEN; i++) {
		temp[i] = 0;
		hum[i] = 0;
	}
}

void DHT_22::loop()
{
	dht->read();
	if (dht->isReady()) {
		lastRead = millis();
		float t = (dht->getTemperature(false) * termoCompesation) + termoTranslation;
		float h = dht->getHunidity();

		if (isnan(t)) return;
		if (isnan(h)) return;
		if (t < -80 || t > 200) return;
		if (h < 0 || h > 100) return;

		temp[bufferIndex] = t;
		hum[bufferIndex] = h;

		bufferIndex++;
		if (bufferIndex == MAX_BUFFER_LEN) bufferIndex = 0;
		if (bufferLength < MAX_BUFFER_LEN) bufferLength++;

		t = 0;
		h = 0;

		for (int i = 0; i < MAX_BUFFER_LEN; i++) {
			t += temp[i];
			h += hum[i];
		}

		t = t / bufferLength;
		h = h / bufferLength;

		if (t != this->t || h != this->h) {
			this->t = t;
			this->h = h;

			Serial.print("DHT [T = ");
			Serial.print(t);
			Serial.print("; H = ");
			Serial.print(h);
			Serial.println("%]");

			Variable::setValue(String("t") + String(Index + 1), t);
			Variable::setValue(String("h") + String(Index + 1), h);
		}
		return;
	}
	else {
		//Serial.println("DHT not ready");
	}
}

Sensors sensors;

void buttonPressed(void* arg) {
	ButtonSensor* btn = (ButtonSensor*)arg;
	btn->pressed();
}

ButtonSensor::ButtonSensor(uint8_t pin)
{
	this->type = "btn";
	btn = new Button(pin, buttonPressed);
	btn->argument = this;
	pinMode(pin, INPUT);
}

ButtonSensor::ButtonSensor(uint8_t pin, int pressedState) :ButtonSensor(pin)
{
	this->btn->condition = pressedState;
}

void ButtonSensor::loop()
{
	btn->handle();
}

void ButtonSensor::pressed()
{
	Variable::setValue(String("btn") + String(btn->pin), btn->state);
}

DS18X20::DS18X20(OneWire* wire)
{
	type = "ds18x20";
	this->wire = wire;
}

void DS18X20::loop()
{
	if (stage == 0) {
		Start_ms = millis();

		wire->reset();
		wire->select(addr);
		wire->write(0x44, 1);//start conversion, with parasite power on at the end
		stage = 1;
	}
	else if (stage == 1) {
		if ((millis() - Start_ms) < 1000) return;

		Serial.printf("DS18X20 %i:\n", Index);

		uint8_t present = wire->reset();
		wire->select(addr);
		wire->write(0xBE);         // Read Scratchpad

		Serial.print("  Data = ");
		Serial.print(present, HEX);
		Serial.print(" ");
		int i;
		for (i = 0; i < 9; i++) {           // we need 9 bytes
			data[i] = wire->read();
			Serial.print(data[i], HEX);
			Serial.print(" ");
		}
		Serial.print(" CRC=");
		Serial.print(OneWire::crc8(data, 8), HEX);
		Serial.println();


		// Convert the data to actual temperature
		// because the result is a 16 bit signed integer, it should
		// be stored to an "int16_t" type, which is always 16 bits
		// even when compiled on a 32 bit processor.
		int16_t raw = (data[1] << 8) | data[0];
		if (type_s) {
			raw = raw << 3; // 9 bit resolution default
			if (data[7] == 0x10) {
				// "count remain" gives full 12 bit resolution
				raw = (raw & 0xFFF0) + 12 - data[6];
			}
		}
		else {
			byte cfg = (data[4] & 0x60);
			// at lower res, the low bits are undefined, so let's zero them
			if (cfg == 0x00) raw = raw & ~7;  // 9 bit resolution, 93.75 ms
			else if (cfg == 0x20) raw = raw & ~3; // 10 bit res, 187.5 ms
			else if (cfg == 0x40) raw = raw & ~1; // 11 bit res, 375 ms
			//// default is 12 bit resolution, 750 ms conversion time
		}

		celsius = (float)raw / 16.0;
		//fahrenheit = celsius * 1.8 + 32.0;
		Serial.print("  Temperature = ");
		Serial.println(celsius);
		Variable::setValue(String("t") + String(Index + 1), celsius);
		stage = 0;
		lastRead = millis();
	}
}

bool DS18X20::findAll(OneWire* wire)
{
	Serial.println("Start finding DS18X20 temperature sonsors...");
	byte addr[8];
	bool present = false;
	while (wire->search(addr)) {
		//знайдено новий пристрій...
		if (OneWire::crc8(addr, 7) != addr[7]) {
			Serial.println("DS18X20: Found device, but CRC is not valid!");
		}
		else {
			present = true;
			Serial.print("DS18X20: ROM =");
			int i;
			for (i = 0; i < 8; i++) {
				Serial.write(' ');
				Serial.print(addr[i], HEX);
			}
			DS18X20* sensor = new DS18X20(wire);

			switch (addr[0]) {
			case 0x10:
				//Serial.println("  Chip = DS18S20");  // or old DS1820
				sensor->type = "DS18S20";
				sensor->type_s = true;
				break;
			case 0x28:
				//Serial.println("  Chip = DS18B20");
				sensor->type = "DS18B20";
				break;
			case 0x22:
				//Serial.println("  Chip = DS1822");
				sensor->type = "DS1822";
				break;
			default:
				Serial.println("Device is not a DS18x20 family device.");
			}
			if (sensor->type != nullptr) {
				for (i = 0; i < 8; i++) {
					sensor->addr[i] = addr[i];
				}
				sensors.add(sensor);
				//sensor->variable = String("t") + String(sensor->index);
			}
		}
		wire->reset();
	}
	return present;
}
