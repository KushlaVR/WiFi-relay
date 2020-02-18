#include "definitions.h"
#include "NTPreciver.h"



NTPreciver::NTPreciver()
{
	setSyncInterval(SECS_PER_HOUR);
}


NTPreciver::~NTPreciver()
{
}

void NTPreciver::setup(String configFile)
{
	timeIsSet = false;
	String fName = configFile;
	if (!SPIFFS.exists(fName)) {
		Serial.print("NTP config not found "); Serial.println(fName);
		Serial.println("Loaded default values:");
		ntpServerName = "europe.pool.ntp.org";

		TimeChangeRule summerTime = { "ST", Last, Sun, Mar, 3, 180 };        //Ukraine Summer Time -  "UTC + 3" or GMT + 3
		TimeChangeRule standartTime = { "WT", Last, Sun, Oct, 4, 120 };      //Ukraine winter Time -  "UTC + 2" or GMT + 2

		timeZone = new Timezone(summerTime, standartTime);
		localPort = 2390;
	}
	else {
		Serial.print("Reading NTP config "); Serial.println(fName);
		File f = SPIFFS.open(fName, "r");
		JsonString json = JsonString(f.readString());
		ntpServerName = json.getValue("server");
		localPort = json.getValue("port").toInt();

		String s = json.getValue("standart-time");
		int start = 0; int end = s.indexOf(',', start);
		String week = s.substring(start, end);

		start = end + 1; end = s.indexOf(',', start);
		String day = s.substring(start, end);

		start = end + 1; end = s.indexOf(',', start);
		String month = s.substring(start, end);

		start = end + 1; end = s.indexOf(',', start);
		String hour = s.substring(start, end);

		start = end + 1; String offset = s.substring(start);
		TimeChangeRule standartTime = { "WT", Timezone::weekNum(week), Timezone::dayNum(day), Timezone::monthNum(month), hour.toInt(), offset.toInt() * 60 };

		s = json.getValue("summer-time");
		if (s.length() > 0) {
			start = 0;	end = s.indexOf(',', start);
			week = s.substring(start, end);

			start = end + 1; end = s.indexOf(',', start);
			day = s.substring(start, end);

			start = end + 1; end = s.indexOf(',', start);
			month = s.substring(start, end);

			start = end + 1; end = s.indexOf(',', start);
			hour = s.substring(start, end);

			start = end + 1;
			offset = s.substring(start);

			TimeChangeRule summerTime = { "ST", Timezone::weekNum(week), Timezone::dayNum(day), Timezone::monthNum(month), hour.toInt(), offset.toInt() * 60 };
			timeZone = new Timezone(summerTime, standartTime);
		}
		else {
			timeZone = new Timezone(standartTime);
		}

		f.close();
	}

	Serial.printf("    %s:%i\n", ntpServerName.c_str(), localPort);
	udp.begin(localPort);
}

void NTPreciver::loop() {
	if (timeIsSet && timeStatus() == timeSet) return;

	if (start == 0) {
		WiFi.hostByName(ntpServerName.c_str(), timeServerIP);
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
				Serial.print("  Seconds since Jan 1 1900 = ");
				Serial.println(secsSince1900);
				time_t t = secsSince1900 - 2208988800UL;
				setTime(t);
				start = 0;
				timeIsSet = true;
				time_t utc = now();
				Serial.print("   UTC: ");
				Serial.print(Timezone::FormatDate(utc));
				Serial.print("    ");
				Serial.print(Timezone::FormatTime(utc));
				Serial.println();

				time_t local = timeZone->toLocal(utc);
				Serial.print("   Local time: ");
				Serial.print(Timezone::FormatDate(local));
				Serial.print("    ");
				Serial.print(Timezone::FormatTime(local));
				Serial.println();

			}
			else if ((millis() - start) > 6000) {
				//timeout
				Serial.println("    NTP: timeout");
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