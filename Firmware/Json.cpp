#include "Json.h"

JsonString::JsonString(const char * cstr) :String(cstr) {}
JsonString::JsonString(const String &str) : String(str) {}

void JsonString::appendComa() {
	if (endsWith("\"") || endsWith("}") || endsWith("]")) *this += ",";
}

void JsonString::AddValue(String name, String value)
{
	appendComa();
	*this += "\"" + name + "\":" + "\"" + value + "\"";
}

void JsonString::beginObject()
{
	appendComa();
	*this += "{";
}

void JsonString::endObject()
{
	String s = *static_cast<String*> (this);
	*this += "}";
}


void JsonString::beginArray(String arrayName)
{
	appendComa();
	*this += "\"" + arrayName + "\":[";
}

void JsonString::endArray()
{
	*this += "]";
}

String JsonString::getValue(const char * key)
{
	return getValue(key, 0);
}

String JsonString::getValue(const char * key, unsigned int fromIndex)
{
	int p = getValuePos(key, fromIndex);
	if (p > 0) {
		int startIndex = indexOf("\"", p);
		if (startIndex > 0) {
			startIndex++;
			int endIndex = indexOf("\"", startIndex);
			if (endIndex > 0) {
				return substring(startIndex, endIndex);
			}
		}
	}
	return "";
}

int JsonString::getValuePos(const char * key, unsigned int fromIndex)
{
	int p = indexOf(key, fromIndex);
	if (p > 0) {
		p = indexOf(":", p + 1);
		if (p > 0) {
			return p + 1;
		}
	}
	return -1;
}
