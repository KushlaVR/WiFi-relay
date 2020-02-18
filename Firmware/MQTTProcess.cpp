#include "MQTTProcess.h"


MQTTProcess::MQTTProcess()
{

}


MQTTProcess::~MQTTProcess()
{

}

void MQTTProcess::printInfo(JsonString * ret)
{
	ret->AddValue("name", name);
	ret->AddValue("type", type);
}

