#include "Translator.h"

Translator::Translator(Program* p)
{
	TextSection = 0;
	program = p;
	sp = bp = pc = ds = ds_dataCnt = label_cnt = 0;
	IsGLobal = 1;
	//////////////////////添加一个空symbol
	AddSymbol("", {{ SYMBOL_VOID,"",0 }, "", 0, ""});
}

void Translator::parse()
{
	ParseProgram(program);
}

void Translator::ParseProgram(Program* program)
{
	for (auto& node : program->SubNode)
	{
		if (node->type == NODE_FUNCTION)
		{
			TextSection = &FunctionTextSection;
			ParseFunction((Function*)node);
		}
		else if (node->type == NODE_DECLARATION)
		{
			TextSection = &DeclarationAndExpressionTextSetion;
			ParseDeclration((Declaration*)node);
		}
		else if (node->type == NODE_STRUCT)ParseStruct((Struct*)node);
		else if (node->type == NODE_EXPRESSION)
		{
			TextSection = &DeclarationAndExpressionTextSetion;
			ParseExpression((Expression*)node, 0);
		}
	}
	////////////////////如果没有名为EntryFunction的函数,报错
	if (symbol.count(EntryFunction)==0||(GetSymbol(EntryFunction).type.type & SYMBOL_FUNCTION) == 0)
	{
		cout << "There is no function named " << EntryFunction << "!" << endl;
		exit(0);
	}
	//////////////////////
	TextSection = new vector<string>;
	AddToTextSection(DeclarationAndExpressionTextSetion);
	AddToTextSection({ string("call "+EntryFunction),string("li $r0,"+to_string(exitSyscall)),string("syscall")});
	AddToTextSection(FunctionTextSection);
}

void Translator::ParseStruct(Struct* node)
{
	Symbol s;
	s.id = node->declaration_list.first;
	s.type.id = s.id;
	s.type.type = SYMBOL_STRUCT;
	auto* curs =&s;
	for (auto& member : node->declaration_list.second)
	{
		auto& [type, id] = ((Member*)(member))->member;
		auto t = ParseType(type);
		Symbol ss{ t,id,GetTypeSize(t) };
		curs->type.ref = new Symbol(ss);
		curs = curs->type.ref;
	}
	int64_t size = 0;
	curs = s.type.ref;
	while (curs)
	{
		////////////////内存对齐
		size=AlignAddress(size, curs->type);
		curs->address = to_string(size);
		size += curs->size;
		curs = curs->type.ref;
	}
	s.size = AlignAddress(size, { SYMBOL_STRUCT });
	AddSymbol(s.id, s);
}

string Translator::AddToDataSection(DWord size, DWord data)
{
	static  unordered_map<DWord, string>mp{
		{1,"Byte"},
		{8,"DWord"}
	};
	if(ds%size)DataSection.emplace_back(".align "+to_string(size));
	auto data_id = "ds" + to_string(ds_dataCnt++);
	auto tp = mp.count(size) ? mp[size] : "Space "+to_string(size);
	DataSection.emplace_back(data_id+" : ." +tp+" " + to_string(data));
	ds += size;
	return data_id;
	
}

string Translator::AddToDataSection(const string& str)
{
	auto data_id = "ds" + to_string(ds_dataCnt++);
	DataSection.emplace_back(data_id + " : .asciiz " +"\"" + str+"\"");
	ds += str.size()+1;//补个'\0'
	return data_id;
}

void Translator::AddSymbol(const string& id, const Symbol& s)
{
	if (symbol[id] == 0)symbol[id] = new vector<Symbol>;
	symbol[id]->emplace_back(s);
	AllSymbol.emplace_back(id);
}

void Translator::RemoveSymbol(const string& id)
{
	symbol[id]->pop_back();
}

void Translator::UseRegister(DWord idx)
{
	used_register.insert(idx);
}

DWord Translator::GetRegister()
{
	DWord i = 0;
	for (; i < MaxRegister; ++i)
		if (used_register.count(i) == 0)break;
	UseRegister(i);
	if (i == MaxRegister) {
		cout << "More Than max counts of register!" << endl;
		exit(0);
	}
	return i;
}

void Translator::RecycleRegister(DWord d)
{
	used_register.erase(d);
}

string Translator::GetStoreInstruction(DWord d)
{
	if (d == 1)return "sb";
	if (d==8)return "sdw";
	return "";
}

string Translator::GetLoadInstruction(DWord d)
{
	if (d == 1)return "lb";
	if (d == 8)return "ldw";
	return "";
}

vector<string> Translator::GenerateLoadInstruction(Type type, const string&r0, const string&r1, int64_t off, DWord temp_reg=-1)
{
	if ((type.type & SYMBOL_STRUCT) == 0)
	{
		string instruction = GetLoadInstruction(GetTypeSize(type));
		return { instruction + " " + r0+ "," + to_string(off) + "(" + r1 + ")" };
	}
	//结构体单独生成拷贝指令
	auto s = GetSymbol(type.id);
	auto* cur = s.type.ref;
	vector<string>res;
	auto temp_off = off;
	int64_t temp_off1 = 0;
	bool recycle = 0;
	if (temp_reg == -1)temp_reg = GetRegister(), recycle = 1;
	while (cur)
	{
		auto o= stoll(cur->address);
		off =temp_off+o;
		//////////将目标地址向下偏移
		auto dis = o - temp_off1;
		temp_off1 = o;
		if(dis)
		res.emplace_back("addi " +r0+","+ r0 + "," + to_string(dis));
		/////////
		if (cur->type.type & SYMBOL_STRUCT)
			for (auto& s : GenerateLoadInstruction(cur->type, r0, r1, off, temp_reg))res.emplace_back(s);
		else {
			for (auto& s : GenerateLoadInstruction(cur->type, "$r" + to_string(temp_reg), r1, off))res.emplace_back(s);
			for (auto& s : GenerateStoreInstruction(cur->type, "$r"+to_string(temp_reg),r0, 0, temp_reg))res.emplace_back(s);
		}
		cur = cur->type.ref;
	}
	if (recycle)
		RecycleRegister(temp_reg);
	return res;
}

vector<string> Translator::GenerateStoreInstruction(Type type, const string&r0, const string&r1, int64_t off, DWord temp_reg=-1)
{
	if ((type.type & SYMBOL_STRUCT) == 0)
	{
		string instruction = GetStoreInstruction(GetTypeSize(type));
		return { instruction + " " + r0 + "," + to_string(off) + "(" +r1+ ")"};
	}
	//结构体单独生成拷贝指令
	auto s = GetSymbol(type.id);
	auto* cur = s.type.ref;
	vector<string>res;
	bool recycle = 0;
	auto temp_off = off;
	if (temp_reg == -1)temp_reg = GetRegister(), recycle = 1;
	while (cur)
	{
		auto o = stoll(cur->address);
		off =temp_off+o;
		if (cur->type.type & SYMBOL_STRUCT)
			for (auto& s : GenerateStoreInstruction(cur->type, r0, r1, off, temp_reg))res.emplace_back(s);
		else {
			for (auto& s : GenerateLoadInstruction(cur->type, "$r"+to_string(temp_reg), r0, o))res.emplace_back(s);
			for (auto& s : GenerateStoreInstruction(cur->type, "$r"+to_string(temp_reg), r1, off, temp_reg))res.emplace_back(s);
		}
		cur = cur->type.ref;
	}
	if (recycle)
		RecycleRegister(temp_reg);
	return res;
}

vector<string> Translator::StoreToStack(Type type, DWord r0)
{
	DWord allocateSize = (type.type & SYMBOL_STRUCT) ? GetSymbol(type.id).size : 8;
	sp -= allocateSize;
	if (allocateSize == 8)return { "push $r" + to_string(r0) };//如果是8字节的直接push入栈里面
	vector<string>res{ "subi $sp,$sp," + to_string(allocateSize) };
	for (auto&& s : GenerateStoreInstruction(type, "$r" + to_string(r0), "$sp", 0))res.emplace_back(s);
	return res;
}

DWord Translator::GetStructMemberOffset(const string& struct_id, const string& member_id)
{
	auto& s = GetSymbol(struct_id);
	auto* cur = s.type.ref;
	while (cur)
	{
		if (cur->id == member_id)return stoull(cur->address);
	}
	return 0;
}

void Translator::WriteToTargetLval(EqualityExp* exp,DWord reg)
{
	auto* relation_exp = (RelationExp*)exp->exp.first.front();
	auto* unary_exp = (UnaryExp*)relation_exp->exp.first.front();
	auto* mul_exp = (MulExp*)unary_exp->exp.first.front();
	auto* postfix_exp = (PostFixExp*)mul_exp->exp.first.front();
	auto* primary_exp = (PrimaryExp*)postfix_exp->exp.first;
	auto symbol = GetSymbol(((ID*)(primary_exp))->tk.literal);
	auto* cur = &symbol;
	DWord off = 0;
	for (auto& node : postfix_exp->exp.second)
	{
		auto& member_id = ((VisitMember*)node)->tk.literal;
		off += GetStructMemberOffset(cur->type.id, member_id);
		auto& symbol = GetSymbol(member_id);
		cur = &symbol;
	}
	//////////////////////////
	if (symbol.address.find("ds") == string::npos)
	{
		DWord addr = stoull(symbol.address)+off;
		for (auto& s : GenerateStoreInstruction(cur->type, "$r" + to_string(reg), "$bp", addr))AddToTextSection(s);
	}
	else {
		auto r0 = GetRegister();
		AddToTextSection(LoadDSAddress(symbol.address, r0));
		for (auto& s : GenerateStoreInstruction(cur->type, "$r" + to_string(reg), "$r"+to_string(r0), off));
		RecycleRegister(r0);
	}	
}

string Translator::LoadDSAddress(const string& s0, DWord r0)
{
	return "la " + ("$r" + to_string(r0)) + "," + s0;
}

string Translator::GenerateLabel()
{
	static const string label = "520NiuNiu";
	return label + to_string(++label_cnt);
}

int64_t Translator::AlignAddress(int64_t addr, Type tp)
{
	if ((tp.type & SYMBOL_BOOL) || (tp.type & SYMBOL_CHAR))return addr;
	return ((addr - 8) / 8 + 1)*8;
}

string Translator::IntToDouble(const string& r0)
{
	return "CITD " + r0;
}

string Translator::DoubleToInt(const string& r0)
{
	return "CDTI " + r0;
}

string Translator::AllocateStackBuffer(DWord s)
{
	sp -= s;
	return "subi $sp,$sp," + to_string(s);
}

string Translator::FreeStackBuffer(DWord s)
{
	sp += s;
	return "addi $sp,$sp," + to_string(s);
}

vector<string> Translator::SaveUsedRegisters()
{
	vector<string>res;
	for (auto reg : used_register)
	{
		res.push_back("push $r" + to_string(reg));
	}
	return res;
}

vector<string> Translator::LoadUsedRegisters()
{
	vector<string>res;
	for (auto itr=used_register.rbegin();itr!=used_register.rend();++itr)
	{
		res.push_back("pop $r" + to_string(*itr));
	}
	return res;
}

string Translator::GetFunctionArg(DWord r,DWord idx)
{
	idx=(idx+2)*8;
	return GenerateLoadInstruction({SYMBOL_INT},"$r"+to_string(r),"$bp",idx).front();
}

void Translator::ResumeSymbolStack()
{
	auto itr = prev(AllSymbol.end());
	while (*itr != "return")itr = prev(itr);//使用return作为哨兵
	for (auto itr_ = next(itr); itr_ != AllSymbol.end(); ++itr_)
		RemoveSymbol(*itr_);
	AllSymbol.erase(itr, AllSymbol.end());
}

void Translator::GenerateTempSymbol()
{
	AllSymbol.push_back("return");
}

string Translator::GetFinalTranslatedCode(bool indent=0)
{
	auto CheckIsLabel = [&](const string& str)->bool {
		if (str.find(":") != string::npos)return 1;
		return 0;
		};
	auto CheckIsEndLabel = [&](const string& s)->bool {
		if (CheckIsLabel(s) == 0)return 0;
		return s.substr(s.size() - 4, 3) == "end";
		};
	auto GenerateEndLabel = [&](const string& s)->string {
		return s.substr(0, s.size() - 1) + "_end:";
		};
	string res;
	res += ".data";
	for (auto& s : DataSection)res += '\n' + s;
	res += "\n" + string(".text");
	int tab = 0;
	set<string>st;
	for (auto& s : *TextSection)if (CheckIsEndLabel(s))st.insert(s);
	for (auto& s : *TextSection)
	{
		bool flag = CheckIsLabel(s);
		if (flag && st.count(s))--tab;
		res += "\n" + (indent ? string(tab, '\t') : "") + s;
		if (flag && st.count(GenerateEndLabel(s)))++tab;
	}
	return res;
}

void Translator::ExportToFile(const string& file, bool indent = 0)
{
	
	ofstream f(file);
	f << GetFinalTranslatedCode(indent);
	f.close();
}

Translator::~Translator()
{
	if(TextSection)
	delete TextSection;
}



void Translator::ParseDeclration(Declaration* node)
{
	Type type = ParseType(node->declaration.first);
	int64_t size = GetTypeSize(type);
	for (auto& [id,subNode] : node->declaration.second)
	{
		Symbol s{ type,id,size };
		//如果是局部变量要先开辟空间
		if (!IsGLobal)
		{
			AddToTextSection("addi $sp,$sp,-" + to_string(size));
			sp -= s.size;
			s.address = to_string(sp);
		}
		else s.address = AddToDataSection(s.size, 0); //在ds段开辟空间
		if (subNode) {//如果有初始化
			auto&& [r, tp] = ParseAssignmentExp((AssignmentExp*)subNode, 1);
			if (IsGLobal) {
				auto address_id = GetRegister();
				AddToTextSection(LoadDSAddress(s.address, address_id));
				AddToTextSection(GenerateStoreInstruction(s.type, "$r" + to_string(r), "$r" + to_string(address_id), 0));
				RecycleRegister(address_id);
			}
			else AddToTextSection(GenerateStoreInstruction(s.type, "$r" + to_string(r), "$bp", stoll(s.address)));
			RecycleRegister(r);
		}
		AddSymbol(s.id, s);
	}
}

//////////////////////////////////////////////////栈帧以及入栈参数由函数自己清理
void Translator::ParseFunction(Function* node)
{
	//////////////////////////////进入函数分析，所有变量不是全局的了
	IsGLobal = 0;
	//////////////////////////////
	Symbol symbol;
	auto&& retType = ParseType(get<0>(node->fun));
	auto&& id = get<1>(node->fun);
	curFun = id;
	AddSymbol(id, { {SYMBOL_FUNCTION,id,new Symbol({retType})},id});
	auto& funSymbol = GetSymbol(id);
	GenerateTempSymbol();
	DWord offset = (retType.type & SYMBOL_STRUCT) ? 24 : 16;
	for (auto& member : get<2>(node->fun))
	{
		auto& [tp, name] = member->member;
		auto&& type = ParseType(tp);
		auto size = GetTypeSize(type);
		AddSymbol(name, { type,name,size,to_string(offset) });
		offset += size;
	}
	funSymbol.size = offset - 16;
	///////////////////////////////
	AddToTextSection(id + ":");
	/////////////////////////////生成栈帧
	AddToTextSection("push $bp");
	AddToTextSection("move $bp,$sp");
	///////////////////////////////
	bp = sp = 0;
	///////////////////////////////
	ParseComposedStatement((ComposedStatement*)get<3>(node->fun));
	///////////////////////////////恢复栈帧
	AddToTextSection(curFun + "_ret:");
	AddToTextSection("move $sp,$bp");
	AddToTextSection("pop $bp");
	AddToTextSection("ret "+to_string(offset-16));
	AddToTextSection(curFun + "_end:");
	ResumeSymbolStack();
	//////////////////////////////
	IsGLobal = 1;
}

void Translator::ParseComposedStatement(ComposedStatement* node)
{
	GenerateTempSymbol();
	//////////////////////////////////////////
	for (auto& statement : node->statement)
		ParseStatement(statement);
	//////////////////////////////////////////
	ResumeSymbolStack();
}

void Translator::ParseStatement(Node* statement)
{
	if (statement->type == NODE_COMPOSEDSTATEMENT)ParseComposedStatement((ComposedStatement*)statement);
	else if (statement->type == NODE_IFSTATEMENT)ParseIfStatement((IfStatement*)statement);
	else if (statement->type == NODE_RETURNSTATEMENT)ParseReturnStatement((ReturnStatement*)statement);
	else if (statement->type == NODE_BREAKSTATEMENT)ParseBreakStatement(statement);
	else if (statement->type == NODE_CONTINUESTATEMENT)ParseContinueStatement(statement);
	else if (statement->type == NODE_FORSTATEMENT)ParseForStatement((ForStatement*)statement);
	else if (statement->type == NODE_WHILESTATEMENT)ParseWhileStatement((WhileStatement*)statement);
	else if (statement->type == NODE_EXPRESSION)ParseExpression((Expression*)statement, 0);
	else if (statement->type == NODE_DECLARATION)ParseDeclration((Declaration*)statement);
	else if (statement->type == NODE_ASSEMBLESTATEMENT)ParseAssembleStatement((AssembleStatement*)statement);

}

void Translator::ParseIfStatement(IfStatement* node)
{
	GenerateTempSymbol();
	auto&& [r0, type] = ParseExpression((Expression*)get<0>(node->statement),1);
	RecycleRegister(r0);
	auto&& label0 = GenerateLabel();
	AddToTextSection("beqz " + ("$r" + to_string(r0)) + "," + label0);
	ParseStatement(get<1>(node->statement));
	if (get<2>(node->statement))
	{
		auto&& label1 = GenerateLabel();
		AddToTextSection("b " + label1);
		AddToTextSection(label0+":");
		ParseStatement(get<2>(node->statement));
		AddToTextSection(label1+":");
	}
	else AddToTextSection(label0+":");
	ResumeSymbolStack();
}

void Translator::ParseReturnStatement(ReturnStatement* statement)
{
	if (statement->exp) {
		auto&& [r, type] = ParseExpression((Expression*)statement->exp, 1);
		if (type.type & SYMBOL_STRUCT)//如果返回值是结构体那么要把返回值写入指定地址
		{
			AddToTextSection(GenerateLoadInstruction({ SYMBOL_INT }, "$ret", "$bp", 16));
			AddToTextSection(GenerateStoreInstruction(type, "$r" + to_string(r), "$ret", 0));
		}
		else AddToTextSection("move $ret," + ("$r" + to_string(r)));
		RecycleRegister(r);
	}
	AddToTextSection("b " + curFun + "_ret");
}

void Translator::ParseBreakStatement(Node* node)
{
	AddToTextSection("b " + curLoop + "_end");
}

void Translator::ParseContinueStatement(Node* node)
{
	AddToTextSection("b " + curLoop);
}

void Translator::ParseWhileStatement(WhileStatement* statement)
{
	GenerateTempSymbol();
	string preloop = curLoop;
	auto&& label0 = GenerateLabel();
	curLoop = label0;
	//////////////////////////////////
	AddToTextSection(label0 + ":");
	auto&& [r, type] = ParseExpression((Expression*)statement->statement.first,1);
	RecycleRegister(r);
	AddToTextSection("beqz " + ("$r" + to_string(r)) + "," + label0 + "_end");
	ParseStatement(statement->statement.second);
	AddToTextSection("b " + label0);
	AddToTextSection(label0 + "_end:");
	/////////////////////////////
	curLoop = preloop;
	ResumeSymbolStack();
}

void Translator::ParseForStatement(ForStatement* statement)
{
	GenerateTempSymbol();
	auto&& label0 = GenerateLabel();
	auto preloop = curLoop;
	curLoop = label0;
	/////////////////////////////
	ParseExpression((Expression*)get<0>(statement->statement),0);
	AddToTextSection(label0 + ":");
	auto&& [r, type] = ParseExpression((Expression*)get<1>(statement->statement), 1);
	RecycleRegister(r);
	AddToTextSection("beqz " + ("$r" + to_string(r)) + "," + label0 + "_end");
	ParseStatement(get<3>(statement->statement));
	ParseExpression((Expression*)get<2>(statement->statement), 0);
	AddToTextSection("b " + label0);
	AddToTextSection(label0 + "_end:");
	/////////////////////////////
	curLoop = preloop;
	ResumeSymbolStack();
}

void Translator::ParseAssembleStatement(AssembleStatement* statement)
{
	AddToTextSection(statement->statement);
}

pair<DWord,Type> Translator::ParseAssignmentExp(AssignmentExp* node, bool needRet)
{
	auto& exp = node->exp;
	if (exp.size() > 1)
	{
		auto&& [r,type] = ParseEqualityExp((EqualityExp*)exp.back(), 1);
		for (int i = exp.size() - 2; i >= 0; --i)
		{
			WriteToTargetLval((EqualityExp*)exp[i],r);
		}
		RecycleRegister(r);
	}
	return ParseEqualityExp((EqualityExp*)exp.front(),needRet);
}

pair<DWord,Type> Translator::ParseEqualityExp(EqualityExp* node, bool needRet)
{
	auto [ret,type] = ParseRelationExp((RelationExp*)node->exp.first.front(), needRet||node->exp.second.size());
	string r0 = "$r" + to_string(ret);
	for (int i = 1; i < node->exp.first.size(); ++i)
	{
		auto op = node->exp.second[i - 1];
		auto&&[r,tp]= ParseRelationExp((RelationExp*)node->exp.first[i],1);
		auto r1 = "$r" + to_string(r);
		string ins;
		if (op == TOOKEN_ORB)ins = "sor";
		else if (op == TOOKEN_ANDB)ins = "sand";
		else if (op == TOOKEN_NE)ins = "sne";
		else if (op == TOOKEN_EQ)ins = "seq";
		if (tp.type&SYMBOL_DOUBLE)ins += "d";
		AddToTextSection(ins + " " + r0 + "," + r0 + "," + r1);
		RecycleRegister(r);
	}
	if (!needRet)RecycleRegister(ret);
	return { ret,type};
}

pair<DWord,Type> Translator::ParseRelationExp(RelationExp* node, bool needRet)
{
	auto&&[ret,type] = ParseUnaryExp((UnaryExp*)node->exp.first.front(), needRet||node->exp.second.size());
	string r0 = "$r" + to_string(ret);
	for (int i = 1; i < node->exp.first.size(); ++i)
	{
		auto op = node->exp.second[i - 1];
		auto&& [r,tp] = ParseUnaryExp((UnaryExp*)node->exp.first[i], 1);
		auto r1 = "$r" + to_string(r);
		string ins;
		if (op == TOOKEN_LT)ins = "slt";
		else if (op == TOOKEN_GT)ins = "sgt";
		else if (op == TOOKEN_LE)ins = "sle";
		else if (op == TOOKEN_GE)ins = "sge";
		if (tp.type&SYMBOL_DOUBLE)ins += "d";
		AddToTextSection(ins + " " + r0 + "," + r0 + "," + r1);
		RecycleRegister(r);
	}
	if (!needRet)RecycleRegister(ret);
	return { ret,type};
}

pair<DWord,Type> Translator::ParseUnaryExp(UnaryExp* node, bool needRet)
{
	auto&&[ret,type] = ParseMulExp((MulExp*)node->exp.first.front(), needRet || node->exp.second.size());
	string r0 = "$r" + to_string(ret);
	for (int i = 1; i < node->exp.first.size(); ++i)
	{
		auto op = node->exp.second[i - 1];
		auto&& [r,tp] = ParseMulExp((MulExp*)node->exp.first[i], 1);
		auto r1 = "$r" + to_string(r);
		string ins;
		if (op == TOOKEN_LT)ins = "slt";
		else if (op == TOOKEN_ADD)ins = "add";
		else if (op == TOOKEN_MINUS)ins = "sle";
		if (tp.type&SYMBOL_DOUBLE)ins += "d";
		AddToTextSection(ins + " " + r0 + "," + r0 + "," + r1);
		RecycleRegister(r);
	}
	if (!needRet)RecycleRegister(ret);
	return { ret,type };
}

pair<DWord, Type> Translator::ParseMulExp(MulExp* node, bool needRet)
{
	auto&& [ret, type] = ParsePostFixExp((PostFixExp*)node->exp.first.front(), needRet || node->exp.second.size());
	string r0 = "$r" + to_string(ret);
	for (int i = 1; i < node->exp.first.size(); ++i)
	{
		auto op = node->exp.second[i - 1];
		auto&& [r, tp] = ParsePostFixExp((PostFixExp*)node->exp.first[i], 1);
		auto r1 = "$r" + to_string(r);
		string ins;
		if (op == TOOKEN_MULT)ins = "mult";
		else if (op == TOOKEN_DIV)ins = "div";
		else if (op == TOOKEN_REM)ins = "rem";
		if (tp.type&SYMBOL_DOUBLE)ins += "d";
		AddToTextSection(ins + " " + r0 + "," + r0 + "," + r1);
		RecycleRegister(r);
	}
	if (!needRet)RecycleRegister(ret);
	return { ret,type };
}

pair<DWord, Type> Translator::ParsePostFixExp(PostFixExp* node, bool needRet)
{
	auto&& [ret, type] = ParsePrimaryExp((PrimaryExp*)node->exp.first,needRet||node->exp.second.size());
	if (node->exp.second.size()) {
		auto& symbol = GetSymbol(((ID*)(node->exp.first))->tk.literal);
		auto* cur = &symbol;
		int64_t offset = 0;
		for (auto& exp : node->exp.second)
		{
			if (exp->type == NODE_ARGUMENTEXPLIST)
			{
				///////////////////////////保存重要寄存器
				RecycleRegister(ret);//当前寄存器不需要压栈
				AddToTextSection(SaveUsedRegisters());
				//////////////////////////如果函数的返回值的类型是一个结构体，提前开辟空间
				DWord retStructSize = 0,reg;
				if (symbol.type.ref->type.type &SYMBOL_STRUCT)
				{
					auto& s = GetSymbol(symbol.type.ref->type.id);
					retStructSize = s.size;
					AddToTextSection(AllocateStackBuffer(retStructSize));
					reg = GetRegister();
					AddToTextSection("move " + ("$r" + to_string(reg) + "," + "$sp"));
				}
				/////////////////////////////////////传参
				auto* e = (ArgumentExpList*)exp;
				for (auto itr=e->exp.rbegin();itr!=e->exp.rend();++itr)
				{
					auto* exp = (AssignmentExp*)(*itr);
					auto&&[r,type]=ParseAssignmentExp(exp,1);
					AddToTextSection(StoreToStack(type, r));
					RecycleRegister(r);
				}
				if (retStructSize)
				{
					//压入返回的结构体地址
					AddToTextSection("push " + ("$r" + to_string(reg)));
					RecycleRegister(reg);
				}
				/////////////////////////////////调用
				AddToTextSection("call " + symbol.id);
				/////////////////////////////////更新当前的返回类型
				type = symbol.type.ref->type;
				cur = &GetSymbol(symbol.type.ref->type.id);
				/////////////////////////////////恢复重要寄存器
				AddToTextSection(LoadUsedRegisters());
				UseRegister(ret);
				if((type.type&SYMBOL_VOID)==0)//如果返回值不是空类型
				AddToTextSection("move $r" + to_string(ret) + ",$ret");//$ret存返回值
			}
			else if (exp->type == NODE_VISITMEMBER)
			{
				DWord o=GetStructMemberOffset(cur->type.id, ((VisitMember*)(exp))->tk.literal);
				offset += o;
				string rr = "$r" + to_string(ret);
				AddToTextSection("addi " +rr+ "," +rr+ "," + to_string(o));
				cur = &GetSymbol(((VisitMember*)(exp))->tk.literal);
				type = cur->type;
				if ((type.type&SYMBOL_STRUCT) == 0)
				{
					GenerateLoadInstruction({ type },rr, rr, 0);
				}
			}
		}
	}
	if (!needRet)RecycleRegister(ret);
	return { ret, type };
}

pair<DWord, Type> Translator::ParsePrimaryExp(PrimaryExp* node, bool needRet)
{
	if (!needRet)return { 0,{SYMBOL_VOID} };
	auto ret = GetRegister();
	string r = "$r" + to_string(ret);
	auto* exp = node;
	Type type;
	if (exp->type == NODE_INTEGER) {
		AddToTextSection("li "+r+ "," + to_string(((Integer*)(exp))->val));
		type.type=SYMBOL_INT;
	}
	else if (exp->type == NODE_BOOL)
	{
		AddToTextSection("li " + r + "," + to_string(((Bool*)exp)->val));
		type.type =SYMBOL_BOOL;
	}
	else if (exp->type == NODE_CHAR)
	{
		AddToTextSection("li " + r + "," + to_string(((Char*)exp)->ch));
		type.type = SYMBOL_CHAR;
	}
	else if (exp->type == NODE_DOUBLE)
	{
		AddToTextSection("li " + r + "," + to_string(((Double*)exp)->val));
		type.type = SYMBOL_DOUBLE;
	}
	else if (exp->type == NODE_STRING)
	{
		auto&& id = AddToDataSection(((String*)exp)->str);
		AddToTextSection("la " + r + "," + id);
		type.type = SYMBOL_STRING;
	}
	else if (exp->type == NODE_ID) {
		auto& symbol = GetSymbol(((ID*)exp)->tk.literal);
		type = symbol.type;
		if ((type.type & SYMBOL_FUNCTION) == 0)
		{
			if (type.type & SYMBOL_STRUCT)AddToTextSection("addi " + r + "," + "$bp" + "," + symbol.address);
			else AddToTextSection(GenerateLoadInstruction(symbol.type, r, "$bp", stoll(symbol.address)));
		}
	}
	else if (exp->type == NODE_EXPRESSION)
	{
		auto&& [reg, tp] = ParseExpression((Expression*)exp, 1);
		AddToTextSection("move " + r + "," + "$r" + to_string(reg));
		type = tp;
		RecycleRegister(reg);
	}
	return { ret,type };
}

pair<DWord, Type> Translator::ParseExpression(Expression* node, bool needRet)
{
	auto&& [reg, type] = ParseAssignmentExp((AssignmentExp*)node->exp.front(), needRet);
	for (auto itr = node->exp.begin() + 1; itr != node->exp.end(); ++itr)
	{
		ParseAssignmentExp((AssignmentExp*)(*itr), 0);
	}
	return { reg,type };
}

Symbol& Translator::GetSymbol(const string& id)
{
	return symbol[id]->back();
}

Type Translator::ParseType(const string& s)
{
	const static string tar = "struct";
	static  unordered_map<string, SymbolType>mp{
		{"int",SYMBOL_INT},
		{"bool",SYMBOL_BOOL},
		{"char",SYMBOL_CHAR},
		{"double",SYMBOL_DOUBLE},
		{"string",SYMBOL_STRING},
	};
	int idx = s.find(tar);
	if (idx == string::npos) {
		auto type = mp[s];
		return { type };
	}
	return { SYMBOL_STRUCT,s.substr(idx + tar.size() + 1,s.size()) };
}

DWord Translator::GetTypeSize(Type t)
{
	static unordered_map<SymbolType, DWord>TypeSize{
		{SYMBOL_VOID,0},
		{SYMBOL_BOOL,1},
		{SYMBOL_CHAR,1},
		{SYMBOL_INT,8},
		{SYMBOL_DOUBLE,8},
		{SYMBOL_STRING,8}
	};
	if (TypeSize.count(t.type))return TypeSize[t.type];
	auto s = GetSymbol(t.id);
	return s.size;
}

void Translator::AddToTextSection(const string&s)
{
	TextSection->emplace_back(s);
}

void Translator::AddToTextSection(const vector<string>&v)
{
	for (auto& s : v)AddToTextSection(s);
}
