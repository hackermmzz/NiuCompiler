#include"VirtualMachine.h"

VirtualMachine::VirtualMachine(const string& file_)
{
	ifstream file(file_,ios::binary);
	if (file.is_open() == 0)
	{
		cout << "File not exist!" << endl;
		exit(0);
	}
	file.seekg(0,SEEK_END);
	auto fileSize = file.tellg();
	file.seekg(0,SEEK_SET);
	vector<byte>res(fileSize);
	file.read((char*)&res[0], fileSize);
	file.close();
	////////////////////////////
	registers.resize(MaxRegister+5);
	stack.resize(stackSize/sizeof(DWord));
	DWord DataSectionSize = 0;
	memcpy(&DataSectionSize, &res[0], 8);
	DataSection.resize(DataSectionSize);
	memcpy(&DataSection[0], &res[8], DataSectionSize);
	TextSection.resize((res.size()-8-DataSectionSize) / sizeof(Instruction));
	memcpy(&TextSection[0], &res[8+DataSectionSize], TextSection.size() * sizeof(Instruction));
	registers[bp] =registers[sp]= (DWord)&stack.back();
	registers[pc] = (DWord)&TextSection.front();
	registers[ds] = (DWord)&DataSection.front();
	registers[ret] = 0;
	////////////////////////////
}

void VirtualMachine::ParseThreeRegisterInstruction(const Instruction& instruction)
{
	auto& [op, r0, r1, r2] = instruction;
	auto& r = registers;
	if (op == ADD)r[r0] = (int64_t)r[r1] + (int64_t)r[r2];
	else if (op == ADDD)*(double*)(&r[r0]) = (*(double*)(&r[r1])) + (*(double*)(&r[r2]));
	else if (op == SUB)r[r0] = (int64_t)r[r1] - (int64_t)r[r2];
	else if (op == SUBD)*(double*)(&r[r0]) = (*(double*)(&r[r1])) - (*(double*)(&r[r2]));
	else if (op == MULT)r[r0] = (int64_t)r[r1] * (int64_t)r[r2];
	else if (op == MULTD)*(double*)(&r[r0]) = (*(double*)(&r[r1])) * (*(double*)(&r[r2]));
	else if (op == DIV)r[r0] = (int64_t)r[r1] / (int64_t)r[r2];
	else if (op == DIVD)*(double*)(&r[r0]) = (*(double*)(&r[r1])) / (*(double*)(&r[r2]));
	else if (op == REM)r[r0] = (int64_t)r[r1] % (int64_t)r[r2];
	else if (op == AND)r[r0] = r[r1] & r[r2];
	else if (op == OR)r[r0] = r[r1] | r[r2];
	else if (op == XOR)r[r0] = r[r1] ^ r[r2];
	else if (op == SL)r[r0] = r[r1] << r[r2];
	else if (op == SR)r[r0] = r[r1] >> r[r2];
	else if (op == SOR)r[r0] = (int64_t)r[r1] || (int64_t)r[r2];
	else if (op == SAND)r[r0] = (int64_t)r[r1] && (int64_t)r[r2];
	else if (op == SEQ)r[r0] = r[r1] == r[r2];
	else if(op==SEQD)r[r0]= (*(double*)(&r[r1]))==(*(double*)(&r[r2]));
	else if(op==SNE)r[r0] = r[r1] != r[r2];
	else if (op == SNED)r[r0] = (*(double*)(&r[r1])) != (*(double*)(&r[r2]));
	else if(op==SLT)r[r0] = (int64_t)r[r1]<(int64_t)r[r2];
	else if(op==SLTD)r[r0] = (*(double*)(&r[r1]))<(*(double*)(&r[r2]));
	else if (op == SGT)r[r0] = (int64_t)r[r1] >(int64_t)r[r2];
	else if (op == SGTD)r[r0] = (*(double*)(&r[r1])) > (*(double*)(&r[r2]));
	else if (op == SLE)r[r0] = (int64_t)r[r1] <= (int64_t)r[r2];
	else if (op == SLED)r[r0] = (*(double*)(&r[r1])) <= (*(double*)(&r[r2]));
	else if (op == SGE)r[r0] = (int64_t)r[r1] >= (int64_t)r[r2];
	else if (op == SGED)r[r0] = (*(double*)(&r[r1])) >= (*(double*)(&r[r2]));
}

void VirtualMachine::ParseTwoRegisterAndImmInstruction(const Instruction& instruction)
{
	auto& [op, r0, r1, val] = instruction;
	auto& r = registers;
	if (op == ADDI)r[r0] = r[r1] + val;
	else if (op == SUBI)r[r0] = r[r1] - val;
	else if (op == SB)*((byte*)(val + r[r1])) = (byte)(r[r0] & 0xff);
	else if (op == LB)r[r0] = (DWord)(*((byte*)(val + r[r1])));
	else if (op == SDW)*((DWord*)(val + r[r1])) = r[r0];
	else if (op == LDW)r[r0] = *((DWord*)(val + r[r1]));

}

void VirtualMachine::ParseJumpInstruction(const Instruction& instruction)
{
	//case BEQZ:case BNEZ:case B:
	auto& r = registers;
	auto& [op, r0, r1, offset] = instruction;
	if (op == BEQZ)
	{
		if (r[r0] == 0)r[pc] += offset;
	}
	else if (op == BNEZ)
	{
		if (r[r0] != 0)r[pc] += offset;
	}
	else if (op == B)
	{
		r[pc] += offset;
	}
}

void VirtualMachine::ParseSyscall()
{
	auto& idx = registers[0];
	if (idx == writeSyscall)WriteSyscall();
	else if (idx == readSyscall)ReadSyscall();
	else if (idx == openSyscall)OpenSyscall();
	else if (idx == closeSyscall)CloseSyscall();
	else if (idx == exitSyscall)ExitSyscall();
	else
	{
		cout << "Error syscall index " << idx << endl;
		exit(0);
	}
}

void VirtualMachine::WriteSyscall()
{
	//$r1->fileId,$r2->src,$r3->size
	fwrite((void*)registers[2],1,registers[3],(FILE*)(registers[1]));
}

void VirtualMachine::ReadSyscall()
{
	//$r1->fileId,$r2->dst,$r3->size
	auto len=fread((void*)registers[2], 1, registers[3], (FILE*)(registers[1]));
	registers[ret] = (DWord)len;
}

void VirtualMachine::OpenSyscall()
{
	//$r1->filePath,$r2->mode
	auto*id=fopen((char*)registers[1], (char*)registers[2]);
	registers[ret] =(DWord)id;
}

void VirtualMachine::CloseSyscall()
{
	//$r1->fileId
	auto state=fclose((FILE*)(registers[1]));
	registers[ret] = (DWord)state;
}

void VirtualMachine::ExitSyscall()
{
	exit(0);
}


void VirtualMachine::run()
{
	while (1)
	{
		auto& instruction = *((Instruction*)(registers[pc]));
		auto&& [op, r0, r1, r2] = instruction;
		registers[pc]+= sizeof(Instruction);
		//////////////////////////////
		//////////////////////////////
		switch (instruction.type)
		{
		case ADD:case ADDD:case SUB:case SUBD:case MULT:case MULTD:case DIV:case DIVD:case REM:case AND:case OR:case XOR:
		case SL:case SR:case SOR:case SAND:case SEQ:case SEQD:case SNE:case SNED:case SLT:case SLTD:case SGT:case SGTD:case SLE:case SLED:
		case SGE:case SGED:
			ParseThreeRegisterInstruction(instruction);
			break;
		case ADDI:case SUBI:case SB:case SDW:case LB:case LDW:
			ParseTwoRegisterAndImmInstruction(instruction);
			break;
		case BEQZ:case BNEZ:case B:
			ParseJumpInstruction(instruction);
			break;
		case NOT:
			registers[r0] = ~registers[r1];
			break;
		case MOVE:
			registers[r0] = registers[r1];
			break;
		case PUSH:
			registers[sp] -=8;
			*((DWord*)registers[sp]) = registers[r0];
			break;
		case POP:
			registers[r0] = *((DWord*)registers[sp]);
			registers[sp] += 8;
			break;
		case CALL:
			registers[sp] -= 8;
			*((DWord*)registers[sp]) = registers[pc];
			registers[pc]+=r2;
			break;
		case RET:
			registers[pc]= *((DWord*)registers[sp]);
			registers[sp] += 8+r2;
			break;
		case SYSCALL:
			ParseSyscall();
			break;
		case LI:
			registers[r0] = (DWord)r2;
			break;
		case LA:
			registers[r0] = r2 + registers[ds];
			break;
		default:
			Error(instruction);
		}
	}
}

void VirtualMachine::Error(const Instruction& instruction)
{
	cout << "Run Time Error!" << endl;
	exit(0);
}
