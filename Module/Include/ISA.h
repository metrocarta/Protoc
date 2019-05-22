//Instruction set architecture

#pragma once
struct mnemonics {
	wchar_t name[20];
	int tok;
};
struct op_code_name {
	wchar_t name[50];
	int tok;
};
struct directive_name {
	wchar_t name[50];
	int tok;
};

class Scanner;
class InstructionParser;
class ISA
{
private:	
protected:
	enum directive_types {
		ORG = 1, DB, DW, DT
	};
	InstructionParser * instructionParser;
	void PerformDirective(Scanner * scanner);
	void PerformORG(Scanner * scanner);
	void PerformDB(Scanner * scanner);
	void PerformDW(Scanner * scanner);
	void PerformDT(Scanner * scanner);
public:
	ISA();
	~ISA();
	static ISA * CreateISAObject(wchar_t * device);
	void DoCodeStatement(Scanner * scanner);
	void CollectLabels(Scanner * scanner);
	virtual void PerformInstruction(wchar_t * mnemonic_name, Scanner * scanner);
	virtual int CheckMnemonic(wchar_t * mnemonic_name);
	virtual int GetOpCodeGroupIndex(wchar_t * op_code_group_name);
	virtual int GetDirectiveNameIndex(wchar_t * directive_name);
};

