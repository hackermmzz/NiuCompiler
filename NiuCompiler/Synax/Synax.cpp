#include "Synax.h"

Synax::Synax(Program*p)
{
	program = p;
	curFun = 0;
	curLoop = 0;
}

void Synax::parse()
{
	ParseProgram(program);
}

void Synax::ParseProgram(Program* node)
{
	auto&v = node->SubNode;
	for (auto& node : v)
	{
		if (node->type == NODE_FUNCTION)ParseFunction((Function*)node);
		else if (node->type == NODE_DECLARATION)ParseDeclaration((Declaration*)node);
		else if (node->type == NODE_STRUCT)ParseStruct((Struct*)node);
		else if (node->type == NODE_EXPRESSION)ParseExpression((Expression*)node);
	}
}

void Synax::ParseStruct(Struct* node)
{
	Symbol symbol;
	symbol.type.type = SYMBOL_STRUCT;
	symbol.type.id = node->declaration_list.first;
	symbol.id = node->declaration_list.first;
	auto cur = &(symbol.type.ref);
	set<string>st;
	for (auto& member : node->declaration_list.second)
	{
		auto& [type, name] = ((Member*)(member))->member;
		if (st.count(name))Error();
		*cur = new Symbol(ParseMember(member));
		cur = &(*cur)->type.ref;
		st.insert(name);
	}
	AddGlobalSymbol(symbol);

}

void Synax::ParseFunction(Function* node)
{
	auto RetType = ParseType(get<0>(node->fun));
	auto id = get<1>(node->fun);
	AddGlobalSymbol({{SYMBOL_FUNCTION,id,new Symbol({RetType})},id});
	auto& sym = stack.GetSymbol(id);
	curFun =&sym;
	auto* cur =sym.type.ref;
	set<string>arg;
	stack.AddStack();
	for (auto& m : get<2>(node->fun))
	{
		auto&& s = ParseMember((Member*)m);
		if (arg.count(s.id))Error();
		arg.insert(s.id);
		cur->type.ref = new Symbol(s);
		AddLocalSymbol(s);
		cur = cur->type.ref;
	}
	ParseComposedStatement((ComposedStatement*)get<3>(node->fun),0);
	stack.PopStack();
	curFun = 0;
}

void Synax::ParseComposedStatement(ComposedStatement* node, bool new_SymbolTable=1)
{
	if (new_SymbolTable)stack.AddStack();
	for (auto& statement : node->statement)
	{
		ParseStatement(statement);
	}
	if (new_SymbolTable)stack.PopStack();
}

void Synax::ParseStatement(Node* statement)
{
	switch (statement->type)
	{
	case NODE_COMPOSEDSTATEMENT:
		ParseComposedStatement((ComposedStatement*)statement);
		break;
	case NODE_IFSTATEMENT:
		ParseIfStatement((IfStatement*)statement);
		break;
	case NODE_RETURNSTATEMENT:
		ParseReturnStatement((ReturnStatement*)statement);
		break;
	case NODE_BREAKSTATEMENT:
		ParseBreakStatement();
		break;
	case NODE_CONTINUESTATEMENT:
		ParseContinueStatement();
		break;
	case NODE_FORSTATEMENT:
		ParseForStatement((ForStatement*)statement);
		break;
	case NODE_WHILESTATEMENT:
		ParseWhileStatement((WhileStatement*)statement);
		break;
	case NODE_EXPRESSION:
		ParseExpression((Expression*)statement);
		break;
	case NODE_DECLARATION:
		ParseDeclaration((Declaration*)statement);
		break;
	case NODE_ASSEMBLESTATEMENT:
		ParseAssembleStatement((AssembleStatement*)statement);
		break;
	default:
		break;
	}
}

void Synax::ParseIfStatement(IfStatement* node)
{
	auto&&t=ParseExpression((Expression*)get<0>(node->statement));
	if (CompareAble(t) == 0)Error();
	ParseStatement(get<1>(node->statement));
	if(get<2>(node->statement))
	ParseStatement(get<2>(node->statement));
}

void Synax::ParseReturnStatement(ReturnStatement* node)
{
	if (node->exp == 0)
	{
		if ((curFun->type.ref->type.type&SYMBOL_VOID)==0)Error();
	}
	else {
		auto&& t = ParseExpression((Expression*)node->exp);
		if (curFun == 0 || ReturnAble(curFun->type.ref->type, t) == 0)Error();
	}
}

void Synax::ParseBreakStatement()
{
	if (curLoop == 0)Error();
}

void Synax::ParseContinueStatement()
{
	if (curLoop == 0)Error();
}

void Synax::ParseForStatement(ForStatement* node)
{
	++curLoop;
	//////////////////////////////
	stack.AddStack();
	auto&& t0 = ParseExpression((Expression*)get<0>(node->statement));
	auto&& t1 = ParseExpression((Expression*)get<1>(node->statement));
	if (CompareAble(t1) == 0)Error();
	auto&& t2 = ParseExpression((Expression*)get<2>(node->statement));
	ParseStatement(get<3>(node->statement));
	stack.PopStack();
	//////////////////////////////
	--curLoop;
}

void Synax::ParseWhileStatement(WhileStatement* node)
{
	++curLoop;
	////////////////////////////
	stack.AddStack();
	auto&& t0 = ParseExpression((Expression*)(node->statement.first));
	if (CompareAble(t0) == 0)Error();
	ParseStatement(node->statement.second);
	stack.PopStack();
	////////////////////////////
	--curLoop;
}

void Synax::ParseAssembleStatement(AssembleStatement* node)
{
	//先不管
}

void Synax::ParseDeclaration(Declaration* node)
{
	auto t=ParseType(node->declaration.first);
	if (t.type & SYMBOL_VOID)Error();
	t.type=(SymbolType)(t.type|SYMBOL_LVAL);
	for (auto& [id, exp] : node->declaration.second)
	{
		if (exp) {
			auto&& tt = ParseAssignmentExp((AssignmentExp*)exp);
			if (AssignAble(t, tt) == 0)Error();
		}
		AddLocalSymbol({t,id });
	}
}

Type Synax::ParseExpression(Expression* node)
{
	Type t;
	for (auto* node : node->exp)
	{
		t=ParseAssignmentExp((AssignmentExp*)node);
	}
	return t;
}

Type Synax::ParseAssignmentExp(AssignmentExp* node)
{
	Type t;
	for (auto itr = node->exp.rbegin(); itr != node->exp.rend(); ++itr)
	{
		Type nt = ParseEqualityExp((EqualityExp*)(*itr));
		if (itr!=node->exp.rbegin()&&AssignAble(nt, t) == 0)Error();
		t = nt;
	}
	return t;
}

Type Synax::ParseEqualityExp(EqualityExp* node)
{
	auto& exp = node->exp.first;
	Type t = ParseRelationExp((RelationExp*)exp[0]);
	for (int i = 1; i < exp.size(); ++i)
	{
		Type tt= ParseRelationExp((RelationExp*)exp[i]);
		if(CompareAble(t, tt)==0)Error();
		t.type = SYMBOL_BOOL;
	}
	return t;
}

Type Synax::ParseRelationExp(RelationExp* node)
{
	auto& exp = node->exp.first;
	Type t = ParseUnaryExp((UnaryExp*)exp[0]);
	for (int i = 1; i < exp.size(); ++i)
	{
		Type tt = ParseUnaryExp((UnaryExp*)exp[i]);
		if (CompareAble(t, tt) == 0)Error();
		t.type = SYMBOL_BOOL;
	}
	return t;
}

Type Synax::ParseMulExp(MulExp* node)
{
	Type t;
	auto& exp = node->exp.first;
	auto& op = node->exp.second;
	t = ParsePostFixExp((PostFixExp*)exp[0]);
	for (int i = 1; i < exp.size(); ++i)
	{
		auto tt= ParsePostFixExp((PostFixExp*)exp[i]);
		t = MulAble(t, tt, op[i - 1]);
		if (t.type &SYMBOL_VOID)Error();
	}
	return t;
}

Type Synax::ParseUnaryExp(UnaryExp* node)
{
	auto& exp = node->exp.first;
	Type t = ParseMulExp((MulExp*)exp[0]);
	for (int i = 1; i < exp.size(); ++i)
	{
		auto tt= ParseMulExp((MulExp*)exp[i]);
		t = UnaryAble(t, tt);
		if (t.type&SYMBOL_VOID)Error();
	}
	return t;
}

Type Synax::ParsePostFixExp(PostFixExp* node)
{
	auto*n = node->exp.first;
	Type t = ParsePrimaryExp((PrimaryExp*)n);
	if (node->exp.second.size()) {
		for (auto& exp : node->exp.second)
		{
			if ((t.type & SYMBOL_LVAL) == 0 && (t.type & SYMBOL_FUNCTION) == 0)Error();
			auto& symbol = stack.GetSymbol(t.id);
			if (exp->type == NODE_ARGUMENTEXPLIST)
			{
				if ((t.type & SYMBOL_FUNCTION) == 0)Error();
				auto* node = (ArgumentExpList*)exp;
				vector<Type>ArgTypes;
				for (auto& nn : node->exp)
					ArgTypes.push_back(ParseAssignmentExp((AssignmentExp*)nn));
				///////////////////检查参数类型是不是匹配
				auto* s0 = symbol.type.ref;
				t = s0->type;//返回值
				int idx = 0;
				for (auto* s = s0->type.ref; s != 0; s = s->type.ref)
				{
					if (idx >= ArgTypes.size())Error();
					if ((s->type.type & ArgTypes[idx++].type) == 0)Error();
				}
				if (ArgTypes.size() - idx > 1)Error();
			}
			else if (exp->type == NODE_VISITMEMBER)
			{
				if ((t.type & SYMBOL_STRUCT) == 0)Error();
				bool flag = 0;
				for (auto* s = symbol.type.ref; s != 0; s = s->type.ref)
				{
					if (s->id == ((VisitMember*)(exp))->tk.literal)
					{
						flag = 1;
						t = { SymbolType(s->type.type&(t.type & SYMBOL_LVAL)),s->type.id,s->type.ref };
						break;
					}
				}
				if (!flag)Error();
			}
		}
	}
	return t;
}

Type Synax::ParsePrimaryExp(PrimaryExp* node)
{
	if (node->type == NODE_ID) {
		auto& s = stack.GetSymbol(((ID*)node)->tk.literal);
		if (s.type.type &SYMBOL_VOID)Error();
		return s.type;
	}
	else if (node->type == NODE_INTEGER)return { SYMBOL_INT };
	else if (node->type == NODE_STRING)return { SYMBOL_STRING };
	else if (node->type == NODE_DOUBLE)return { SYMBOL_DOUBLE };
	else if (node->type == NODE_CHAR)return { SYMBOL_CHAR };
	else if (node->type == NODE_BOOL)return { SYMBOL_BOOL };
	return ParseExpression((Expression*)node);
}

Symbol Synax::ParseMember(Member* node)
{
	static const string tar = "struct";
	static  unordered_map<string, SymbolType>mp{
		{"int",SYMBOL_INT},
		{"bool",SYMBOL_BOOL},
		{"char",SYMBOL_CHAR},
		{"double",SYMBOL_DOUBLE},
		{"string",SYMBOL_STRING},
	};
	auto&[type,name] = node->member;
	int idx = type.find(tar);
	if (idx == string::npos)return { {mp[type],"",0},name };
	auto struct_id = type.substr(idx + 1 + tar.size());
	Symbol*s;
	if (s =&stack.GetSymbol(struct_id); s->type.type!= SYMBOL_STRUCT)Error();
	return { {SYMBOL_STRUCT,struct_id,0},name };
}

Type Synax::ParseType(string& s)
{
	static string tar = "struct";
	int idx = s.find(tar);
	if (idx == string::npos)
	{
		if (s == "int")return { SYMBOL_INT };
		if (s == "char")return { SYMBOL_CHAR };
		if (s == "bool")return { SYMBOL_BOOL };
		if (s == "double")return { SYMBOL_DOUBLE };
		if (s == "string")return { SYMBOL_STRING };
		if (s == "void")return { SYMBOL_VOID };
	}
	auto struct_id = s.substr(idx + 1 + tar.size());
	Symbol* sy;
	if (sy = &stack.GetSymbol(struct_id); sy->type.type != SYMBOL_STRUCT)Error();
	return {SYMBOL_STRUCT,struct_id,0};
}

void Synax::AddLocalSymbol(SymbolType t, const string&s)
{
	if (stack.PushSymbol({ {t,"",0},s}) == 0)Error();
}

void Synax::AddGlobalSymbol(SymbolType t, const string&s)
{
	if (stack.IsGlobalSymbolExist(s))Error();
	stack.PushSymbol({ {t,"",0},s});
}

void Synax::AddLocalSymbol(const Symbol&s)
{
	if (stack.PushSymbol(s) == 0)Error();
}

void Synax::AddGlobalSymbol(const Symbol&s)
{
	if (stack.IsGlobalSymbolExist(s.id))Error();
	stack.PushSymbol(s);
}

void Synax::Error()
{
	cout << "Synax Error!" << endl;
	exit(0);
}

bool Synax::AssignAble(Type a, Type b)
{
	if ((a.type&SYMBOL_LVAL) == 0)return 0;//前一个不是左值不能赋值
	if (IsSameKind(a,b)==0)return 0;//类型不同不可以赋值
	return 1;
}

bool Synax::CompareAble(Type a, Type b)
{
	//整实数类型或者同为浮点数才可以比较
	if ((a.type & SYMBOL_DOUBLE) && (b.type & SYMBOL_DOUBLE))return 1;
	if (IsSameKind(a, { SYMBOL_INT }) && IsSameKind(b, { SYMBOL_INT }))return 1;
	return 0;
}

bool Synax::CompareAble(Type a)
{
	return IsSameKind(a, { SYMBOL_INT });
}

bool Synax::ReturnAble(Type ret, Type src)
{
	return IsSameKind(ret, src);
}

Type Synax::MulAble(Type a, Type b, TookenType op)
{
	if ((a.type&SYMBOL_STRUCT) || (b.type&SYMBOL_STRUCT)
		||(a.type&SYMBOL_STRING)||(b.type&SYMBOL_STRING)
		)return { SYMBOL_VOID };
	if ((a.type & SYMBOL_DOUBLE) && (b.type & SYMBOL_DOUBLE)&&op!=TOOKEN_REM)
		return { SYMBOL_DOUBLE };
	if (IsSameKind(a, { SYMBOL_INT }) && IsSameKind(b, { SYMBOL_INT }))return { SYMBOL_INT };
	return { SYMBOL_VOID };
}

Type Synax::UnaryAble(Type a, Type b)
{
	if ((a.type & SYMBOL_STRING) || (b.type & SYMBOL_STRING))return { SYMBOL_VOID };
	//如果是结构体那么相加减条件是内部的成员均可以相加减
	if ((a.type & SYMBOL_STRUCT) && (b.type & SYMBOL_STRUCT))
	{
		if (a.id != b.id)return { SYMBOL_VOID };
		auto& s = stack.GetSymbol(a.id);
		auto* cur = s.type.ref;
		while (cur) {
			if ((UnaryAble(cur->type, cur->type).type & SYMBOL_VOID))return { SYMBOL_VOID };
			cur = cur->type.ref;
		}
		a.type = (SymbolType)(a.type & LvalMaskC);
		return a;
	}
	if (IsSameKind(a, b))
	{
		if (IsSameKind(a, { SYMBOL_INT }))return { SYMBOL_INT };
		a.type = (SymbolType)(a.type & LvalMaskC);
		return a;
	}
	return { SYMBOL_VOID };
}

bool Synax::IsSameKind(Type a, Type b)
{
	if ((a.type & SYMBOL_STRUCT)&&(b.type & SYMBOL_STRUCT))return a.id==b.id;
	if ((a.type & SYMBOL_DOUBLE) && (b.type & SYMBOL_DOUBLE))return 1;
	if ((a.type & SYMBOL_STRING) && (b.type & SYMBOL_STRING))return 1;
	if (((a.type & SYMBOL_STRUCT) || (b.type & SYMBOL_STRUCT)))return 0;
	if ((a.type & SYMBOL_DOUBLE) ||(b.type & SYMBOL_DOUBLE))return 0;
	if ((a.type & SYMBOL_STRING) || (b.type & SYMBOL_STRING))return 0;
	return 1;
}
