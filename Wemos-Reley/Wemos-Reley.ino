/*************************************************************
Download latest Blynk library here:
https://github.com/blynkkk/blynk-library/releases/latest

Blynk is a platform with iOS and Android apps to control
Arduino, Raspberry Pi and the likes over the Internet.
You can easily build graphic interfaces for all your
projects by simply dragging and dropping widgets.

Downloads, docs, tutorials: http://www.blynk.cc
Sketch generator:           http://examples.blynk.cc
Blynk community:            http://community.blynk.cc
Follow us:                  http://www.fb.com/blynkapp
http://twitter.com/blynk_app

Blynk library is licensed under MIT license
This example code is in public domain.

*************************************************************
This example runs directly on ESP8266 chip.

Note: This requires ESP8266 support package:
https://github.com/esp8266/Arduino

Please be sure to select the right ESP8266 module
in the Tools -> Board menu!

Change WiFi ssid, pass, and Blynk auth token to run :)
Feel free to apply it to any other example. It's simple!
*************************************************************/

/* Comment this out to disable prints and save space */
#define BLYNK_PRINT Serial


#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include "Gsender.h"

// You should get Auth Token in the Blynk App.
// Go to the Project Settings (nut icon).
char auth[] = "22ad38531aa442138d0638e6063a4107";

// Your WiFi credentials.
// Set password to "" for open networks.

char ssid[] = "Prosvity19_98";//
char pass[] = "29111929"; //

/*
char ssid[] = "IDS5";
char pass[] = "IDS5IDS5IDS5";
*/


unsigned long notificationTime;


byte OUT1 = LOW;
byte OUT2 = LOW;
byte OUT3 = LOW;

byte notified_OUT1 = LOW;
byte notified_OUT2 = LOW;
byte notified_OUT3 = LOW;

bool sendMail(const char * message);

void setup()
{
	// Debug console
	Serial.begin(115200);
	pinMode(D5, OUTPUT);
	pinMode(D6, OUTPUT);
	pinMode(D7, OUTPUT);
	Blynk.begin(auth, ssid, pass);

	notificationTime = millis();

}

String message = "";

void loop()
{
	Blynk.run();
	if ((millis() - notificationTime) > 60000) {
		if (OUT1 != notified_OUT1) {
			if (OUT1 == HIGH)
			{
				message += "OUT1 ON\n";
				notified_OUT1 = OUT1;
			}
			else
			{
				message += "OUT1 OFF\n";
				notified_OUT1 = OUT1;
			}
		}
		else if (OUT2 != notified_OUT2) {
			if (OUT2 == HIGH)
			{
				message += "OUT2 ON\n";
				notified_OUT2 = OUT2;
			}
			else
			{
				message += "OUT2 OFF\n";
				notified_OUT2 = OUT2;
			}
		}
		else if (OUT3 != notified_OUT3) {
			if (OUT3 == HIGH)
			{
				message += "OUT3 ON\n";
				notified_OUT3 = OUT3;
			}
			else
			{
				message += "OUT3 OFF\n";
				notified_OUT3 = OUT3;
			}
		}
	}
	if (message.length() > 0) {
		sendMail(message.c_str());
		message = "";
	}
}

bool sendMail(const char * message)
{
	Gsender *gsender = Gsender::Instance();    // Getting pointer to class instance
	String subject = "3-way WI-FI Relay!";
	if (gsender->Subject(subject)->Send("kushlavr@gmail.com", message)) {
		Serial.print("Message send.");
		Serial.println(message);
		return true;
	}
	else {
		Serial.print("Error sending message: ");
		Serial.println(gsender->getError());
	}
	return false;
}

// This function will run every time Blynk connection is established
BLYNK_CONNECTED() {
	// Request Blynk server to re-send latest values for all pins
	Blynk.syncAll();

	// You can also update individual virtual pins like this:
	//Blynk.syncVirtual(V0, V2);

	// Let's write your hardware uptime to Virtual Pin 2
	//int value = millis() / 1000;
	//Blynk.virtualWrite(V2, value);
}

BLYNK_WRITE(V1)
{
	if (param.asInt()) {
		//HIGH
		digitalWrite(D5, HIGH);
		OUT1 = HIGH;
	}
	else {
		//LOW
		digitalWrite(D5, LOW);
		OUT1 = LOW;
	}
}

BLYNK_WRITE(V2)
{
	if (param.asInt()) {
		//HIGH
		digitalWrite(D6, HIGH);
		OUT2 = HIGH;
	}
	else {
		//LOW
		digitalWrite(D6, LOW);
		OUT2 = LOW;
	}
}

BLYNK_WRITE(V3)
{
	if (param.asInt()) {
		//HIGH
		digitalWrite(D7, HIGH);
		OUT3 = HIGH;
	}
	else {
		//LOW
		digitalWrite(D7, LOW);
		OUT3 = LOW;
	}
}