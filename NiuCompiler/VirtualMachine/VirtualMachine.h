#pragma once
#include "instruction.cpp"
////////////////////////////////////////
struct VirtualMachine {
	vector<DWord>registers;
	vector<byte>DataSection;
	vector<DWord>stack;
	vector<Instruction>TextSection;
	VirtualMachine(const string& file);
	void ParseThreeRegisterInstruction(const Instruction& instruction);
	void ParseTwoRegisterAndImmInstruction(const Instruction& instruction);
	void ParseJumpInstruction(const Instruction& instruction);
	void ParseSyscall();
	void WriteSyscall();
	void ReadSyscall();
	void OpenSyscall();
	void CloseSyscall();
	void ExitSyscall();
	void run();
	void Error(const Instruction&instruction);
};