#pragma once
#include"../Lexier/Lexier.h"
#include"Json.h"
/////////////////////////////////
enum NodeType {
	NODE_PROGRAM,
	NODE_STRUCT,
	NODE_MEMBER,
	NODE_FUNCTION,
	NODE_DECLARATION,
	NODE_ASSIGNMENTEXP,
	NODE_EQUALITYEXP,
	NODE_RELATIONEXP,
	NODE_MULEXP,
	NODE_UNARYEXP,
	NODE_POSTFIXEXP,
	NODE_ARGUMENTEXPLIST,
	NODE_VISITMEMBER,
	NODE_PRIMARYEXP,
	NODE_EXPRESSION,
	NODE_COMPOSEDSTATEMENT,
	NODE_IFSTATEMENT,
	NODE_RETURNSTATEMENT,
	NODE_BREAKSTATEMENT,
	NODE_CONTINUESTATEMENT,
	NODE_FORSTATEMENT,
	NODE_WHILESTATEMENT,
	NODE_ASSEMBLESTATEMENT,
	/////////////////
	NODE_INTEGER,
	NODE_BOOL,
	NODE_ID,
	NODE_STRING,
	NODE_DOUBLE,
	NODE_CHAR,
	////////////////

};
///////////////////////////////////////
using NodeRet = pair<string,Json>;
///////////////////////////////////////
struct Node{
	NodeType type;
	void* data;
	Node(NodeType,void*);
	virtual NodeRet show();
};
///////////////////////////////
struct Program:Node{
	vector<Node*>SubNode;
	Program();
	virtual NodeRet show();
};
////////////////////////////////
////////////////////////////////
struct Member :Node {
	pair<string, string>member;
	Member(string&,string&);
	virtual NodeRet show();
};
///////////////////////////////
struct Struct :Node {
	pair<string, vector<Member*>>declaration_list;
	Struct();
	virtual NodeRet show();
};
//////////////////////////////
struct Function :Node {
	tuple<string,string, vector<Member*>,Node*>fun;
	Function();
	virtual NodeRet show();
};
//////////////////////////////
//////////////////////////////
struct Declaration :Node {
	pair<string, vector<pair<string,Node*>>>declaration;
	Declaration();
	virtual NodeRet show();
};
//////////////////////////////
struct AssignmentExp :Node {
	vector<Node*>exp;
	AssignmentExp();
	virtual NodeRet show();
};
//////////////////////////////
struct EqualityExp :Node {
	pair<vector<Node*>, vector<TookenType>>exp;
	EqualityExp();
	virtual NodeRet show();
};
/////////////////////////////
struct RelationExp:Node{
	pair<vector<Node*>, vector<TookenType>>exp;
	RelationExp();
	virtual NodeRet show();
};
/////////////////////////////
struct MulExp :Node {
	pair<vector<Node*>, vector<TookenType>>exp;
	MulExp();
	virtual NodeRet show();
};
//////////////////////////////
struct UnaryExp :Node {
	pair<vector<Node*>, vector<TookenType>>exp;
	UnaryExp();
	virtual NodeRet show();
};
//////////////////////////////
struct PostFixExp :Node {
	pair<Node*, vector<Node*>>exp;
	PostFixExp();
	virtual NodeRet show();
};
//////////////////////////////
struct ArgumentExpList :Node {
	vector<Node*>exp;
	ArgumentExpList();
	virtual NodeRet show();
};
//////////////////////////////
struct VisitMember :Node {
	Tooken tk;
	VisitMember(const Tooken& );
	virtual NodeRet show();
};
//////////////////////////////
struct PrimaryExp :Node {
	Node* exp;
	PrimaryExp();
	virtual NodeRet show();
};
//////////////////////////////
struct Expression :Node {
	vector<Node*>exp;
	Expression();
	virtual NodeRet show();
};
//////////////////////////////
struct ID :Node {
	Tooken tk;
	ID(const Tooken&);
	virtual NodeRet show();
};
//////////////////////////////
struct Integer :Node {
	uint64_t val;
	Integer(const Tooken& );
	virtual NodeRet show();
};
/////////////////////////////
struct String :Node {
	string str;
	String(const Tooken&);
	virtual NodeRet show();
};
/////////////////////////////
struct Double:Node
{
	double val;
	Double(const Tooken&);
	virtual NodeRet show();
};
/////////////////////////////
struct Char:Node
{
	char ch;
	Char(const Tooken&);
	virtual NodeRet show();
};
/////////////////////////////
struct Bool :Node {
	bool val;
	Bool(const Tooken&);
	virtual NodeRet show();
};
/////////////////////////////
struct ComposedStatement :Node {
	vector<Node*>statement;
	ComposedStatement();
	virtual NodeRet show();
};
//////////////////////////////
struct IfStatement :Node {
	tuple<Node*, Node*, Node*>statement;
	IfStatement();
	virtual NodeRet show();
};
/////////////////////////////
struct ReturnStatement :Node {
	Node* exp;
	ReturnStatement();
	virtual NodeRet show();
};
/////////////////////////////
struct ForStatement :Node {
	tuple<Node*, Node*, Node*, Node*>statement;
	ForStatement();
	virtual NodeRet show();
};
/////////////////////////////
struct WhileStatement :Node {
	pair<Node*, Node*>statement;
	WhileStatement();
	virtual NodeRet show();
};
//////////////////////////////
struct AssembleStatement :Node {
	vector<string>statement;
	AssembleStatement();
	virtual NodeRet show();
};
//////////////////////////////
NodeRet ShowNodeAndTookenType(const string&,pair<vector<Node*>, vector<TookenType>>&);