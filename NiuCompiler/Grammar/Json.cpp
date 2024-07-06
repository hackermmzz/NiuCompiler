#include "Json.h"

Json::Json()
{
	res = "{";
}

void Json::AddInt(const string& key, int val)
{
	if (res.size()!=1)res += ",";
	res += "\"" + key + "\"" + ":" + to_string(val);
}

void Json::AddDouble(const string& key, double val)
{
	if (res.size() != 1)res += ",";
	res += "\"" + key + "\"" + ":" + to_string(val);
}

void Json::AddString(const string& key, const string& str)
{
	if (res.size() != 1)res += ",";
	res += "\"" + key + "\"" + ":" + "\"" +str+"\"";
}

void Json::AddJson(const string& key, Json json)
{
	if (res.size() != 1)res += ",";
	res += "\"" + key + "\"" + ":" + json.GetRes();
}

string& Json::GetRes()
{
	res += "}";
	return res;
}
