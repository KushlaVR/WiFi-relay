#include "Trigger.h"
#include "MQTTprocess.h"
#include "MQTTswitch.h"
#include <TimeLib.h>
#include <FS.h>
#include "Json.h"

static int _uid;
static Trigger * _firstTrigger = nullptr;
static Trigger * _lastTrigger = nullptr;
static Trigger * _currentProcesing = nullptr;

Trigger::Trigger()
{
	_uid++;
	uid = _uid;
}

Trigger::~Trigger()
{
}

void Trigger::Register()
{
	//Serial.println("register 1");
	if (_firstTrigger == nullptr) {
		//Serial.println("register 2");
		_firstTrigger = this;
		_firstTrigger->next = nullptr;
		_lastTrigger = this;
	}
	else {
		//Serial.println("register 3");
		Trigger * t = _firstTrigger;
		while (t != nullptr) {
			//Serial.println("register 4");
			if (t->next == nullptr) {
				//Serial.println("register 5");
				t->next = this;
				this->next = nullptr;
				t = nullptr;
				_lastTrigger = this;
			}
			else
			{
				//Serial.println("register 6");
				t = t->next;
			}
		}
	}
	if (_uid <= uid) _uid = uid;

	Serial.printf("Trigger %i - %s registered\n", uid, type);
}

void Trigger::Unregister()
{
	Trigger * t = _firstTrigger;
	Trigger * prev = nullptr;
	while (t != nullptr) {
		if (t == this) {
			if (prev == nullptr) {
				_firstTrigger = this->next;
			}
			else {
				prev->next = this->next;
			}
			t = nullptr;
		}
		else {
			prev = t;
			t = t->next;
		}
	}
	if (_currentProcesing == this) _currentProcesing = this->next;
}

void Trigger::printInfo(JsonString * ret, bool detailed)
{
	ret->AddValue("name", name);
	ret->AddValue("uid", String(uid));
	ret->AddValue("days", String(days));
	if (detailed) {
		ret->AddValue("type", type);
		ret->AddValue("template", type);
		ret->AddValue("editingtemplate", String(type) + "edit");
	}
}

bool Trigger::save()
{
	Serial.println("Trigger save...");
	String num = String(uid);
	if (num.length() < 2) num = "0" + num;

	String fileName = "/config/" + proc->name + "/" + num + String(type) + ".json";
	Serial.println(fileName.c_str());
	File f = SPIFFS.open(fileName, "w");
	JsonString ret = "";
	printInfo(&ret, false);
	f.print(ret);
	f.flush();
	f.close();
	Serial.println("Saved OK.");
	return true;
}

Trigger * Trigger::getFirstTrigger()
{
	return _firstTrigger;
}

Trigger * Trigger::getLastTrigger()
{
	return _lastTrigger;
}

void Trigger::processNext(time_t * time)
{
	if (timeStatus() == timeNotSet) return;
	if (_currentProcesing == nullptr) { _currentProcesing = _firstTrigger; }
	if (_currentProcesing != nullptr) {
		_currentProcesing->loop(time);
		_currentProcesing = _currentProcesing->next;
	}
}

void Trigger::loadConfig(MQTTswitch * proc)
{
	Serial.printf("Loding %s config\n", proc->name.c_str());
	String dirName = "/config/" + proc->name;
	Dir dir = SPIFFS.openDir(dirName);
	while (dir.next()) {
		Serial.println(dir.fileName());
		if (dir.fileSize()) {
			File f = dir.openFile("r");
			String fName = String(f.name()).substring(dirName.length());
			String fNum = fName.substring(1, 3);
			fName = fName.substring(3);
			Serial.printf("config found num=%s name=%s \n", fNum.c_str(), fName.c_str());

			Trigger * t = nullptr;
			if (fName.startsWith("onoff")) {
				t = new OnOffTrigger();
			}
			else if (fName.startsWith("pwm")) {
				t = new PWMTrigger();
			}
			if (t != nullptr) {
				t->uid = fNum.toInt();
				t->load(&f);
				t->proc = proc;
				t->Register();
			}
		}
	}
}

int Trigger::generateNewUid()
{
	int ret = 1;
	int maxUid = 0;
	Trigger * t = _firstTrigger;
	while (t != nullptr) {
		if (t->uid > maxUid) maxUid = t->uid;
		if (t->uid == ret) {
			ret++;
			t = _firstTrigger;
		}
		else {
			t = t->next;
		}
	}
	if (_uid != maxUid) {
		Serial.printf("Max UID reseed %i => %i", _uid, maxUid);
		_uid = maxUid;
	}
	return ret;
}



OnOffTrigger::OnOffTrigger() :Trigger()
{
	type = "onoff";
}

OnOffTrigger::~OnOffTrigger()
{
}

void OnOffTrigger::loop(time_t * time)
{
	if (year(lastFire) == year(*time) && month(lastFire) == month(*time) && day(lastFire) == day(*time)) return;//Якщо сьогодні вже спрацював, то нічого не робимо

	int d = weekday(*time);
	if (days & (1 << d)) {//Дань тиждня підходящий
		unsigned int t = (unsigned int)hour(*time) * 60UL + (unsigned int)minute(*time);//хвилин від початку доби
		//Serial.printf("t=%i, time=%i\n", t, this->time);
		if (t >= this->time) {
			if (proc != nullptr) {
				proc->setState(action);
			}
			Serial.printf("Trigger %i - %s Fire\n", uid, type);
			lastFire = *time;
		}
	}
}

void OnOffTrigger::load(File * f) {
	Trigger::load(f);
	JsonString s = JsonString(f->readString());
	name = s.getValue("name");
	days = (unsigned char)(s.getValue("days").toInt());
	time = s.getValue("time").toInt();
	if (s.getValue("action") == "1") {
		action = HIGH;
	}
	else {
		action = LOW;
	}
}

void OnOffTrigger::printInfo(JsonString * ret, bool detailed)
{
	Trigger::printInfo(ret, detailed);
	ret->AddValue("time", String(time));
	if (detailed) {
		if (action == HIGH)
			ret->AddValue("action", "on");
		else
			ret->AddValue("action", "off");
	}
	else {
		if (action == HIGH)
			ret->AddValue("action", "1");
		else
			ret->AddValue("action", "0");
	}

}

PWMTrigger::PWMTrigger()
{
	type = "pwm";
	lastFire = 0;
	stage = 0;
}

PWMTrigger::~PWMTrigger()
{
}

void PWMTrigger::loop(time_t * time)
{

	int d = weekday(*time);
	if (days & (1 << d)) {//Дань тиждня підходящий
		unsigned long deltaT = *time - lastFire;
		unsigned int t = (unsigned int)hour(deltaT) * 60UL + (unsigned int)minute(deltaT);
		//Serial.println(t);
		if (t < onlength) {
			if (stage == 0) {
				stage++;
				if (!proc->isOn()) {
					Serial.printf("Trigger %i - %s ON\n", uid, type);
					proc->setState(true);
				}
			}
		}
		else if (t < (onlength + offlength))
		{
			if (stage == 1) {
				stage++;
				if (proc->isOn()) {
					Serial.printf("Trigger %i - %s OFF\n", uid, type);
					proc->setState(false);
				}
			}
		}
		else {
			Serial.printf("Trigger %i - %s new Period\n", uid, type);
			lastFire = *time;
			stage = 0;
		}
	}
}

void PWMTrigger::load(File * f)
{
	Trigger::load(f);
	JsonString s = JsonString(f->readString());
	name = s.getValue("name");
	days = (unsigned char)(s.getValue("days").toInt());
	onlength = s.getValue("onlength").toInt();
	offlength = s.getValue("offlength").toInt();
}

void PWMTrigger::printInfo(JsonString * ret, bool detailed)
{
	Trigger::printInfo(ret, detailed);
	ret->AddValue("onlength", String(onlength));
	ret->AddValue("offlength", String(offlength));
}
