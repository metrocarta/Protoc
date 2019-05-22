#include "../Include/CodeRunner.h"
#include "../ProgramArgumentsParser.h"
#include "../Scanner.h"
#include "../SundCompiler.h"
#include "../ErrorPrinter.h"
#include "../Include/ISA.h"
#include "../Include/Module.h"
#include "../Include/ModuleParser.h"


CodeRunner::CodeRunner()
{
}
CodeRunner::CodeRunner(Scanner * scanner)
{
	this->scanner = scanner;
	wcscpy(device, L"");
}
void CodeRunner::Run()
{
	//int is_end_code = 0;
	ScannerState * state = new ScannerState(scanner);
	// is next token '(' ?
	scanner->GetToken();
	if (*(scanner->token) != '('){
		// print error
		Err->Print(state, ErrorPrinter::MODULE_CODE_SYNTAX);
		return;
	}
	// get device
	//scanner->GetToken();
	//wcscpy(device,scanner->token);
	wcscpy(device, ProgramArgumentsParser::GetInstance()->GetPlatform());
	ISA * isa = ISA::CreateISAObject(device);
	// check device's name
	// is next token ')' ?
	scanner->GetToken();
	if (*(scanner->token) != ')'){
		// print error
		Err->Print(state,ErrorPrinter::MODULE_CODE_SYNTAX);
		return;
	}
	// collect labels first
	ScannerState * old_state = new ScannerState(scanner);
	isa->CollectLabels(scanner);
	scanner->Revert(*old_state);
	// do code block
	isa->DoCodeStatement(scanner);
	if (scanner->tok == SundCompiler::FINISHED){
		// print error
		Err->Print(state,ErrorPrinter::MODULE_CODE_REQUIRES_ENDC);
		return;
	}
	scanner->Putback();
}

CodeRunner::~CodeRunner()
{
}
