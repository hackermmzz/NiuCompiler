#include "Grammar.h"
////////////////////////////////////////////
Tooken EmptyTooken{ 0,0,string(""),TOOKEN_EOF };
////////////////////////////////////////////
Grammar::Grammar(vector<Tooken>&tk)
{
	tookens = &tk;
	point = 0;
	program = 0;
}

void Grammar::parse()
{
	program = ParseProgram();
}

Program* Grammar::ParseProgram()
{
	Program* p = new Program();
	while (GetTooken().type!=TOOKEN_EOF)
	{
		Node*node=ParseExternalDeclaration();
		p->SubNode.push_back(node);
		NextTooken();
	}
	return p;
}

Node* Grammar::ParseExternalDeclaration()
{
	auto& tk0 = GetTooken();
	auto& tk1 = AdvanceNtooken(1);
	auto& tk2 = AdvanceNtooken(2);
	auto& tk3 = AdvanceNtooken(3);
	if (int cnt=IsType(tk0, tk1))
	{
		if((cnt==1&&tk2==TOOKEN_LPAR)||(cnt==2&&tk3==TOOKEN_LPAR))return ParseFunction();
		else if((cnt==1&&tk2==TOOKEN_ASSIGN)||(cnt==2&&tk3==TOOKEN_ASSIGN))return ParseDeclaration();
		else if(cnt==2&&tk2==TOOKEN_LBRA)return ParseStruct();
		else Error();
	}
	else if (tk0 == TOOKEN_ID)
	{
		auto* node = ParseExpression();
		if (NextTooken() != TOOKEN_SEM)Error();
		return node;
	}
	else Error();
	return 0;
}

Tooken& Grammar::GetTooken()
{
	if (point >= tookens->size())return EmptyTooken;
	return (*tookens)[point];
}

Tooken& Grammar::NextTooken()
{
	++point;
	return GetTooken();
}

Tooken& Grammar::NextNTooken(uint32_t n)
{
	point += n;
	return GetTooken();
}

Tooken& Grammar::AdvanceTooken()
{
	if (point + 1 >= tookens->size())return EmptyTooken;
	return (*tookens)[point + 1];
}

Tooken& Grammar::AdvanceNtooken(uint32_t n)
{
	if (point + n >= tookens->size())return EmptyTooken;
	return (*tookens)[point + n];
}

void Grammar::Error()
{
	auto&tk=GetTooken();
	cout << "Grammar Error in file: " << tk.info.file << " at row: " << tk.info.row << " at col: " << tk.info.col << endl;
	exit(0);
}

int Grammar::IsType(Tooken&tk0, Tooken&tk1)
{
	static const unordered_set<string>st{ "int","double","string","char","bool","void"};
	if (tk0.type == TOOKEN_KEYWORD)
		if (st.count(tk0.literal))return 1;
		else if (tk0.literal == "struct" && tk1 == TOOKEN_ID)return 2;
	return 0;
}

Node* Grammar::ParseStruct()
{
	auto& tk0 = GetTooken(), &tk1 = AdvanceTooken(),&tk2=AdvanceNtooken(2);
	if (tk0 != TOOKEN_KEYWORD || tk0.literal != "struct"||tk1!=TOOKEN_ID||tk2!=TOOKEN_LBRA)Error();
	Struct* node = new Struct;
	node->declaration_list.first = tk1.literal;
	NextNTooken(3);
	while (1)
	{
		auto& tk0 = GetTooken();
		auto& tk1 = AdvanceTooken();
		auto& tk2 = AdvanceNtooken(2);
		if (int cnt = IsType(tk0, tk1))
		{
			string kind, name;
			if (cnt == 1)
			{
				kind = tk0.literal;
				if (tk1.type != TOOKEN_ID)Error();
				name = tk1.literal;
				NextNTooken(2);
			}
			else {
				kind = tk0.literal + " " + tk1.literal;
				if (tk2.type != TOOKEN_ID)Error();
				name = tk2.literal;
				NextNTooken(3);
			}
			node->declaration_list.second.push_back(new Member(kind, name));
			if (GetTooken() != TOOKEN_SEM)Error();
			else if (NextTooken() == TOOKEN_RBRA)break;
		}
		else Error();
	}
	return node;
}

Node* Grammar::ParseFunction()
{
	auto& tk0 = GetTooken(), & tk1 = AdvanceTooken(), & tk2 = AdvanceNtooken(2);
	Function* node = new Function;
	if (int cnt = IsType(tk0, tk1))
	{
		if (cnt == 1)
		{
			get<0>(node->fun) = tk0.literal;
			if (tk1.type != TOOKEN_ID)Error();
			get<1>(node->fun) = tk1.literal;
			NextNTooken(2);
		}
		else if (cnt == 2)
		{
			get<0>(node->fun) = tk0.literal + " " + tk1.literal;
			if (tk2 != TOOKEN_ID)Error();
			get<1>(node->fun) = tk2.literal;
			NextNTooken(3);
		}
		if (GetTooken() != TOOKEN_LPAR)Error();
		NextTooken();
	}
	else Error();
	/////////////////////////////////////
	bool flag = GetTooken() != TOOKEN_RPAR;
	while (flag)
	{
		auto& tk0 = GetTooken(), & tk1 = AdvanceTooken(), & tk2 = AdvanceNtooken(2);
		if (int cnt = IsType(tk0, tk1))
		{
			string kind, name;
			if (cnt == 1)
			{
				kind = tk0.literal;
				if (tk1 != TOOKEN_ID)Error();
				name = tk1.literal;
				NextNTooken(2);
			}
			else if (cnt == 2)
			{
				kind = tk0.literal + " " + tk1.literal;
				if (tk2 != TOOKEN_ID)Error();
				name = tk2.literal;
				NextNTooken(3);
			}
			get<2>(node->fun).push_back(new Member(kind, name));
			if (GetTooken() == TOOKEN_COMMA)NextTooken();
			else if (GetTooken() == TOOKEN_RPAR)flag = 0;
		}
		else Error();
	}
	///////////////////////////////////////funbody
	NextTooken();
	get<3>(node->fun) = ParseComposedStatement();
	return node;
}

Node* Grammar::ParseDeclaration()
{
	Declaration* dec = new Declaration;
	Tooken& tk0 = GetTooken(), & tk1 = AdvanceTooken(), & tk2 = AdvanceNtooken(2);
	if (int cnt = IsType(tk0, tk1))
	{
		string kind;
		if (cnt == 1)
		{
			kind = tk0.literal;
			NextTooken();
		}
		else if (cnt == 2)
		{
			kind = tk0.literal + " " + tk1.literal;
			NextNTooken(2);
		}
		dec->declaration.first = kind;
	}
	do {
		auto& tk0 = GetTooken();
		if (tk0 != TOOKEN_ID)Error();
		Node* node = 0;
		if (AdvanceTooken() == TOOKEN_ASSIGN) {
			NextNTooken(2);
			node = ParseAssignmentExp();
		}
		dec->declaration.second.push_back({ tk0.literal,node });
		if (NextTooken() == TOOKEN_SEM)break;
		NextTooken();
	} while (1);

	return dec;
}

Node* Grammar::ParseAssignmentExp()
{
	AssignmentExp* exp = new AssignmentExp();
	do {
		auto*node=ParseEqualityExp();
		exp->exp.push_back(node);
		if (AdvanceTooken() == TOOKEN_ASSIGN) {
			NextNTooken(2);
		}
		else break;
	} while (1);
	return exp;
}

Node* Grammar::ParseEqualityExp()
{
	static const unordered_set<TookenType>st{ TOOKEN_ORB,TOOKEN_EQ,TOOKEN_NE,TOOKEN_ANDB };
	EqualityExp* exp = new EqualityExp();
	do {
		auto*node=ParseRelationExp();
		exp->exp.first.push_back(node);
		if (st.count(AdvanceTooken().type) == 1)
		{
			exp->exp.second.push_back(NextTooken().type);
			NextTooken();
		}
		else break;
	} while (1);
	return exp;
}

Node* Grammar::ParseRelationExp()
{
	static const unordered_set<TookenType>st{ TOOKEN_LT,TOOKEN_GT,TOOKEN_LE,TOOKEN_GE };
	RelationExp* exp = new RelationExp;
	do {
		auto* node = ParseUnaryExp();
		exp->exp.first.push_back(node);
		if (st.count(AdvanceTooken().type) == 1)
		{
			exp->exp.second.push_back(NextTooken().type);
			NextTooken();
		}
		else break;
	} while (1);
	return exp;
}

Node* Grammar::ParseMulExp()
{
	static const unordered_set<TookenType>st{ TOOKEN_MULT,TOOKEN_DIV,TOOKEN_REM };
	MulExp* exp = new MulExp;
	do {
		auto* node = ParsePostfixExp();
		exp->exp.first.push_back(node);
		if (st.count(AdvanceTooken().type))
		{
			exp->exp.second.push_back(NextTooken().type);
			NextTooken();
		}
		else break;
	} while (1);
	return exp;
}

Node* Grammar::ParseUnaryExp()
{
	static const unordered_set<TookenType>st{ TOOKEN_ADD,TOOKEN_MINUS};
	UnaryExp* exp = new UnaryExp;
	do {
		auto* node = ParseMulExp();
		exp->exp.first.push_back(node);
		if (st.count(AdvanceTooken().type))
		{
			exp->exp.second.push_back(NextTooken().type);
			NextTooken();
		}
		else break;
	} while (1);
	return exp;
}

Node* Grammar::ParsePostfixExp()
{
	PostFixExp* exp = new PostFixExp;
	auto* node = ParsePrimaryExp();
	exp->exp.first = node;
	while (AdvanceTooken() == TOOKEN_LPAR || AdvanceTooken() == TOOKEN_DOT)
	{
		if (AdvanceTooken() == TOOKEN_LPAR)
		{
			NextTooken();
			NextTooken();
			////////////////////arg_exp_list
			if (GetTooken() != TOOKEN_RPAR)
			{
				exp->exp.second.push_back(ParseArgExpList());
				NextTooken();
			}
			else exp->exp.second.push_back(new ArgumentExpList());
			auto tk = GetTooken();
			if (GetTooken() != TOOKEN_RPAR)Error();
			////////////////////
		}
		else if (AdvanceTooken() == TOOKEN_DOT)
		{
			NextTooken();
			if (NextTooken() != TOOKEN_ID)Error();
			exp->exp.second.push_back(new VisitMember(GetTooken()));
		}
		if (AdvanceTooken() == TOOKEN_LPAR || AdvanceTooken() == TOOKEN_DOT)NextTooken();
		else break;
	}
	return exp;
}

Node* Grammar::ParsePrimaryExp()
{
	auto& tk = GetTooken();
	if (tk.type == TOOKEN_ID)return new ID(tk);
	else if (tk.type == TOOKEN_INTEGER)return new Integer(tk);
	else if (tk.type == TOOKEN_STRING)return new String(tk);
	else if (tk.type == TOOKEN_DOUBLE)return new Double(tk);
	else if (tk.type == TOOKEN_CHAR)return new Char(tk);
	else if (tk.type == TOOKEN_BOOL)return new Bool(tk);
	else if (tk.type == TOOKEN_LPAR)
	{
		NextTooken();
		auto* node = ParseExpression();
		NextTooken();
		return node;
	}
	Error();
	return 0;
}

Node* Grammar::ParseArgExpList()
{
	ArgumentExpList* exp = new ArgumentExpList;
	do {
		auto* node = ParseAssignmentExp();
		exp->exp.push_back(node);
		if (AdvanceTooken() == TOOKEN_COMMA)NextNTooken(2);
		else break;
	} while (1);
	return exp;
}

Node* Grammar::ParseExpression()
{
	auto exp = new Expression();
	do {
		auto* node = ParseAssignmentExp();
		exp->exp.push_back(node);
		if (AdvanceTooken() == TOOKEN_COMMA)
			NextNTooken(2);
		else break;
	} while (1);
	return exp;
}

Node* Grammar::ParseComposedStatement()
{
	auto* composed_statement = new ComposedStatement;
	auto& tk0 = GetTooken();
	if (tk0 != TOOKEN_LBRA)Error();
	if(NextTooken() != TOOKEN_RBRA)
	while (1)
	{
		auto* node = ParseStatement();
		composed_statement->statement.push_back(node);
		if (NextTooken() == TOOKEN_RBRA)break;
	}
	return composed_statement;
}

Node* Grammar::ParseStatement()
{
	auto& tk = GetTooken();
	if (tk == TOOKEN_LBRA)return ParseComposedStatement();
	else if (IsType(tk, AdvanceTooken()))return ParseDeclaration();
	else if (tk == TOOKEN_KEYWORD)
	{
		if (tk.literal == "if")return ParseIfStatement();
		else if (tk.literal == "return")return ParseReturnStatement();
		else if (tk.literal == "break")return ParseBreakStatement();
		else if (tk.literal == "continue")return ParseContinueStatement();
		else if (tk.literal == "for")return ParseForStatement();
		else if (tk.literal == "while")return ParseWhileStatement();
		else if (tk.literal == "assemble")return ParseAssembleStatement();
	}
	//否则就是表达式语句
	auto* node = ParseExpression();
	if (NextTooken() != TOOKEN_SEM)Error();
	return node;
}

Node* Grammar::ParseIfStatement()
{
	auto* statement = new IfStatement;
	if (GetTooken()!=TOOKEN_KEYWORD ||GetTooken().literal != "if")Error();
	if (NextTooken() != TOOKEN_LPAR)Error();
	NextTooken();
	get<0>(statement->statement) = ParseExpression();
	if (NextTooken() != TOOKEN_RPAR)Error();
	NextTooken();
	get<1>(statement->statement)= ParseStatement();
	if (AdvanceTooken()==TOOKEN_KEYWORD && AdvanceTooken().literal == "else")
	{
		NextNTooken(2);
		get<2>(statement->statement) = ParseStatement();
	}
	return statement;
	
}

Node* Grammar::ParseReturnStatement()
{
	if (GetTooken()!=TOOKEN_KEYWORD||GetTooken().literal != "return")Error();
	auto* ret = new ReturnStatement;
	if (NextTooken() == TOOKEN_SEM)ret->exp = 0;
	else {
		ret->exp = ParseExpression();
		if (NextTooken() != TOOKEN_SEM)Error();
	}
	return ret;
}

Node* Grammar::ParseBreakStatement()
{
	if (GetTooken()!=TOOKEN_KEYWORD||GetTooken().literal != "break")Error();
	NextTooken();
	if (NextTooken() != TOOKEN_SEM)Error();
	return new Node(NODE_BREAKSTATEMENT,0);
}

Node* Grammar::ParseContinueStatement()
{
	if (GetTooken()!=TOOKEN_KEYWORD||GetTooken().literal != "continue")Error();
	NextTooken();
	if (NextTooken() != TOOKEN_SEM)Error();
	return new Node(NODE_CONTINUESTATEMENT, 0);
}

Node* Grammar::ParseForStatement()
{
	if (GetTooken()!=TOOKEN_KEYWORD||GetTooken().literal != "for")Error();
	if (NextTooken() != TOOKEN_LPAR)Error();
	auto* statement = new ForStatement;
	NextTooken();
	get<0>(statement->statement) = ParseExpression();
	if (NextTooken() != TOOKEN_SEM)Error();
	NextTooken();
	get<1>(statement->statement) = ParseExpression();
	if (NextTooken() != TOOKEN_SEM)Error();
	NextTooken();
	get<2>(statement->statement) = ParseExpression();
	if (NextTooken() != TOOKEN_RPAR)Error();
	NextTooken();
	get<3>(statement->statement) = ParseStatement();
	return statement;
}

Node* Grammar::ParseWhileStatement()
{
	if (GetTooken()!=TOOKEN_KEYWORD||GetTooken().literal != "while")Error();
	if (NextTooken() != TOOKEN_LPAR)Error();
	auto* statement = new WhileStatement;
	NextTooken();
	statement->statement.first = ParseExpression();
	if (NextTooken() != TOOKEN_RPAR)Error();
	NextTooken();
	statement->statement.second = ParseStatement();
	return statement;
}

Node* Grammar::ParseAssembleStatement()
{
	if (GetTooken() != TOOKEN_KEYWORD || GetTooken().literal != "assemble")Error();
	if (NextTooken() != TOOKEN_LBRA)Error();
	auto* statement = new AssembleStatement;
	NextTooken();
	do {
		auto& tk = GetTooken();
		if (tk.type != TOOKEN_STRING)Error();
		statement->statement.emplace_back(tk.literal);
		if (NextTooken() == TOOKEN_COMMA) NextTooken();
		else if (GetTooken() != TOOKEN_RBRA)Error();
		else break;
	} while (1);
	return statement;
}

void Grammar::show()
{
	auto&& r = program->show();
	Json res;
	res.AddJson(r.first, r.second);
	cout << res.GetRes() << endl;
}


