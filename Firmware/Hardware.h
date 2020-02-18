#pragma once
#include <FS.h>
#include <OneWire.h>
#include "Collection.h"
#include "Json.h"

class Hardware
{
private:
	int pinLevel(String value);
	int pinNumber(String pinName);


public:
	Hardware() {};
	~Hardware() {};

	void begin();


};

