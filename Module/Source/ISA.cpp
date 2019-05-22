#include "../Include/ISA.h"
#include <cwchar>
#include "../../Intel8051/Include/Intel8051ISA.h"
#include "../../Device.h"
#include "../Include/Module.h"
#include "../Include/PropertyVar.h"
#include "../Include/InstructionParser.h"
#include "../Scanner.h"
#include "../SundCompiler.h"
#include "../ErrorPrinter.h"
#include "../Include/InstructionOperand.h"


//Instruction set architecture
ISA::ISA()
{
}


ISA::~ISA()
{
}

ISA * ISA::CreateISAObject(wchar_t * device)
{
	if (!wcscmp(device, I8051)) {
		return new Intel8051ISA();
	}
	return new ISA();
}
void ISA::DoCodeStatement(Scanner * scanner)
{
	this->instructionParser = new InstructionParser(scanner, Module::GetInstance());

	int is_end_code = 0;
	while (!is_end_code && scanner->tok != SundCompiler::FINISHED)
	{
		ScannerState * s = new ScannerState(scanner);
		scanner->GetToken();
		if (scanner->token_type == SundCompiler::MODULE_DIR)
		{
			// '@' , then check is endc dir.
			wchar_t temp_buff[80];
			scanner->ReadNextToken(temp_buff);
			if (!wcscmp(temp_buff, L"endc")) {
				is_end_code = 1;
				// get token "endc"
				scanner->GetToken();
				// get next token
				scanner->GetToken();
			}
			else {
				// handle directives (org,db,dw,dt...)
				PerformDirective(scanner);
			}
		}
		else {
			// handle labels,mnemonics instructions
			wchar_t token[80];
			wcscpy_s(token, scanner->token);
			if(Module::GetInstance()->IsExistLabel(token)) {
				// token is label
				// get next token ':'
				scanner->GetToken();
				if(*scanner->token != ':') {
					ScannerState * current_state = new ScannerState(scanner);
					Err->Print(current_state,Err->MODULE_CODE_SYNTAX);
					return;
				}
				continue;
			}
			PerformInstruction(token, scanner);
		}
	}
}
void ISA::CollectLabels(Scanner * scanner)
{
	int is_end_code = 0;
	wchar_t name[80];
	while (!is_end_code && scanner->tok != SundCompiler::FINISHED)
	{
		scanner->GetToken();
		wcscpy_s(name, scanner->token);
		if (scanner->token_type == SundCompiler::MODULE_DIR)
		{
			// '@' , then check is endc dir.
			wchar_t temp_buff[80];
			scanner->ReadNextToken(temp_buff);
			if (!wcscmp(temp_buff, L"endc")) {
				is_end_code = 1;
				// get token "endc"
				scanner->GetToken();
				// get next token
				scanner->GetToken();
			}
		}
		else {
			// handle labels
			// ':' 
			wchar_t temp_buff[80];
			scanner->ReadNextToken(temp_buff);
			if (!wcscmp(temp_buff, L":")) {
				Module::GetInstance()->DeclLabel(name);
				// get token ":"
				scanner->GetToken();
				// get next token
				scanner->GetToken();
			}
		}
	}
}
void ISA::PerformDirective(Scanner * scanner)
{
	// get directive
	scanner->GetToken();
	int directive_name_index = this->GetDirectiveNameIndex(scanner->token);
	switch(directive_name_index) {
	case ISA::ORG:
		this->PerformORG(scanner);
		break;
	case ISA::DB:
		this->PerformDB(scanner);
		break;
	case ISA::DW:
		this->PerformDW(scanner);
		break;
	case ISA::DT:
		this->PerformDT(scanner);
		break;
	default:
		// print error message
		break;
	}
}
void ISA::PerformORG(Scanner * scanner)
{
	PropertyVar * prop_value = new PropertyVar();
	// get next token '('
	scanner->GetToken();
	if (*(scanner->token) != '(') {
		// print error
		Err->Print(new ScannerState(scanner), ErrorPrinter::MODULE_CODE_SYNTAX);
		return;
	}
	// get org value
	scanner->GetToken();
	int prop_result = Module::GetInstance()->GetPropertyVarData(scanner->token, prop_value);
	int token_type = scanner->token_type;
	if(prop_result == 0 && (token_type != SundCompiler::NUMBER && token_type != SundCompiler::HEX_NUMBER && token_type != SundCompiler::B_NUMBER))
	{
		// print error
		Err->Print(new ScannerState(scanner), ErrorPrinter::MODULE_ORG_ERROR);
		return;
	}
	// get next token ')'
	scanner->GetToken();
	if (*(scanner->token) != ')') {
		// print error
		Err->Print(new ScannerState(scanner), ErrorPrinter::MODULE_CODE_SYNTAX);
		return;
	}
	wchar_t temp_buff[80];
	scanner->ReadNextToken(temp_buff);
	if (!wcscmp(temp_buff, L";")) {
		scanner->GetToken();
	}
}
void ISA::PerformDB(Scanner * scanner)
{
	PropertyVar * prop_value = new PropertyVar();
	// get next token '('
	scanner->GetToken();
	if (*(scanner->token) != '(') {
		// print error
		Err->Print(new ScannerState(scanner), ErrorPrinter::MODULE_CODE_SYNTAX);
		return;
	}
	// get db value
	scanner->GetToken();
	int prop_result = Module::GetInstance()->GetPropertyVarData(scanner->token, prop_value);
	int token_type = scanner->token_type;
	if (prop_result == 0 && (token_type != SundCompiler::NUMBER && token_type != SundCompiler::HEX_NUMBER && token_type != SundCompiler::B_NUMBER && token_type != SundCompiler::CHAR))
	{
		// print error
		Err->Print(new ScannerState(scanner), ErrorPrinter::MODULE_DB_ERROR);
		return;
	}
	// get next token ')'
	scanner->GetToken();
	if (*(scanner->token) != ')') {
		// print error
		Err->Print(new ScannerState(scanner), ErrorPrinter::MODULE_CODE_SYNTAX);
		return;
	}
	wchar_t temp_buff[80];
	scanner->ReadNextToken(temp_buff);
	if (!wcscmp(temp_buff, L";")) {
		scanner->GetToken();
	}
}
void ISA::PerformDW(Scanner * scanner)
{

}
void ISA::PerformDT(Scanner * scanner)
{

}
void ISA::PerformInstruction(wchar_t * mnemonic_name, Scanner * scanner)
{
	if (!CheckMnemonic(mnemonic_name)) {
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
		InstructionOperand * operand = new InstructionOperand(chain);
		//
		/*if (*(scanner->token) != ')')
		{
		// get (,)
		scanner->GetToken();
		}
		else break;*/
		scanner->GetToken();
	} while (*(scanner->token) == ',');
	// get ';'
	scanner->GetToken();
	return;
}
int ISA::CheckMnemonic(wchar_t * mnemonic_name)
{
	return 0;
}
int ISA::GetOpCodeGroupIndex(wchar_t * op_code_group_name)
{
	return 0;
}
int ISA::GetDirectiveNameIndex(wchar_t * directive_name)
{
	struct directive_name table[] = { L"org",ORG,L"DB",DB,L"DW",DW,L"dt",DT};
	int length = sizeof(table) / sizeof(table[0]);
	for (int i = 0; i<length; i++)
	{
		if (!wcscmp((table + i)->name, directive_name)) {
			return (table + i)->tok;
		}
	}
	return 0;
}
