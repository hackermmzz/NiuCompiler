#pragma once
#include"Instruction.cpp"
///////////////////////////////////////NiuÎÄ¼şÃèÊö
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