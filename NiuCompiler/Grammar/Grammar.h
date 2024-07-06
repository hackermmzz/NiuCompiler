#pragma once
#include"../Lexier/Lexier.h"
#include"Node.h"
////////////////////////////////////
extern  Tooken EmptyTooken;
///////////////////////////////////
struct Grammar {
	vector<Tooken>* tookens;
	uint32_t point;
	Program* program;
	///////////////////////////////
	Grammar(vector<Tooken>&);
	void parse();
	Program* ParseProgram();
	Node* ParseExternalDeclaration();
	Tooken& GetTooken();
	Tooken& NextTooken();
	Tooken& NextNTooken(uint32_t n);
	Tooken& AdvanceTooken();
	Tooken& AdvanceNtooken(uint32_t n);
	void Error();
	int IsType(Tooken&, Tooken&);
	Node* ParseStruct();
	Node* ParseFunction();
	Node* ParseDeclaration();
	Node* ParseAssignmentExp();
	Node* ParseEqualityExp();
	Node* ParseRelationExp();
	Node* ParseMulExp();
	Node* ParseUnaryExp();
	Node* ParsePostfixExp();
	Node* ParsePrimaryExp();
	Node* ParseArgExpList();
	Node*ParseExpression();
	Node* ParseComposedStatement();
	Node* ParseStatement();
	Node* ParseIfStatement();
	Node* ParseReturnStatement();
	Node* ParseBreakStatement();
	Node* ParseContinueStatement();
	Node* ParseForStatement();
	Node* ParseWhileStatement();
	Node* ParseAssembleStatement();
	void show();
};