#include "../Include/Intel8051ISA.h"
#include <cwchar>
#include "../Scanner.h"
#include "../SundCompiler.h"
#include "../ErrorPrinter.h"
#include "../../Module/Include/InstructionParser.h"

#include <string.h>
#include <stdio.h>

Intel8051ISA::Intel8051ISA()
{
}


Intel8051ISA::~Intel8051ISA()
{
}
/*
void Intel8051ISA::PerformInstruction(wchar_t * mnemonic_name, Scanner * scanner)
{
	if(!CheckMnemonic(mnemonic_name)) {
		// print error
		ScannerState * state = new ScannerState(scanner);
		Err->AddError(state, ErrorPrinter::MODULE_UNKNOWN_MNEMONIC, mnemonic_name);
		//return;
	}
	// get '('
	scanner->GetToken();
	do {
		// get arg
		//scanner->GetToken();
		ScannerState * state = new ScannerState(scanner);
		ChainExpression * chain = this->instructionParser->PerformInstructionExp(state);
		//if (*(scanner->token) != ')')
		//{
			// get (,)
			//scanner->GetToken();
		//}
		//else break;
		scanner->GetToken();
	} while (*(scanner->token) == ',');
	// get ';'
	scanner->GetToken();
	return;
}*/
int Intel8051ISA::CheckMnemonic(wchar_t * mnemonic_name)
{
	struct mnemonics mnemonic_table[] = { L"acall",ACALL,L"add",ADD,L"addc",ADDC,L"ajmp",AJMP,L"anl",ANL,L"cjne",CJNE,
										 L"clr",CLR,L"cpl",CPL,L"da",DA,L"dec",DEC,L"div",DIV,L"djnz",DJNZ,L"inc",INC,
		                                 L"jb",JB,L"jbc",JBC,L"jc",JC,L"jmp",JMP,L"jnb",JNB,L"jnc",JNC,L"jnz",JNZ,L"jz",JZ,
		                                 L"lcall",LCALL,L"ljmp",LJMP,L"mov",MOV,L"movc",MOVC,L"movx",MOVX,L"mul",MUL,
		                                 L"nop",NOP,L"orl",ORL,L"pop",POP,L"push",PUSH,L"ret",RET,L"reti",RETI,L"rl",RL,
		                                 L"rlc",RLC,L"rr",RR,L"rrc",RRC,L"setb",SETB,L"sjmp",SJMP,L"subb",SUBB,L"swap",SWAP,
		                                 L"xch",XCH,L"xchd",XCHD,L"xrl",XRL};

	int length = sizeof(mnemonic_table) / sizeof(mnemonic_table[0]);
	for (int i = 0; i<length; i++)
	{
		if(!wcscmp((mnemonic_table + i)->name, mnemonic_name)) {
			return (mnemonic_table + i)->tok;
		}
	}
	return 0;
}
int Intel8051ISA::GetOpCodeGroupIndex(wchar_t * op_code_group_name)
{
	struct op_code_name table[] = {L"REG",REG,L"DIRECT",DIRECT,L"BIT",BIT,L"A",A,L"C",C,L"DPTR",DPTR,
							       L"A_PLUS_DPTR",A_PLUS_DPTR,L"A_PLUS_PC",A_PLUS_PC};
	int length = sizeof(table) / sizeof(table[0]);
	for (int i = 0; i<length; i++)
	{
		if (!wcscmp((table + i)->name, op_code_group_name)) {
			return (table + i)->tok;
		}
	}
	return 0;
}

