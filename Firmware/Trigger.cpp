#include "Trigger.h"

static Collection triggers = Collection();
static int _uid;


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
	triggers.add(this);
	if (_uid <= uid) _uid = uid;
	Serial.printf("Trigger %i - %s registered\n", uid, type);
}

void Trigger::Unregister()
{
	triggers.remove(this);
	//if (_currentProcesing == this) _currentProcesing = this->getNextTrigger();
}

void Trigger::DeleteConfig()
{
	String fileName = getConfigFileName();
	if (SPIFFS.exists(fileName)) SPIFFS.remove(fileName);
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

void Trigger::fillFromWebServer(ESP8266WebServer * server)
{
	if (server->hasArg("name")) {
		this->name = server->arg("name");
		Serial.printf("name=%s\n", this->name.c_str());
	}

	if (server->hasArg("days")) {
		this->days = server->arg("days").toInt();
		Serial.printf("days=%i\n", this->days);
	}

}

String Trigger::getConfigFileName()
{
	String num = String(uid);
	if (num.length() < 2) num = "0" + num;
	return "/cfg/" + proc->getName() + "/" + num + String(type) + ".json";
}

bool Trigger::save()
{
	Serial.println("Trigger save...");
	String fileName = getConfigFileName();
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
	return (Trigger *)(triggers.getFirst());
}

Trigger * Trigger::getLastTrigger()
{
	return (Trigger *)(triggers.getLast());
}

void Trigger::processNext(time_t * time)
{
	if (timeStatus() == timeNotSet) return;
	Trigger * _currentProcesing = Trigger::getFirstTrigger();
	while (_currentProcesing != nullptr) {
		_currentProcesing->loop(time);
		_currentProcesing = _currentProcesing->getNextTrigger();
	}
}

void Trigger::loadConfig(Output * proc)
{
	Serial.printf("Loding %s config\n", proc->getName().c_str());
	String dirName = "/cfg/" + proc->getName();
	Dir dir = SPIFFS.openDir(dirName);
	Trigger * t = nullptr;

	while (dir.next()) {
		Serial.println(dir.fileName());
		if (dir.fileSize()) {
			File f = dir.openFile("r");
			String fName = String(f.name()).substring(dirName.length());
			String fNum = fName.substring(1, 3);
			fName = fName.substring(3);
			if (fName.length() > 5) {//5 = ".json" string length
				fName = fName.substring(0, fName.length() - 5);
				Serial.printf("config found num=%s name=%s \n", fNum.c_str(), fName.c_str());
				t = CreateTriggerByType(fName);
				if (t != nullptr) {
					t->uid = fNum.toInt();
					t->load(&f);
					t->proc = proc;
					t->Register();
				}
			}
			f.close();
		}
	}
}

int Trigger::generateNewUid()
{
	int ret = 1;
	int maxUid = 0;
	Trigger * t = Trigger::getFirstTrigger();
	while (t != nullptr) {
		if (t->uid > maxUid) maxUid = t->uid;
		if (t->uid == ret) {
			ret++;
			t = Trigger::getFirstTrigger();
		}
		else {
			t = t->getNextTrigger();
		}
	}
	if (_uid != maxUid) {
		Serial.printf("Max UID reseed %i => %i", _uid, maxUid);
		_uid = maxUid;
	}
	return ret;
}

void Trigger::Sort()
{
	Serial.println("Sorting triggers...");
	//На диску елементи можуть бути роміщені у довільному порядку
	//Після зчитування їх треба посортувати (наприклад бульбашковим методом)
	triggers.sort();

	Serial.println("serted");
	Trigger * t = (Trigger *)(triggers.getFirst());
	while (t != nullptr) {
		Serial.println(t->proc->getName());
		t = (Trigger *)(t->next);
	}
	Serial.println("---");

}

int Trigger::Compare(const void * a, const void * b)
{
	Trigger * ta = (Trigger *)a;
	Trigger * tb = (Trigger *)b;

	int ai = ta->getSort();
	int bi = tb->getSort();
	if (ai > bi)
		return 1;
	else if (ai < bi)
		return -1;
	return 0;
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
	
	int d = weekday(*time) - 1;
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

void OnOffTrigger::fillFromWebServer(ESP8266WebServer * server)
{
	Trigger::fillFromWebServer(server);
	if (server->hasArg("time")) {
		this->time = server->arg("time").toInt();
		Serial.printf("time=%i\n", this->time);
	}

	String action = "";
	if (server->hasArg("action")) {
		action = server->arg("action");
		if (action == "true")
			this->action = HIGH;
		else
			this->action = LOW;
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

	int d = weekday(*time) - 1;
	if (days & (1 << d)) {//Дань тиждня підходящий
		unsigned long deltaT = *time - lastFire;
		unsigned int t = (unsigned int)hour(deltaT) * 60UL + (unsigned int)minute(deltaT);
		//Serial.println(t);
		if (t < onlength) {
			if (stage == 0) {
				stage++;
				if (!proc->isOn()) {
					Serial.printf("PWMTrigger Trigger %i - %s ON\n", uid, type);
					proc->setState(true);
				}
			}
		}
		else if (t < (onlength + offlength))
		{
			if (stage == 1) {
				stage++;
				if (proc->isOn()) {
					Serial.printf("PWMTrigger Trigger %i - %s OFF\n", uid, type);
					proc->setState(false);
				}
			}
		}
		else {
			Serial.printf("PWMTrigger Trigger %i - %s new Period\n", uid, type);
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

void PWMTrigger::fillFromWebServer(ESP8266WebServer * server)
{
	Trigger::fillFromWebServer(server);

	if (server->hasArg("onlength")) {
		this->onlength = server->arg("onlength").toInt();
		Serial.printf("onlength=%i\n", this->onlength);
	}

	if (server->hasArg("offlength")) {
		this->offlength = server->arg("offlength").toInt();
		Serial.printf("offlength=%i\n", this->offlength);
	}
}

Termostat::Termostat()
{
	type = "termo";
}

void Termostat::printInfo(JsonString * ret, bool detailed)
{
	Trigger::printInfo(ret, detailed);
	ret->AddValue("start", String(start));
	ret->AddValue("end", String(end));
	ret->AddValue("variable", variable);
	ret->AddValue("min", String(min));
	ret->AddValue("max", String(max));
}

void Termostat::load(File * f)
{
	Trigger::load(f);
	JsonString s = JsonString(f->readString());
	name = s.getValue("name");
	days = (unsigned char)(s.getValue("days").toInt());
	start = s.getValue("start").toInt();
	end = s.getValue("end").toInt();
	variable = s.getValue("variable");
	min = s.getValue("min").toInt();
	max = s.getValue("max").toInt();
}

void Termostat::loop(time_t * time)
{
	int d = weekday(*time) - 1;
	if (days == 127 || days & (1 << d)) {//Дань тиждня підходящий
		unsigned int t = (unsigned int)hour(*time) * 60UL + (unsigned int)minute(*time);
		if ((start == end/*цілий день*/) || (t >= start && t <= end)) {//час підходящий
			float v = Variable::getValue(variable);
			if (v < min) {//температура впала нижче мінімума
				if (!proc->isOn()) {
					Serial.printf("Termostat Trigger %i - %s Temperature too low => ON\n", uid, type);
					proc->setState(true);
					state = true;
				}
			}
			else {
				if (v > max) {//температура вище максимума
					if (proc->isOn()) {
						Serial.printf("Termostat Trigger %i - %s Temperature too high => OFF\n", uid, type);
						proc->setState(false);
						state = false;
					}
				}
			}
		}
		else if (start > end) { //Початок більше кінця. Наприклад з 22:00 до 05:00
			if (t >= start || t <= end) {
				float v = Variable::getValue(variable);
				if (v < min) {//температура впала нижче мінімума
					if (!proc->isOn()) {
						Serial.printf("Termostat Trigger %i - %s Temperature too low => ON\n", uid, type);
						proc->setState(true);
						state = true;
					}
				}
				else {
					if (v > max) {//температура вище максимума
						if (proc->isOn()) {
							Serial.printf("Termostat Trigger %i - %s Temperature too high => OFF\n", uid, type);
							proc->setState(false);
							state = false;
						}
					}
				}
			}
		}
		else {
			if (state) {
				if (proc->isOn()) {
					Serial.printf("Termostat Trigger %i - %s OFF\n", uid, type);
					proc->setState(false);
					state = false;
				}
			}
		}
	}
}

void Termostat::fillFromWebServer(ESP8266WebServer * server)
{
	Trigger::fillFromWebServer(server);

	if (server->hasArg("start")) {
		this->start = server->arg("start").toInt();
		Serial.printf("start=%i\n", this->start);
	}

	if (server->hasArg("end")) {
		this->end = server->arg("end").toInt();
		Serial.printf("end=%i\n", this->end);
	}

	if (server->hasArg("variable")) {
		this->variable = server->arg("variable");
		Serial.printf("variable=%s\n", this->variable.c_str());
	}

	if (server->hasArg("min")) {
		this->min = server->arg("min").toInt();
		Serial.printf("min=%i\n", this->min);
	}

	if (server->hasArg("max")) {
		this->max = server->arg("max").toInt();
		Serial.printf("max=%i\n", this->max);
	}
}

Venting::Venting()
{
	type = "vent";
}

void Venting::printInfo(JsonString * ret, bool detailed)
{
	Trigger::printInfo(ret, detailed);
	ret->AddValue("start", String(start));
	ret->AddValue("end", String(end));
	ret->AddValue("variable", variable);
	ret->AddValue("min", String(min));
	ret->AddValue("max", String(max));
}

void Venting::load(File * f)
{
	Trigger::load(f);
	JsonString s = JsonString(f->readString());
	name = s.getValue("name");
	days = (unsigned char)(s.getValue("days").toInt());
	start = s.getValue("start").toInt();
	end = s.getValue("end").toInt();
	variable = s.getValue("variable");
	min = s.getValue("min").toInt();
	max = s.getValue("max").toInt();
}

void Venting::loop(time_t * time)
{
	int d = weekday(*time) - 1;
	if (days == 127 || days & (1 << d)) {//Дань тиждня підходящий
		unsigned int t = (unsigned int)hour(*time) * 60UL + (unsigned int)minute(*time);
		if ((start == end/*цілий день*/) || (t >= start && t <= end)) {//час підходящий
			float v = Variable::getValue(variable);
			if (v < min) {//Вологість впала нижче мінімума
				if (state/*proc->isOn()*/) {
					Serial.printf("Venting Trigger %i - %s Humidity too low => OFF\n", uid, type);
					proc->setState(false);
					state = false;
				}
			}
			else {
				if (v > max) {//Вологість вище максимума
					if (!state/*!proc->isOn()*/) {
						Serial.printf("Venting Trigger %i - %s Humidity too high => ON\n", uid, type);
						proc->setState(true);
						state = true;
					}
				}
			}
		}
		else if (start > end) { //Початок більше кінця. Наприклад з 22:00 до 05:00
			if (t >= start || t <= end) {
				float v = Variable::getValue(variable);
				if (v < min) {//Вологість впала нижче мінімума
					if (state/*proc->isOn()*/) {
						Serial.printf("Venting Trigger %i - %s Humidity too low => OFF\n", uid, type);
						proc->setState(false);
						state = false;
					}
				}
				else {
					if (v > max) {//Вологість вище максимума
						if (!state/*!proc->isOn()*/) {
							Serial.printf("Trigger %i - %s Temperature too high => ON\n", uid, type);
							proc->setState(true);
							state = true;
						}
					}
				}
			}
		}
		/*else {
			if (state) {//TODO: ??? Що автор хотів им сказати ???
				if (proc->isOn()) {
					Serial.printf("Trigger %i - %s OFF\n", uid, type);
					proc->setState(false);
					state = false;
				}
			}
		}*/
	}
}

void Venting::fillFromWebServer(ESP8266WebServer * server)
{
	Trigger::fillFromWebServer(server);
	if (server->hasArg("start")) {
		this->start = server->arg("start").toInt();
		Serial.printf("start=%i\n", this->start);
	}

	if (server->hasArg("end")) {
		this->end = server->arg("end").toInt();
		Serial.printf("end=%i\n", this->end);
	}

	if (server->hasArg("variable")) {
		this->variable = server->arg("variable");
		Serial.printf("variable=%s\n", this->variable.c_str());
	}

	if (server->hasArg("min")) {
		this->min = server->arg("min").toInt();
		Serial.printf("min=%i\n", this->min);
	}

	if (server->hasArg("max")) {
		this->max = server->arg("max").toInt();
		Serial.printf("max=%i\n", this->max);
	}
}

TimeoutTrigger::TimeoutTrigger()
{
	type = "timeout";
}

void TimeoutTrigger::loop(time_t * time)
{
	//Serial.printf("Loop index=%i, %s\n", this->Index, name.c_str());
	int d = weekday(*time) - 1;
	if (days & (1 << d)) {//Дань тиждня підходящий
		if (proc != nullptr) {
			if (action == LOW && proc->isOn() && startTime == 0) {//якщо включено і необхідно вимкнути
				Serial.printf("Timeout trigger %i - %s Timer start\n", uid, type);
				startTime = millis();
			}
			else if (action == HIGH && !proc->isOn() && startTime == 0) {//якщо вимкнено і необхідно включити
				Serial.printf("Timeout trigger %i - %s Timer start\n", uid, type);
				startTime = millis();
			}
			else if (startTime > 0) {//Якщо таймер заведено
				if ((millis() - startTime) > (60000UL * len)) {
					proc->setState(action);
					Serial.printf("Timeout trigger %i - %s Fire\n", uid, type);
					startTime = 0;
				}
				else if (action == HIGH && proc->isOn()) {
					Serial.printf("Timeout trigger %i - %s Timer stopped\n", uid, type);
					startTime = 0;
				}
				else if (action == LOW && !proc->isOn()) {
					Serial.printf("Timeout trigger %i - %s Timer stopped\n", uid, type);
					startTime = 0;
				}
			}
		}
	}
}

void TimeoutTrigger::load(File * f)
{
	Trigger::load(f);
	JsonString s = JsonString(f->readString());
	name = s.getValue("name");
	days = (unsigned char)(s.getValue("days").toInt());
	len = s.getValue("len").toInt();
	if (s.getValue("action") == "1") {
		action = HIGH;
	}
	else {
		action = LOW;
	}
}

void TimeoutTrigger::printInfo(JsonString * ret, bool detailed)
{
	Trigger::printInfo(ret, detailed);
	ret->AddValue("len", String(len));
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

void TimeoutTrigger::fillFromWebServer(ESP8266WebServer * server)
{
	Trigger::fillFromWebServer(server);

	if (server->hasArg("len")) {
		this->len = server->arg("len").toInt();
		Serial.printf("len=%i\n", this->len);
	}

	String action = "";
	if (server->hasArg("action")) {
		action = server->arg("action");
		if (action == "true")
			this->action = HIGH;
		else
			this->action = LOW;
	}
}

Trigger * CreateTriggerByType(String type)
{
	Trigger * trigger = nullptr;
	if (type == "onoff")
		trigger = new OnOffTrigger();
	else if (type == "pwm")
		trigger = new PWMTrigger();
	else if (type == "termo")
		trigger = new Termostat();
	else if (type == "vent")
		trigger = new Venting();
	else if (type == "timeout")
		trigger = new TimeoutTrigger();
	else
		return false;
	trigger->uid = Trigger::generateNewUid();

	return trigger;
}
