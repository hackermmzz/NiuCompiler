#pragma once
#include"stack"
#include"string"
#include"set"
#include"map"
#include"../VirtualMachine/VirtualMachine.h"
using namespace std;
/////////////////////////////////
enum SymbolType {
	SYMBOL_CHAR=1,
	SYMBOL_BOOL=2,
	SYMBOL_INT=1<<2,
	SYMBOL_DOUBLE=1<<3,
	SYMBOL_STRING=1<<4,
	SYMBOL_VOID = 1 <<5,
	SYMBOL_STRUCT=1<<6,
	SYMBOL_FUNCTION=1<<7,
	SYMBOL_LVAL=1<<8,
	SYMBOL_GLOBAL=1<<9,
	
};
const int LvalMaskC = ~SYMBOL_LVAL;
/////////////////////////////////
struct Type {
	SymbolType type;
	string id;
	struct Symbol* ref;
};
/////////////////////////////////
struct Symbol{
	Type type;
	string id;
	DWord size;
	string address;
	bool operator==(const Symbol&)const;
};
////////////////////////////////
struct SymbolTable {
	map<string,Symbol>table;
	bool AddSymbol(const Symbol&s);
};
/////////////////////////////////
struct SymbolStack {
	vector<SymbolTable*>stack;
	SymbolStack();
	void AddStack();
	void PopStack();
	bool PushSymbol(const Symbol& s);
	bool IsLocalSymbolExist(const string& s);
	bool IsGlobalSymbolExist(const string& s);
	Symbol& GetSymbol(const string& s);
	~SymbolStack();
};