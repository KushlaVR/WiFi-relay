#include "MQTTprocess.h"



MQTTprocess::MQTTprocess()
{
}


MQTTprocess::~MQTTprocess()
{
}

void MQTTprocess::printInfo(JsonString * ret)
{
	ret->AddValue("name", name);
	ret->AddValue("type", type);
}
