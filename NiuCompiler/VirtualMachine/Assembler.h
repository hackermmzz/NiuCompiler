#pragma once
#include"Instruction.cpp"
///////////////////////////////////////Niu�ļ�����
//DataSectionSize(8) DataSection TextSection
//////////////////////////////////////
struct Assembler {
	vector<string>code;
	DWord point;
	vector<byte>AssemblerData;
	////////////////////////
	Assembler(const string& str);
	void parse();
	void Error();
	void ExportToFile(const string&);
	string& GetCode();
};