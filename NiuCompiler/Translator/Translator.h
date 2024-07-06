#pragma once
#include"../Synax/Synax.h"
#include"../VirtualMachine/VirtualMachine.h"
#include"set"
///////////////////////////////////////
//////////////////////////////////////
struct Translator {
	vector<string>FunctionTextSection;
	vector<string>DeclarationAndExpressionTextSetion;
	vector<string>*TextSection;
	vector<string>DataSection;
	map<string,vector<Symbol>*>symbol;
	vector<string>AllSymbol;
	Program* program;
	int64_t sp, bp, pc, ds, ds_dataCnt,label_cnt;
	string curLoop;
	string curFun;
	set<DWord>used_register;
	bool IsGLobal;
	///////////////////////////
	Translator(Program*);
	void parse();
	void ParseProgram(Program*program);
	void ParseStruct(Struct* node);
	void ParseDeclration(Declaration* node);
	void ParseFunction(Function*node);
	void ParseComposedStatement(ComposedStatement* node);
	void ParseStatement(Node* node);
	void ParseIfStatement(IfStatement* node);
	void ParseReturnStatement(ReturnStatement* statement);
	void ParseBreakStatement(Node* node);
	void ParseContinueStatement(Node* node);
	void ParseWhileStatement(WhileStatement* statement);
	void ParseForStatement(ForStatement* statement);
	void ParseAssembleStatement(AssembleStatement* statement);
	pair<DWord,Type> ParseAssignmentExp(AssignmentExp* node,bool needRet);
	pair<DWord,Type> ParseEqualityExp(EqualityExp* node, bool needRet);
	pair<DWord,Type> ParseRelationExp(RelationExp* node, bool needRet);
	pair<DWord,Type> ParseUnaryExp(UnaryExp* node, bool needRet);
	pair<DWord,Type> ParseMulExp(MulExp* node, bool needRet);
	pair<DWord,Type> ParsePostFixExp(PostFixExp* node, bool needRet);
	pair<DWord,Type>ParsePrimaryExp(PrimaryExp* node, bool needRet);
	pair<DWord,Type> ParseExpression(Expression* node, bool needRet);
	Symbol& GetSymbol(const string& id);
	Type ParseType(const string& s);
	DWord GetTypeSize(Type);
	void AddToTextSection(const string&);
	void AddToTextSection(const vector<string>&);
	string AddToDataSection(DWord size,DWord data);
	string AddToDataSection(const string& str);
	void AddSymbol(const string& id, const Symbol& symbol);
	void RemoveSymbol(const string& id);
	void UseRegister(DWord);
	DWord GetRegister();
	void RecycleRegister(DWord);
	string GetStoreInstruction(DWord);
	string GetLoadInstruction(DWord);
	vector<string> GenerateLoadInstruction(Type, const string&, const string&, int64_t,DWord);
	vector<string>	GenerateStoreInstruction(Type, const string&, const string&, int64_t, DWord);
	vector<string> StoreToStack(Type, DWord);
	DWord GetStructMemberOffset(const string&struct_id,const string& member_id);
	void WriteToTargetLval(EqualityExp* exp,DWord reg);
	string LoadDSAddress(const string&s0, DWord r0);
	string GenerateLabel();
	int64_t AlignAddress(int64_t, Type);
	string IntToDouble(const string&r0);
	string DoubleToInt(const string& r0);
	string AllocateStackBuffer(DWord);
	string FreeStackBuffer(DWord);
	vector<string>SaveUsedRegisters();
	vector<string>LoadUsedRegisters();
	string GetFunctionArg(DWord,DWord);
	void ResumeSymbolStack();
	void GenerateTempSymbol();
	string GetFinalTranslatedCode(bool indent );
	void ExportToFile(const string& file,bool indent);
	~Translator();
};