// Json.h

#ifndef _JSON_h
#define _JSON_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif

class IJson
{
public:
	virtual String toJson(String json) = 0;
};

class JsonString :public String {
public:
	JsonString(const char *cstr = "");
	JsonString(const String &str);
	void appendComa();
	void AddValue(String name, String value);
	void beginObject();
	void endObject();
	void beginArray(String arrayName);
	void endArray();
	String getValue(const char* key);
	String getValue(const char* key, unsigned int fromIndex);
	int getValuePos(const char* key, unsigned int fromIndex);
};

#endif

