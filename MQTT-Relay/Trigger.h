#pragma once
#include <FS.h>
#include <Time.h>
#include "MQTTprocess.h"
#include "MQTTswitch.h"
#include "Json.h"


class Trigger
{
private:

	Trigger * next;
public:
	Trigger();
	~Trigger();

	void Register();
	void Unregister();

	MQTTswitch * proc;

	int uid;
	char * type = nullptr;
	char * _tempate = nullptr;
	char * _editingTempate = nullptr;
	String name = "";
	unsigned char days = 0xFF;

	virtual void loop(time_t * time) {};
	virtual void load(File * f) {};
	virtual void printInfo(JsonString * ret);
	Trigger * getNextTrigger() { return next; };


	static Trigger * getFirstTrigger();
	static void processNext(time_t * time);
	static void loadConfig(MQTTswitch * proc);
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
	void printInfo(JsonString * ret);
};

