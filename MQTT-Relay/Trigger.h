#pragma once
#include <Time.h>

class Trigger
{
private:
	static int _uid;


public:
	Trigger();
	~Trigger();

	int uid;
	char * type = nullptr;
	char * _tempate = nullptr;
	char * _editingTempate = nullptr;
	char * name = nullptr;
	char * days = nullptr;

	virtual void loop(time_t * time) {};

};

class OnOffTrigger : Trigger {

public:
	OnOffTrigger();
	~OnOffTrigger();


};

