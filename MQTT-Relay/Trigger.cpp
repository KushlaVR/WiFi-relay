#include "Trigger.h"



Trigger::Trigger()
{
	Trigger::_uid++;
	uid = _uid;
}


Trigger::~Trigger()
{
}

OnOffTrigger::OnOffTrigger():Trigger()
{
	type = "onoff";
	_tempate = "onoff";
	_editingTempate = "onoffedit";
}

OnOffTrigger::~OnOffTrigger()
{
}
