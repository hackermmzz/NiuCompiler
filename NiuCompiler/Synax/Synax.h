#pragma once
#include "../Grammar/Grammar.h"
#include"../VirtualMachine/VirtualMachine.h"
#include"SymbolTable.h"
struct Synax {
	Program* program;
	SymbolStack stack;
	Symbol* curFun;
	uint64_t curLoop;
	Synax(Program*);
	void parse();
	void ParseProgram(Program*node);
	void ParseStruct(Struct* node);
	void ParseFunction(Function* node);
	void ParseComposedStatement(ComposedStatement* node,bool new_SymbolTable);
	void ParseStatement(Node* node);
	void ParseIfStatement(IfStatement* node);
	void ParseReturnStatement(ReturnStatement* node);
	void ParseBreakStatement();
	void ParseContinueStatement();
	void ParseForStatement(ForStatement* node);
	void ParseWhileStatement(WhileStatement* node);
	void ParseAssembleStatement(AssembleStatement* node);
	void ParseDeclaration(Declaration* node);
	Type ParseExpression(Expression* node);
	Type ParseAssignmentExp(AssignmentExp* node);
	Type ParseEqualityExp(EqualityExp* node);
	Type ParseRelationExp(RelationExp* node);
	Type ParseMulExp(MulExp* node);
	Type ParseUnaryExp(UnaryExp* node);
	Type ParsePostFixExp(PostFixExp* node);
	Type ParsePrimaryExp(PrimaryExp* node);
	Symbol ParseMember(Member* node);
	Type ParseType(string&s);
	void AddLocalSymbol(SymbolType, const string&);
	void AddGlobalSymbol(SymbolType, const string&);
	void AddLocalSymbol(const Symbol&);
	void AddGlobalSymbol(const Symbol&);
	void Error();
	bool AssignAble(Type a, Type b);
	bool CompareAble(Type a, Type b);
	bool CompareAble(Type a);
	bool ReturnAble(Type ret, Type src);
	Type MulAble(Type a, Type b,TookenType op);
	Type UnaryAble(Type a, Type b);
	bool IsSameKind(Type a, Type b);
};