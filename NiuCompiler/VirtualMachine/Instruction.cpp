#pragma once
#include "unordered_map"
#include"string"
#include"iostream"
#include"fstream"
#include"map"
using namespace std;
enum InstructionType {
	/////////////////机器指令
	ADD,
	ADDI,
	ADDD,
	SUB,
	SUBI,
	SUBD,
	MULT,
	MULTD,
	DIV,
	DIVD,
	REM,
	AND,
	OR,
	NOT,
	XOR,
	SL,
	SR,
	BEQZ,
	BNEZ,
	B,
	MOVE,
	SB,
	SDW,
	LB,
	LDW,
	PUSH,
	POP,
	CALL,
	RET,
	SYSCALL,
	LI,
	LA,
	SOR,
	SAND,
	SEQ,
	SNE,
	SLT,
	SLE,
	SGT,
	SGE,
	SEQD,
	SNED,
	SLTD,
	SLED,
	SGTD,
	SGED,
};
//////////////////////////////////////////////
//////////////////////////////////////////////
const unordered_map<string,InstructionType>MapToInstruction{
	{"add",ADD},
	{"addi",ADDI},
	{"addd",ADDD},
	{"sub",SUB},
	{"subi",SUBI},
	{"subd",SUBD},
	{"mult",MULT},
	{"multd",MULTD},
	{"div",DIV},
	{"divd",DIVD},
	{"rem",REM},
	{"and",AND},
	{"or",OR},
	{"not",NOT},
	{"xor",XOR},
	{"sl",SL},
	{"sr",SR},
	{"beqz",BEQZ},
	{"bnez",BNEZ},
	{"move",MOVE},
	{"sb",SB},
	{"sdw",SDW},
	{"lb",LB},
	{"ldw",LDW},
	{"push",PUSH},
	{"pop",POP},
	{"call",CALL},
	{"ret",RET},
	{"b",B},
	{"syscall",SYSCALL},
	{"li",LI},
	{"la",LA},
	{"sor",SOR},
	{"sand",SAND},
	{"seq",SEQ},
	{"seqd",SEQD},
	{"sne",SNE},
	{"sned",SNED},
	{"slt",SLT},
	{"sltd",SLTD},
	{"sgt",SGT},
	{"sgtd",SGTD},
	{"sle",SLE},
	{"sled",SLED},
	{"sge",SGE},
	{"sged",SGED}
};
/////////////////////////////////////////////
using DWord = uint64_t;
struct Instruction {
	InstructionType type;
	DWord src0, src1;
	int64_t src2;
};
/////////////////////////////////////////////定义
const string EntryFunction = "NiuNiu";
const DWord stackSize = 8 * 1024 * 1024;//栈的大小8MB
const DWord MaxRegister = 65536;
const DWord sp =MaxRegister;
const DWord bp = sp + 1;
const DWord pc = bp + 1;
const DWord ds = pc + 1;
const DWord ret = ds + 1;
//系统调用参数
const DWord writeSyscall = 0;
const DWord readSyscall = 1;
const DWord openSyscall = 2;
const DWord closeSyscall = 3;
const DWord exitSyscall = 10;