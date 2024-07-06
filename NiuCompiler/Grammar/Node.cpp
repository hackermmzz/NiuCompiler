#include "Node.h"

Node::Node(NodeType tp, void*d=0)
{
	data = d;
	type = tp;
}

NodeRet Node::show()
{
	return {};
}


Program::Program():Node(NODE_PROGRAM,&SubNode)
{

}

NodeRet Program::show()
{
	NodeRet ret;
	ret.first = "Program";
	auto& res = ret.second;
	int idx = 1;
	for (auto& node : SubNode)
	{
		auto&& r = node->show();
		res.AddJson(r.first+"_"+to_string(idx++), r.second);
	}
	return ret;
	
}

Member::Member(string&type, string&id):Node(NODE_MEMBER,&member)
{
	member = { type,id };
}

NodeRet Member::show()
{
	string type="Member";
	Json res;
	res.AddString("Type", member.first);
	res.AddString("ID", member.second);
	return NodeRet{ type,res };
}

Struct::Struct():Node(NODE_STRUCT,&declaration_list)
{
}

NodeRet Struct::show()
{
	string kind = "Struct";
	Json res;
	res.AddString("Struct ID", declaration_list.first);
	int idx = 1;
	for (auto& member : declaration_list.second)
	{
		auto&&r=member->show();
		res.AddJson("Member_" + to_string(idx++), r.second);
		
	}
	return { kind,res };
}

Function::Function():Node(NODE_FUNCTION,&fun)
{

}

NodeRet Function::show()
{
	string type = "Function";
	Json json;
	json.AddString("Return Type", get<0>(fun));
	json.AddString("Function ID", get<1>(fun));
	int idx = 1;
	for (auto& member : get<2>(fun))
	{
		auto&&r=member->show();
		json.AddJson("Argument_" + to_string(idx++), r.second);
	}
	json.AddJson("FunBody", get<3>(fun)->show().second);
	return {type,json};
}

Declaration::Declaration():Node(NODE_DECLARATION,&declaration)
{
}

NodeRet Declaration::show()
{
	string kind = "Declaration";
	Json json;
	json.AddString("Type", declaration.first);
	int idx = 1;
	for (auto&[name,node]:declaration.second)
	{	
		json.AddString("ID_"+to_string(idx), name);
		json.AddJson("InitialExp_"+to_string(idx), node->show().second);
		++idx;
	}
	return { kind,json };
}

AssignmentExp::AssignmentExp():Node(NODE_ASSIGNMENTEXP,&exp)
{

}

NodeRet AssignmentExp::show()
{
	string kind = "AssignmentExp";
	Json json;
	int idx = 1;
	for (auto& node : exp)
	{
		auto&& r = node->show();
		json.AddJson(r.first+"_"+to_string(idx++), r.second);
	}
	return { kind,json };
}

EqualityExp::EqualityExp():Node(NODE_EQUALITYEXP,&exp)
{
}

NodeRet EqualityExp::show()
{
	return ShowNodeAndTookenType("EquilityExp",exp);
}

RelationExp::RelationExp():Node(NODE_RELATIONEXP,&exp)
{
}

NodeRet RelationExp::show()
{
	return ShowNodeAndTookenType("RelationExp", exp);
}

MulExp::MulExp():Node(NODE_MULEXP,&exp)
{

}

NodeRet MulExp::show()
{
	return ShowNodeAndTookenType("MulExp", exp);
}

UnaryExp::UnaryExp():Node(NODE_UNARYEXP,&exp)
{

}

NodeRet UnaryExp::show()
{
	return ShowNodeAndTookenType("UnaryExp", exp);
}

PostFixExp::PostFixExp():Node(NODE_POSTFIXEXP,&exp)
{
}

NodeRet PostFixExp::show()
{
	string kind="PostFixExp";
	Json json;
	auto&&r=exp.first->show();
	json.AddJson(r.first, r.second);
	int cnt = 1;
	for (auto& node : exp.second)
	{
		auto&& r = node->show();
		json.AddJson(r.first + "_" + to_string(cnt++), r.second);
	}
	return { kind,json };
}

PrimaryExp::PrimaryExp():Node(NODE_PRIMARYEXP,exp)
{

}

NodeRet PrimaryExp::show()
{
	return exp->show();
}

ID::ID(const Tooken&t):tk(t),Node(NODE_ID,&tk)
{
}

NodeRet ID::show()
{
	string kind = "ID";
	Json json;
	json.AddString("Name", tk.literal);
	return {kind,json};
}

Integer::Integer(const Tooken& tk):Node(NODE_INTEGER,&val)
{
	val = stoll(tk.literal);
}

NodeRet Integer::show()
{
	string kind = "Integer";
	Json json;
	json.AddInt("Val", val);
	return { kind,json };
}

String::String(const Tooken&t):Node(NODE_STRING,&str)
{
	str = t.literal;
}

NodeRet String::show()
{
	string kind = "String";
	Json json;
	json.AddString("Str", str);
	return { kind,json };
}

Double::Double(const Tooken&t):Node(NODE_DOUBLE,&val)
{
	val = stod(t.literal);
}

NodeRet Double::show()
{
	Json json;
	json.AddDouble("Val", val);
	return { "Double",json };
}

Char::Char(const Tooken&t):Node(NODE_CHAR,&ch)
{
	ch = t.literal.front();
}

NodeRet Char::show()
{
	Json json;
	string s;
	s += ch;
	json.AddString("Val",s);
	return { "Char", json };
}

Bool::Bool(const Tooken&t):Node(NODE_BOOL,&val)
{
	val = t.literal == "true" ? 1 : 0;
}

NodeRet Bool::show()
{
	Json json;
	json.AddString("Val", val ? "true" : "false");
	return { "Bool",json };
}

Expression::Expression():Node(NODE_EXPRESSION,&exp)
{

}

NodeRet Expression::show()
{
	string kind = "Expression";
	Json json;
	int idx = 1;
	for (auto& node : exp)
	{
		auto&& r = node->show();
		json.AddJson(r.first+"_"+to_string(idx++), r.second);
	}
	return { kind,json };
}

ComposedStatement::ComposedStatement():Node(NODE_COMPOSEDSTATEMENT,&statement)
{
}

NodeRet ComposedStatement::show()
{
	Json json;
	int idx = 1;
	for (auto& node : statement)
	{
		auto&&r=node->show();
		json.AddJson(r.first+"_"+to_string(idx++), r.second);
	}
	return { "ComposedStatement",json };
}

IfStatement::IfStatement():Node(NODE_IFSTATEMENT,&statement)
{
	statement = { 0,0,0 };
}

NodeRet IfStatement::show()
{
	Json json;
	json.AddJson("Condition", get<0>(statement)->show().second);
	json.AddJson("Body", get<1>(statement)->show().second);
	if (get<2>(statement))
		json.AddJson("Else",get<2>(statement)->show().second);
	return { "IfStatement",json };
}

ReturnStatement::ReturnStatement():Node(NODE_RETURNSTATEMENT,exp)
{
}

NodeRet ReturnStatement::show()
{
	Json json;
	auto&& r = exp->show();
	json.AddJson(r.first, r.second);
	return { "ReturnStatement",json };
}

ForStatement::ForStatement():Node(NODE_FORSTATEMENT,&statement)
{

}

NodeRet ForStatement::show()
{
	Json json;
	json.AddJson("Initalial", get<0>(statement)->show().second);
	json.AddJson("Condition", get<1>(statement)->show().second);
	json.AddJson("Update", get<2>(statement)->show().second);
	json.AddJson("Body", get<3>(statement)->show().second);
	return { "ForStatement",json };
}

WhileStatement::WhileStatement():Node(NODE_WHILESTATEMENT,&statement)
{

}

NodeRet WhileStatement::show()
{
	Json json;
	json.AddJson("Condition", statement.first->show().second);
	json.AddJson("Body", statement.second->show().second);
	return {"WhileStatement",json };
}

NodeRet ShowNodeAndTookenType(const string&kind,pair<vector<Node*>, vector<TookenType>>&exp)
{
	Json json;
	auto& v0 = exp.first;
	auto& v1 = exp.second;
	int size = v0.size();
	int idx0 = 1, idx1 = 1;
	for (int i = 0; i < size; ++i)
	{
		if (i)
		{
			auto&& str = Tooken::show(v1[i - 1]);
			json.AddString("Operation_"+to_string(idx0++), str);
		}
		auto&&r=v0[i]->show();
		json.AddJson(r.first+"_"+to_string(idx1++), r.second);
	}
	return { kind,json };
}

ArgumentExpList::ArgumentExpList():Node(NODE_ARGUMENTEXPLIST,&exp)
{
}

NodeRet ArgumentExpList::show()
{
	Json json;
	int idx = 1;
	for (auto& node : exp)
	{
		auto&& r = node->show();
		json.AddJson("Arg_" + to_string(idx++), r.second);
	}
	return { "ArgumentExpList",json };
}

VisitMember::VisitMember(const Tooken&t):tk(t),Node(NODE_VISITMEMBER,&tk)
{

}

NodeRet VisitMember::show()
{
	Json json;
	json.AddString("MemberID", tk.literal);
	return { "VisitMember",json };
}

AssembleStatement::AssembleStatement():Node(NODE_ASSEMBLESTATEMENT,&statement)
{
}

NodeRet AssembleStatement::show()
{
	Json json;
	int idx = 0;
	for (auto& s : statement)json.AddString("Instruction_" + to_string(idx++), s);
	return {"AssembleStatement",json};
}
