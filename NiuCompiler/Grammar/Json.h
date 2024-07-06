#pragma once
#include"string"
using namespace std;
/////////////////////////////////////////
struct Json {
	string res;
	Json();
	void AddInt(const string& key, int val);
	void AddDouble(const string& key, double val);
	void AddString(const string& key, const string& str);
	void AddJson(const string& key,Json json);
	string& GetRes();
};