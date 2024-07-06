#include "Lexier.h"
Lexier::Lexier(const char* file_)
{
	/////////////////////////
	row = 1;
	col = 1;
	point = 0;
	/////////////////////////
	file = file_;
	ifstream f(file,ios_base::binary);
	if (f.is_open() == 0)
	{
		cout << "No Such File!" << endl;
		exit(0);
	}
	f.seekg(0, SEEK_END);
	auto size=f.tellg();
	f.seekg(0, SEEK_SET);
	data.resize(uint32_t(size)+1);
	f.read(&data[0], data.size());
	data.back()='\n';
	f.close();
}

void Lexier::parse()
{
	while (GetChar())
	{
		SkipSpace();
		char ch = GetChar();
		if (ch ==0)break;
		/////////////////////////
		if (ch == '+')AddTooken(TOOKEN_ADD, "+");
		else if (ch == '-')AddTooken(TOOKEN_MINUS, "-");
		else if (ch == '*')AddTooken(TOOKEN_MULT, "*");
		else if (ch == '/')AddTooken(TOOKEN_DIV, "/");
		else if (ch == '%')AddTooken(TOOKEN_REM, "%");
		else if (ch == '&') {
			auto nxt = AdvanceChar();
			if (nxt == '&') {
				AddTooken(TOOKEN_ANDB, "&&");
				NextChar();
			}
			else AddTooken(TOOKEN_ANDL, "&");
		}
		else if (ch == '|')
		{
			auto nxt = AdvanceChar();
			if (nxt == '|') {
				AddTooken(TOOKEN_ORB, "||");
				NextChar();
			}
			else AddTooken(TOOKEN_ORL, "|");
		}
		else if (ch == '~')AddTooken(TOOKEN_NOTL, "~");
		else if (ch == '^')AddTooken(TOOKEN_XOR, "^");
		else if (ch == '<')
		{
			auto nxt = AdvanceChar();
			if (nxt == '<')
			{
				AddTooken(TOOKEN_SHIFTL, "<<");
				NextChar();
			}
			else if (nxt == '=')
			{
				AddTooken(TOOKEN_LE, "<=");
				NextChar();
			}
			else AddTooken(TOOKEN_LT, "<");
		}
		else if (ch == '>')
		{
			auto nxt = AdvanceChar();
			if (nxt == '>')
			{
				AddTooken(TOOKEN_SHIFTR, ">>");
				NextChar();
			}
			else if (nxt == '=')
			{
				AddTooken(TOOKEN_GE, ">=");
				NextChar();
			}
			else AddTooken(TOOKEN_GT, ">");
		}
		else if (ch == '!')
		{
			char ch = AdvanceChar();
			if (ch == '=')
			{
				AddTooken(TOOKEN_NE, "!=");
				NextChar();
			}
			else AddTooken(TOOKEN_NOTB, "!");
		}
		else if (ch == '=')
		{
			auto nxt = AdvanceChar();
			if (nxt == '=')
			{
				AddTooken(TOOKEN_EQ, "==");
				NextChar();
			}
			else AddTooken(TOOKEN_ASSIGN, "=");
		}
		else if (ch == '(')AddTooken(TOOKEN_LPAR, "(");
		else if (ch == ')')AddTooken(TOOKEN_RPAR, ")");
		else if (ch == '[')AddTooken(TOOKEN_LSUQ, "[");
		else if (ch == ']')AddTooken(TOOKEN_RSUQ, "]");
		else if (ch == '{')AddTooken(TOOKEN_LBRA, "{");
		else if (ch == '}')AddTooken(TOOKEN_RBRA, "}");
		else if (ch == '.')AddTooken(TOOKEN_DOT, ".");
		else if (ch == '\'')
		{
			char nxt;
			string target;
			int i = 1;
			for (;; ++i)
			{
				nxt = AdvanceNChar(i);
				if (nxt == 0)Error();
				if (nxt == '\'')break;
				target += nxt;
				if (nxt == '\\')target += AdvanceNChar(++i);
			}
			auto&& res = ParseString(target);
			if (res.size() != 1)Error();//如果解析出来字符大小不为1则爆错
			AddTooken(TOOKEN_CHAR,res);
			NextNChar(i);
		}
		else if (ch == '\"') {
			char nxt;
			string target;
			int i = 1;
			for (;; ++i)
			{
				nxt = AdvanceNChar(i);
				if (nxt == 0)Error();
				if (nxt == '\"')break;
				target += nxt;
				if (nxt == '\\')target += AdvanceNChar(++i);
			}
			AddTooken(TOOKEN_STRING, ParseString(target));
			NextNChar(i);
		}
		else if (ch == ';')AddTooken(TOOKEN_SEM,";");
		else if (ch == ',')AddTooken(TOOKEN_COMMA,",");
		else if (ch == '#')
		{
			while (AdvanceChar() != '\n')NextChar();
		}
		else if (isdigit(ch)) {
			string res;
			int i = 0;
			bool is_double = 0;
			for (;; ++i)
			{
				auto ch = AdvanceNChar(i);
				if (ch == '.' && !is_double)
				{
					is_double = 1;
					res += '.';
				}
				else if (isdigit(ch))res += ch;
				else break;
			}
			AddTooken(is_double ? TOOKEN_DOUBLE : TOOKEN_INTEGER, res);
			NextNChar(i - 1);
		}
		else if (ch == '_' || isalpha(ch))
		{
			string res;
			int i = 0;
			for (;;++i)
			{
				ch = AdvanceNChar(i);
				if (isalpha(ch) || ch == '_' || isdigit(ch))res += ch;
				else break;
			}
			if (res == "true" || res == "false")AddTooken(TOOKEN_BOOL, res);
			else if (KeyWords.count(res))
				AddTooken(TOOKEN_KEYWORD, res);
			else AddTooken(TOOKEN_ID, res);
			NextNChar(i - 1);
		}
		else Error();
		/////////////////////////////////////////
		NextChar();
	}
}

char Lexier::NextChar()
{
	++point;
	if (point>= data.size())return 0;
	++col;
	return data[point];
}

char Lexier::NextNChar(uint32_t n)
{
	point += n;
	if (point>= data.size())return 0;
	col += n;
	return data[point];
}

void Lexier::RollBack()
{
	if (point >= data.size())return;
	--col;
	--point;
}

char Lexier::AdvanceChar()
{
	if (point + 1 >= data.size())return 0;
	return data[point + 1];
}

char Lexier::AdvanceNChar(uint32_t n)
{
	if (point + n >= data.size())return 0;
	return data[point + n];
}

char Lexier::GetChar()
{
	if (point >= data.size())return 0;
	return data[point];
}

void Lexier::SkipSpace()
{
	char cur = GetChar();
	while (1)
	{
		if (cur == ' '||cur=='\t'|| cur == '\n' || cur == '\r')
		{
			if (cur == '\n') {
				++row;
				col = 0;
			}
			cur = NextChar();
		}
		else break;
	}
}

bool Lexier::IsEmpty(char ch)
{
	if (ch==0||ch == ' ' || ch == '\n' || ch == '\r'||ch=='\t')return 1;
	return 0;
}

void Lexier::AddTooken(TookenType type)
{
	tookens.emplace_back(row, col, file, type);
}

void Lexier::AddTooken(TookenType type, const string& literal)
{
	tookens.emplace_back(row, col, file, type,literal);
}

void Lexier::Error()
{
	cout << "Lexier Pase Error in file: " << file << " row: " << row << " col: " << col << endl;
	exit(0);
}

string Lexier::ParseString(string& str)
{
	int idx = 0;
	string res;
	while (idx < str.size())
	{
		auto ch = str[idx];
		if (ch == '\\')
		{
			auto nxt = str[idx + 1];
			if (nxt == 'n')res += '\n';
			else if (nxt == 'r')res += '\r';
			else if (nxt == '0')res += '\0';
			else if (nxt == 'a')res += '\a';
			else if (nxt == 't')res += '\t';
			else res += nxt;
			++idx;
		}
		else res += ch;
		++idx;
	}
	return res;
}
