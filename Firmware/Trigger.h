#pragma once
#include <TimeLib.h>
#include "Json.h"
#include "Collection.h"
#include "Output.h"
#include "Variable.h"
#include <ESP8266WebServer.h>

class Trigger: public Item
{

public:
	Trigger();
	~Trigger();

	void Register();
	void Unregister();
	void DeleteConfig();

	Output * proc;

	int uid;
	char * type = nullptr;
	String name = "";
	unsigned char days = 0xFF;

	virtual void loop(time_t * time) {};
	virtual void load(File * f) {};
	virtual void printInfo(JsonString * ret, bool detailed);
	virtual int getSort() { return sort; };
	virtual void fillFromWebServer(ESP8266WebServer * server);
	String getConfigFileName();
	Trigger * getNextTrigger() { return (Trigger *)next; };
	bool save();

	static Trigger * getFirstTrigger();
	static Trigger * getLastTrigger();

	static void processNext(time_t * time);
	static void loadConfig(Output * proc);
	static int generateNewUid();
	static void Sort();
	static int Compare(const void * a, const void * b);
};

class OnOffTrigger : public Trigger {
	time_t lastFire = 0;
public:

	OnOffTrigger();
	~OnOffTrigger();

	unsigned int time;
	unsigned char action = LOW;

	virtual void loop(time_t * time);
	virtual void load(File * f);
	virtual void printInfo(JsonString * ret, bool detailed);
	virtual int getSort() { return time; };
	virtual void fillFromWebServer(ESP8266WebServer * server);
};

class PWMTrigger : public Trigger {
	unsigned char stage = 0;
	time_t lastFire = 0;

public:

	PWMTrigger();
	~PWMTrigger();

	unsigned long onlength = 0;
	unsigned long offlength = 0;

	virtual void loop(time_t * time);
	virtual void load(File * f);
	virtual void printInfo(JsonString * ret, bool detailed);
	virtual void fillFromWebServer(ESP8266WebServer * server);

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

	virtual void printInfo(JsonString * ret, bool detailed);
	virtual void load(File * f);
	virtual void loop(time_t * time);
	virtual void fillFromWebServer(ESP8266WebServer * server);

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

	virtual void printInfo(JsonString * ret, bool detailed);
	virtual void load(File * f);
	virtual void loop(time_t * time);
	virtual void fillFromWebServer(ESP8266WebServer * server);

};


class TimeoutTrigger : public Trigger {
	unsigned long startTime = 0;
public:

	TimeoutTrigger();
	~TimeoutTrigger() {};

	unsigned int len;
	unsigned char action = LOW;

	virtual void loop(time_t * time);
	virtual void load(File * f);
	virtual void printInfo(JsonString * ret, bool detailed);
	virtual void fillFromWebServer(ESP8266WebServer * server);
};


//Trigger factory
Trigger * CreateTriggerByType(String type);
