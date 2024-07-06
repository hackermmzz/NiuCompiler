#pragma once
#include "iostream"
#include"unordered_set"
#include"unordered_map"
using namespace std;
///////////////////////////////////
enum TookenType {
	TOOKEN_KEYWORD,
	//
	TOOKEN_ID,
	TOOKEN_INTEGER,
	TOOKEN_DOUBLE,
	TOOKEN_CHAR,
	TOOKEN_STRING,
	TOOKEN_BOOL,
	//
	TOOKEN_ADD,			//+
	TOOKEN_MINUS,		//-
	TOOKEN_MULT,		//*
	TOOKEN_DIV,			// /
	TOOKEN_REM,			//%
	TOOKEN_ANDB,		//&
	TOOKEN_ORB,			//|
	TOOKEN_NOTL,		//~
	TOOKEN_XOR,			//^
	TOOKEN_SHIFTL,		//<<
	TOOKEN_SHIFTR,		//>>
	TOOKEN_GT,			//>
	TOOKEN_LT,			//<
	TOOKEN_GE,			//>=
	TOOKEN_LE,			//<=
	TOOKEN_EQ,			//==
	TOOKEN_NE,			//!=
	TOOKEN_ANDL,		//&&
	TOOKEN_ORL,			//||
	TOOKEN_NOTB,		//!
	TOOKEN_ASSIGN,		//=
	TOOKEN_LPAR,		//(
	TOOKEN_RPAR,		//)
	TOOKEN_LSUQ,		//[
	TOOKEN_RSUQ,		//]
	TOOKEN_LBRA,		//{
	TOOKEN_RBRA,		//}
	TOOKEN_DOT,			//.
	TOOKEN_DQUO,		//"
	TOOKEN_QUO,			// '
	TOOKEN_SEM,			//;
	TOOKEN_COMMA,		// ,
	TOOKEN_HASHTAG,     //#
	TOOKEN_EOF,			//eof
};
////////////////////////////////
struct Info {
	uint32_t row,col;
	string file;
};
////////////////////////////////
struct Tooken {
	Info info;
	TookenType type;
	string literal;
	Tooken(uint32_t r,uint32_t c,const string&file,TookenType t)
	{
		info = { r,c,file };
		type = t;
	}
	Tooken(uint32_t r, uint32_t c, const string& file, TookenType t,const string&literal_)
	{
		info = { r,c,file };
		type = t;
		literal = literal_;
	}
	bool operator==(TookenType tk)const {
		return tk == type;
	}
	bool operator!=(TookenType tk)const {
		return tk != type;
	}
	static string show(const TookenType&tp) {
		static  unordered_map<TookenType, string>mp{
		{TOOKEN_ADD,"+"},
		{TOOKEN_MINUS,"-"},
		{TOOKEN_MULT,"*"},
		{TOOKEN_DIV,"/"},
		{TOOKEN_REM,"%"},
		{TOOKEN_ANDL,"&"},
		{TOOKEN_ORL,"|"},
		{TOOKEN_NOTL,"~"},
		{TOOKEN_SHIFTL,"<<"},
		{TOOKEN_SHIFTR,">>"},
		{TOOKEN_GT,">"},
		{TOOKEN_LT,"<"},
		{TOOKEN_GE,">="},
		{TOOKEN_LE,"<="},
		{TOOKEN_NOTB,"!"},
		{TOOKEN_NE,"!="},
		{TOOKEN_EQ,"=="},
		{TOOKEN_ASSIGN,"="},
		{TOOKEN_ANDB,"&&"},
		{TOOKEN_ORB,"||"},
		{TOOKEN_LPAR,"("},
		{TOOKEN_RPAR,")"},
		{TOOKEN_LSUQ,"["},
		{TOOKEN_RSUQ,"]"},
		{TOOKEN_LBRA,"{"},
		{TOOKEN_RBRA,"}"},
		{TOOKEN_DQUO,"\""},
		{TOOKEN_QUO,"\'"},
		{TOOKEN_SEM,";"},
		{TOOKEN_COMMA,","},
		{TOOKEN_DOT,"."},
		};
		if (mp.count(tp))
		{
			return mp[tp];
		}
	}
};
////////////////////////////////¹Ø¼ü×Ö
const unordered_set<string>KeyWords{
"for",
"while",
"continue",
"break",
"if",
"else",
"return",
"int",
"double",
"string",
"char",
"bool",
"void",
"true",
"false",
"struct",
"assemble",
};