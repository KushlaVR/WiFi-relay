#include "SerialController.h"



SerialController::SerialController()
{
}


SerialController::~SerialController()
{
}

void SerialController::loop()
{
	if (Serial.available()) {
		cmd[cmdPos] = Serial.read();
		if (cmd[cmdPos] == 10 || cmd[cmdPos] == 13) {
			if (cmdPos > 0) {
				cmd[cmdPos + 1] = 0;
				String s = String(cmd);
				if (s.startsWith("time=")) {
					cmdTime(s.substring(5));
				}
			}
			cmdPos = 0;
		}
		else if (cmdPos == 255) {
			cmdPos = 0;
		}
		else {
			cmdPos++;
		}
	}
}

void SerialController::cmdTime(String cmd)
{
	//2018.01.13 10:24:55
	setTime(
		cmd.substring(11, 13).toInt(),
		cmd.substring(14, 16).toInt(),
		cmd.substring(17, 19).toInt(),
		cmd.substring(8, 10).toInt(),
		cmd.substring(5, 7).toInt(),
		cmd.substring(0, 4).toInt()
	);


	time_t t = now();
	Serial.printf("now=%s s%\n", Utils::FormatDate(t).c_str(), Utils::FormatTime(t).c_str());
	Serial.printf("weekDay=%i, [%i]\n", weekday(t), (1 << (weekday(t) - 1)));

}
