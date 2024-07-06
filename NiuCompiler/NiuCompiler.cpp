#include <iostream>
#include"Lexier/Lexier.h"
#include"Grammar/Grammar.h"
#include"Synax/Synax.h"
#include"Translator/Translator.h"
#include"VirtualMachine/Assembler.h"
int main(int argc,char**argv)
{
	////////////////////////汇编器实现部分
	/*if (argc <3)
	{
		cout << "Arguments less than 3!" << endl;
		return 0;
	}
	Lexier lexier(argv[1]);
	lexier.parse();
	Grammar grammar(lexier.tookens);
	grammar.parse();
	Synax synax(grammar.program);
	synax.parse();
	Translator translator(synax.program);
	translator.parse();
	translator.ExportToFile(argv[2], 0);
	Assembler assembler(argv[2]);
	assembler.parse();
	assembler.ExportToFile(argv[2]);
	*/
	/////////////////////////虚拟机实现部分
	if (argc <2)
	{
		cout << "Please input executable file!" << endl;
		return 0;
	}
	VirtualMachine machine(argv[1]);
	machine.run();
	return 0;
}