#include "Sensor.h"

static Sensor * _firstSensor = nullptr;
static Sensor * _lastSensor = nullptr;
static Sensor * _currentSensor = nullptr;

Sensor::Sensor()
{
}


Sensor::~Sensor()
{
}

void Sensor::Register()
{
	uid = generateNewUid();
	Serial.print("Registering Sensor: ");
	if (_firstSensor == nullptr) {
		//Serial.println("register 2");
		_firstSensor = this;
		_firstSensor->next = nullptr;
		_firstSensor = this;
	}
	else {
		Sensor * t = _firstSensor;
		while (t != nullptr) {
			if (t->next == nullptr) {
				t->next = this;
				this->next = nullptr;
				t = nullptr;
				_lastSensor = this;
			}
			else
			{
				t = t->next;
			}
		}
	}
	Serial.printf("Sensor %i - %s registered\n", uid, type);
}

bool Sensor::loop(unsigned long m)
{
}

void Sensor::processNext()
{
	unsigned long m = millis();
	if (_currentSensor == nullptr) { _currentSensor = _firstSensor; }
	if (_currentSensor != nullptr) {
		if (_currentSensor->loop(m))
			_currentSensor = _currentSensor->next;
	}
}

int Sensor::generateNewUid()
{
	int ret = 1;
	Sensor * t = _firstSensor;
	while (t != nullptr) {
		if (t->uid == ret) {
			ret++;
			t = _firstSensor;
		}
		else {
			t = t->next;
		}
	}
	return ret;
}


OneWire _ds18x20(D2);
//OneWire _ds18x20(D0);


DS18X20::DS18X20()
{

}

bool DS18X20::loop(unsigned long m)
{
	if (stage == 0) {
		if ((m - lastLoop) < interval) return false;
		lastLoop = m;

		_ds18x20.reset();
		_ds18x20.select(addr);
		_ds18x20.write(0x44, 1);//start conversion, with parasite power on at the end
		stage = 1;
	}
	else if (stage == 1) {
		if ((m - lastLoop) < 1000) return false;
		Serial.printf("DS18X20 %i:\n", uid);

		uint8_t present = _ds18x20.reset();
		_ds18x20.select(addr);
		_ds18x20.write(0xBE);         // Read Scratchpad

		Serial.print("  Data = ");
		Serial.print(present, HEX);
		Serial.print(" ");
		int i;
		for (i = 0; i < 9; i++) {           // we need 9 bytes
			data[i] = _ds18x20.read();
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
		Variable::setValue(variable, celsius);
		stage = 0;
	}
	return true;
}

bool DS18X20::findAll()
{
	Serial.println("Start finding DS18X20 temperature sonsors...");
	byte addr[8];
	bool present = false;
	while (_ds18x20.search(addr)) {
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
			DS18X20 * sensor = new DS18X20();

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
				sensor->Register();
				sensor->variable = String("t") + String(sensor->uid);
			}
		}
		_ds18x20.reset();
	}
	return present;
}

AsyncDHT * dht;

bool DHT_22::loop(unsigned long m)
{
	dht->readAsync();
	if (lastLoop != 0 && (m - lastLoop) < interval) return false;
	if (dht->isReady()) {
		lastLoop = m;
		float t = (dht->getTemperature(false) * termoCompesation) + termoTranslation;
		float h = dht->getHunidity();
		if (t != this->t || h != this->h) {
			this->t = t;
			this->h = h;

			Serial.print("DHT [T = ");
			Serial.print(t);
			Serial.print("; H = ");
			Serial.print(h);
			Serial.println("%]");

			Variable::setValue(String("t") + String(uid), t);
			Variable::setValue(String("h") + String(uid), h);
		}

		return true;
	}
	return false;
}

bool DHT_22::findAll()
{
	dht = new AsyncDHT();
	dht->begin(D2, DHT22);

	DHT_22 * sensor = new DHT_22();
	sensor->type = "DHT22";
	sensor->Register();
	sensor->interval = 10000;

	return true;
}
