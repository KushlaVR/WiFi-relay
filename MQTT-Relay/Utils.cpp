#include "Utils.h"



Utils::Utils()
{
}


Utils::~Utils()
{
}

String Utils::FormatTime(time_t time)
{
	long h =hour(time);
	int m = minute(time);
	int s = second(time);
	String ret = "";
	
	if (h < 10) ret = ret + "0";
	ret += String(h);
	ret += ":";
	
	if (m < 10) ret = ret + "0";
	ret += String(m);
	ret += ":";

	if (s < 10) ret = ret + "0";
	ret += String(s);

	return ret;
}


String Utils::FormatDate(time_t time)
{
	long y = year(time);
	int m = month(time);
	int d = day(time);
	String ret = "";

	ret += String(y);
	ret += ".";

	if (m < 10) ret = ret + "0";
	ret += String(m);
	ret += ".";

	if (d < 10) ret = ret + "0";
	ret += String(d);

	return ret;
}