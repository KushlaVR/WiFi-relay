#include "Trigger.h"
#include "MQTTprocess.h"
#include "MQTTswitch.h"
#include <TimeLib.h>
#include <FS.h>

static int _uid;
static Trigger * _firstTrigger = nullptr;
static Trigger * _currentProcesing = nullptr;

Trigger::Trigger()
{
	_uid++;
	uid = _uid;
	//Register();
}

Trigger::~Trigger()
{
}

void Trigger::Register()
{
	Serial.println("register 1");
	if (_firstTrigger == nullptr) {
		Serial.println("register 2");
		_firstTrigger = this;
	}
	else {
		Serial.println("register 3");
		Trigger * t = _firstTrigger;
		while (t != nullptr) {
			Serial.println("register 4");
			if (t->next == nullptr) {
				t->next = this;
				t = nullptr;
			}
			else
				t = t->next;
		}
	}
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
	Dir dir = SPIFFS.openDir("/config/" + proc->name);
	while (dir.next()) {
		Serial.println(dir.fileName());
		if (dir.fileSize()) {
			File f = dir.openFile("r");
			String fName = String(f.name());
			if (fName.startsWith("onoff")) {
				OnOffTrigger * t = new OnOffTrigger();
				//t->load(f);
			}
		}
	}

}



OnOffTrigger::OnOffTrigger() :Trigger()
{
	type = "onoff";
	_tempate = "onoff";
	_editingTempate = "onoffedit";
}

OnOffTrigger::~OnOffTrigger()
{
}

void OnOffTrigger::loop(time_t * time)
{
	if (year(lastFire) == year(*time) && month(lastFire) == month(*time) && day(lastFire) == day(*time)) return;//Якщо сьогодні вже спрацював, то нічого не робимо

	int d = weekday(*time);
	if (days & (1 << d)) {//Дань тиждня підходящий
		time_t t = (time_t)hour(*time) * 60UL + (time_t)minute(*time);//Час від початку доби
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
