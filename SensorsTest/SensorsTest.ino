/*
 Name:		SensorsTest.ino
 Created:	3/25/2020 10:24:25 PM
 Author:	Віталік
*/

#include <Wire.h>
#include "Adafruit_Sensor.h"
#include <Adafruit_AM2320.h>

#define pinSDA D2
#define pinSCL D1
//#define AM2320 0xB8

TwoWire i2c_bus = TwoWire();
Adafruit_AM2320 sensor = Adafruit_AM2320(&i2c_bus);

// the setup function runs once when you press reset or power the board
void setup() {
	Serial.begin(115200);
	i2c_bus.begin(pinSDA, pinSCL);
	sensor.begin();
}

ulong lastRead = 0;

// the loop function runs over and over again until power down or reset
void loop() {
	if (lastRead == 0 || (millis() - lastRead) > 2000) {
		float h = sensor.readHumidity();
		float t = sensor.readTemperature();
		Serial.printf("T = %f, H = %f\n", t, h);
		lastRead = millis();
	}
}
