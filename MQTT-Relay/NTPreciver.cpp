#include <TimeLib.h>
#include "NTPreciver.h"



NTPreciver::NTPreciver()
{
	setSyncInterval(SECS_PER_HOUR);
}


NTPreciver::~NTPreciver()
{
}

void NTPreciver::setup()
{
	udp.begin(localPort);
}

void NTPreciver::loop() {
	if (timeStatus() == timeSet) return;
	
	if (start == 0) {
		WiFi.hostByName(ntpServerName, timeServerIP);
		start = millis();
		sendNTPpacket(timeServerIP); // send an NTP packet to a time server
		Serial.println("NTP: send");
	}
	else {

		if ((millis() - start) > 500) {
			int cb = udp.parsePacket();
			if (cb > 0) {
				udp.read(packetBuffer, NTP_PACKET_SIZE); // read the packet into the buffer
				Serial.println("NTP: recived");

				unsigned long highWord = word(packetBuffer[40], packetBuffer[41]);
				unsigned long lowWord = word(packetBuffer[42], packetBuffer[43]);
				// combine the four bytes (two words) into a long integer
				// this is NTP time (seconds since Jan 1 1900):
				unsigned long secsSince1900 = highWord << 16 | lowWord;
				Serial.print("Seconds since Jan 1 1900 = ");
				Serial.println(secsSince1900);
				time_t t = secsSince1900 - 2208988800UL + timeZone * SECS_PER_HOUR;
				setTime(t);
				start = 0;
			}
			else if ((millis() - start) > 6000) {
				//timeout
				Serial.println("NTP: timeout");
				start = 0;
			}
		}
	}
};

// send an NTP request to the time server at the given address
void NTPreciver::sendNTPpacket(IPAddress& address) {
	Serial.println("sending NTP packet...");
	// set all bytes in the buffer to 0
	memset(packetBuffer, 0, NTP_PACKET_SIZE);
	// Initialize values needed to form NTP request
	// (see URL above for details on the packets)
	packetBuffer[0] = 0b11100011;   // LI, Version, Mode
	packetBuffer[1] = 0;     // Stratum, or type of clock
	packetBuffer[2] = 6;     // Polling Interval
	packetBuffer[3] = 0xEC;  // Peer Clock Precision
	// 8 bytes of zero for Root Delay & Root Dispersion
	packetBuffer[12] = 49;
	packetBuffer[13] = 0x4E;
	packetBuffer[14] = 49;
	packetBuffer[15] = 52;

	// all NTP fields have been given values, now
	// you can send a packet requesting a timestamp:
	udp.beginPacket(address, 123); //NTP requests are to port 123
	udp.write(packetBuffer, NTP_PACKET_SIZE);
	udp.endPacket();
}