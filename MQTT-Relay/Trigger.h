#pragma once
#include <FS.h>
#include <Time.h>
#include "MQTTprocess.h"
#include "MQTTswitch.h"
#include "Json.h"


class Trigger
{
private:
	int sort;

	Trigger * next;
public:
	Trigger();
	~Trigger();

	void Register();
	void Unregister();

	MQTTswitch * proc;

	int uid;
	char * type = nullptr;
	String name = "";
	unsigned char days = 0xFF;

	virtual void loop(time_t * time) {};
	virtual void load(File * f) {};
	virtual void printInfo(JsonString * ret, bool detailed);
	virtual int getSort() { return sort; };
	Trigger * getNextTrigger() { return next; };
	bool save();

	static Trigger * getFirstTrigger();
	static Trigger * getLastTrigger();

	static void processNext(time_t * time);
	static void loadConfig(MQTTswitch * proc);
	static int generateNewUid();
};

class OnOffTrigger : public Trigger {
	time_t lastFire = 0;
public:

	OnOffTrigger();
	~OnOffTrigger();

	unsigned int time;
	unsigned char action = LOW;

	void loop(time_t * time);
	void load(File * f);
	void printInfo(JsonString * ret, bool detailed);
	int getSort() { return time; };
};

class PWMTrigger : public Trigger {
	unsigned char stage = 0;
	time_t lastFire = 0;

public:

	PWMTrigger();
	~PWMTrigger();

	unsigned long onlength = 0;
	unsigned long offlength = 0;

	void loop(time_t * time);
	void load(File * f);
	void printInfo(JsonString * ret, bool detailed);

};


class Termostat : public Trigger {

	bool state = false;

public:
	Termostat();
	~Termostat() {};

	unsigned long start = 0;
	unsigned long end = 0;

	String variable = "t1";

	int min = 0;
	int max = 0;

	void printInfo(JsonString * ret, bool detailed);
	void load(File * f);
	void loop(time_t * time);

};

class Venting : public Trigger {

	bool state = false;

public:
	Venting();
	~Venting() {};

	unsigned long start = 0;
	unsigned long end = 0;

	String variable = "h1";

	int min = 0;
	int max = 0;

	void printInfo(JsonString * ret, bool detailed);
	void load(File * f);
	void loop(time_t * time);

};