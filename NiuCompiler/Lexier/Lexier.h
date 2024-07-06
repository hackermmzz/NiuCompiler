#pragma once
#include"Tooken.cpp"
#include"map"
#include"string"
#include "fstream"
/////////////////////////////
struct Lexier
{
	vector<Tooken>tookens;
	uint32_t point, row, col;
	string file;
	string data;
	/////////////////////////////////
	Lexier(const char* file);
	void parse();
	char NextChar();
	char NextNChar(uint32_t);
	void RollBack();
	char AdvanceChar();
	char AdvanceNChar(uint32_t n);
	char GetChar();
	void SkipSpace();
	bool IsEmpty(char);
	void AddTooken(TookenType type);
	void AddTooken(TookenType type,const string&literal);
	void Error();
	string ParseString(string& str);
	
};