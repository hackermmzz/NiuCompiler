#include "Assembler.h"

Assembler::Assembler(const string& file_)
{
	point = 0;
	ifstream file(file_);
	if (file.is_open() == 0)
	{
		exit(0);
		cout << "File Not Exist!" << endl;
	}
	string temp;
	while (getline(file,temp))
	{
		code.emplace_back(temp);
	}
	file.close();
}

void Assembler::parse()
{
	////////////////////////////////////////处理静态数据段
	unordered_map<string, DWord>DsDataOffset;
	vector<byte>DsData;
	DWord DsOffset = 0;
	auto FillDsSpace = [&]()
	{
			DsData.resize(DsOffset);
	};
	auto CopyToDs = [&](void* src, DWord size)
	{
			memcpy(&DsData[DsData.size() - size], src, size);
	};
	while (GetCode() != ".text")
	{
		auto& code = GetCode();
		if (auto idx=code.find(".align");idx!=string::npos)
		{
			DsOffset = ((DsOffset - 8) / 8 + 1) * 8;
		}
		else if (auto idx = code.find(".DWord"); idx != string::npos)
		{
			auto label = code.substr(0, idx-3);
			auto data =stoull(code.substr(idx + 7, code.size()));
			DsDataOffset[label] = DsOffset;
			DsOffset += 8;
			FillDsSpace();
			CopyToDs(&data, sizeof(data));
		}
		else if (auto idx = code.find(".Byte"); idx != string::npos)
		{
			auto label = code.substr(0, idx - 3);
			auto data = byte(stoull(code.substr(idx + 6, code.size())));
			DsDataOffset[label] = DsOffset;
			DsOffset += 1;
			FillDsSpace();
			CopyToDs(&data, sizeof(data));
		}
		else if (auto idx = code.find(".Space"); idx != string::npos)
		{
			auto label = code.substr(0, idx - 3);
			auto size = stoull(code.substr(idx + 7, code.size()));
			DsDataOffset[label] = DsOffset;
			DsOffset += size;
			FillDsSpace();
		}
		else if (auto idx =code.find(".asciiz"); idx != string::npos)
		{
			auto label = code.substr(0, idx - 3);
			auto str = code.substr(idx + 9, code.size() - idx - 10);
			DsDataOffset[label] = DsOffset;
			DsOffset += str.size();
			FillDsSpace();
			CopyToDs(&str[0], str.size());
			++DsOffset;
			FillDsSpace();
		}
		///////////////////////
		++point;
	}
	////////////////////////////////////////
	auto CheckIsLabel = [&](const string& s)->bool {
		return s.find(":") != string::npos || s.find(".") != string::npos;
	};
	///////////////////////////////////////
	///////////////////////////////////////
	auto GetRegister = [&](const string& s)->int64_t {

		if (s == "$sp")return sp;
		else if (s == "$bp")return bp;
		else if (s == "$pc")return pc;
		else if (s == "$ds")return ds;
		else if (s == "$ret")return ret;
		auto idx = s.find('r');
		return stoull(s.substr(idx + 1, s.size()));
		};
	auto ParseThreeOpCode = [&](const string& s)->tuple<DWord, DWord, int64_t> {
		int idx0 = s.find_first_of(' ');
		int idx1 = s.find_first_of(',');
		int idx2 = s.find_last_of(',');
		DWord r0 = GetRegister(s.substr(idx0+1, idx1 - idx0 - 1));
		DWord r1 = GetRegister(s.substr(idx1+1,idx2-idx1-1));
		int64_t r2;
		if (s[idx2 + 1] == '$')
			r2 = GetRegister(s.substr(idx2+1, s.size()));
		else r2 = stoll(s.substr(idx2 + 1, s.size()));
		return { r0,r1,r2 };
	};
	int64_t textOffset = 0;
	map<string,int64_t>TextLabelOffset;
	vector<Instruction>TextCode;
	auto AddToTextCode = [&](InstructionType tp, DWord r0, DWord r1, int64_t r2) {
		TextCode.emplace_back(tp,r0,r1,r2);
	};
	int64_t temp_point =++point;
	while (temp_point < code.size())
	{
		auto& s = code[temp_point];
		if (CheckIsLabel(s))
		{
			auto&& ss = s.substr(0, s.size() - 1);
			if (TextLabelOffset.count(ss))Error();
			TextLabelOffset[ss] = textOffset;
		}
		else textOffset += sizeof(Instruction);
		++temp_point;
	}
	//
	textOffset = 0;
	while (point < code.size())
	{
		auto& code = GetCode();
		bool flag = CheckIsLabel(code);
		/////////////////////////////
		if(!flag){
			auto op = MapToInstruction.at(code.substr(0, code.find_first_of(' ')));
			switch(op)
			{
			case ADD:case ADDI:case ADDD:case SUB:case SUBI:case SUBD:case MULT:case MULTD:case DIV:case DIVD:case REM:case AND:case OR:case XOR:
			case SL:case SR:case SOR:case SAND:case SEQ:case SEQD:case SNE:case SNED:case SLT:case SLTD:case SGT:case SGTD:
			case SLE:case SLED:case SGE:case SGED:
			{
				auto&& [v0, v1, v2] = ParseThreeOpCode(code);
				AddToTextCode(op, v0, v1, v2);
				break;
			}
			case SB:case SDW:case LB:case LDW:
			{
				int idx1 = code.find_first_of(' ');
				int idx2 = code.find_first_of(',');
				int idx3 = code.find_first_of('(');
				int idx4 = code.find_first_of(")");
				DWord r0 = GetRegister(code.substr(idx1 + 1, idx2 - idx1 - 1));
				DWord r1 = GetRegister(code.substr(idx3 + 1, idx4 - idx3 - 1));
				int64_t r2 = stoll(code.substr(idx2 + 1, idx3 - idx2 - 1));
				AddToTextCode(op, r0, r1, r2);
				break;
			}
			case MOVE:case NOT:
			{
				int idx1 = code.find_first_of(' ');
				int idx2 = code.find_first_of(',');
				DWord r0 = GetRegister(code.substr(idx1 + 1, idx2 - idx1 - 1));
				DWord r1 = GetRegister(code.substr(idx2 + 1, code.size()));
				AddToTextCode(op, r0, r1, 0);
				break;
			}
			case PUSH:case POP:
			{
				int idx1 = code.find_first_of(' ');
				DWord r0 = GetRegister(code.substr(idx1 + 1, code.size()));
				AddToTextCode(op, r0, 0, 0);
				break;
			}
			case BEQZ:case BNEZ:
			{
				int idx1 = code.find_first_of(' ');
				int idx2 = code.find_first_of(',');
				DWord r0 = GetRegister(code.substr(idx1 + 1, idx2 - idx1 - 1));
				auto pos = TextLabelOffset[code.substr(idx2 + 1, code.size())];
				AddToTextCode(op, r0, 0, pos - textOffset-sizeof(Instruction));
				break;
			}
			case RET:
			{
				int idx0 = code.find_first_of(' ');
				int64_t d = stoll(code.substr(idx0 + 1, code.size()));
				AddToTextCode(op, 0, 0, d);
				break;
			}
			case B:
			{
				int idx = code.find_first_of(' ');
				auto pos = TextLabelOffset[code.substr(idx + 1, code.size())];
				AddToTextCode(op, 0, 0, pos - textOffset-sizeof(Instruction));
				break;
			}
			case LI:case LA:
			{
				int idx0 = code.find_first_of(' ');
				auto idx1 = code.find_first_of(',');
				DWord r0 = GetRegister(code.substr(idx0 + 1, idx1 - idx0 - 1));
				auto s = code.substr(idx1 + 1, code.size());
				if (op == LA)
				{
					int64_t res = DsDataOffset[s];
					AddToTextCode(op, r0, 0, res);
				}
				else {
					int64_t res;
					if (int idx = s.find('.'); idx != string::npos) {
						double r = stod(s);
						memcpy(&res, &r, sizeof(r));
					}
					else res = stoll(s);
					AddToTextCode(op, r0, 0, res);
				}
				break;
			}
			case SYSCALL:
				AddToTextCode(op, 0, 0, 0);
				break;
			case CALL:
			{	
				int idx = code.find_first_of(' ');
				auto pos = TextLabelOffset[code.substr(idx + 1, code.size())];
				AddToTextCode(op, 0, 0, pos - textOffset - sizeof(Instruction));
				break;
			}
			default:
				Error();
			}
			textOffset += sizeof(Instruction);
		}
		/////////////////////////////
		++point;
	}
	//////////////////////
	AssemblerData.resize(8+DsData.size()+ TextCode.size() * sizeof(Instruction));
	DWord dsSize = DsData.size();
	memcpy(&AssemblerData[0], &dsSize, 8);
	memcpy(&AssemblerData[8], &DsData[0], DsData.size());
	memcpy(&AssemblerData[DsData.size()+8], &TextCode[0], TextCode.size() * sizeof(Instruction));
}

void Assembler::Error()
{
	cout << "Error at row: " << (point + 1) << endl;
	exit(0);
}

void Assembler::ExportToFile(const string&s)
{
	ofstream file(s,ios::binary);
	file.write((char*) & AssemblerData[0], AssemblerData.size());
	file.close();
}

string& Assembler::GetCode()
{
	return code[point];
}
