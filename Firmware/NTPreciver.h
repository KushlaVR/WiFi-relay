#pragma once

#include <ESP8266WiFi.h>
#include <WiFiUdp.h>

const int NTP_PACKET_SIZE = 48; // NTP time stamp is in the first 48 bytes of the message

class NTPreciver
{
	bool timeIsSet = false;
	ulong start = 0;
	
	unsigned int localPort;// local port to listen for UDP packets
	IPAddress timeServerIP;
	String ntpServerName;
	//int timeZone = 2;

	byte packetBuffer[NTP_PACKET_SIZE]; //buffer to hold incoming and outgoing packets

	// A UDP instance to let us send and receive packets over UDP
	WiFiUDP udp;

public:
	NTPreciver();
	~NTPreciver();

	Timezone * timeZone;

	void setup(String configFile);
	void loop();
	// send an NTP request to the time server at the given address
	void sendNTPpacket(IPAddress& address);
};

