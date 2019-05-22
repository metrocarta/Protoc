//Intel8051 instruction set architecture
#pragma once
#include "../../Module/Include/ISA.h"



class Scanner;
class Intel8051ISA : public ISA
{
private:
	enum tok_types {
		ACALL=1, ADD, ADDC, AJMP, ANL, CJNE, CLR, CPL, DA, DEC, DIV, DJNZ, INC, JB, JBC, JC, JMP, JNB, JNC, JNZ, JZ, LCALL,
		LJMP, MOV, MOVC, MOVX, MUL, NOP, ORL, POP, PUSH, RET, RETI, RL, RLC, RR, RRC, SETB, SJMP, SUBB, SWAP, XCH,
		XCHD, XRL
	};
	enum op_code_types {
		REG = 1,DIRECT,BIT,A,C,DPTR,A_PLUS_DPTR,A_PLUS_PC
	};
public:
	Intel8051ISA();
	~Intel8051ISA();
	//void PerformInstruction(wchar_t * mnemonic_name, Scanner * scanner);
	int CheckMnemonic(wchar_t * mnemonic_name);
	int GetOpCodeGroupIndex(wchar_t * op_code_group_name);
};

