#include "../Include/LibRunner.h"
#include "../Scanner.h"
#include "../SundCompiler.h"
#include "../ErrorPrinter.h"


LibRunner::LibRunner()
{
}
LibRunner::LibRunner(Scanner * scanner)
{
	this->scanner = scanner;
	wcscpy(device, L"");
}
void LibRunner::Run()
{
	int is_end_code = 0;
	ScannerState * state = new ScannerState(scanner);
	// is next token '(' ?
	scanner->GetToken();
	if (*(scanner->token) != '('){
		// print error
		Err->Print(state, ErrorPrinter::MODULE_LIB_SYNTAX);
		return;
	}
	// get device
	scanner->GetToken();
	wcscpy(device, scanner->token);
	// check device's name
	// is next token ')' ?
	scanner->GetToken();
	if (*(scanner->token) != ')'){
		// print error
		Err->Print(state, ErrorPrinter::MODULE_LIB_SYNTAX);
		return;
	}
	/*
	// get device
	scanner->GetToken();
	wcscpy(device, scanner->token);
	// check device's name
	ScannerState * state = new ScannerState(scanner);*/
	while (!is_end_code && scanner->tok != SundCompiler::FINISHED)
	{
		scanner->GetToken();
		if (scanner->token_type == SundCompiler::MODULE_DIR)
		{
			// '@' , then check is endl dir.
			wchar_t temp_buff[80];
			scanner->ReadNextToken(temp_buff);
			if (!wcscmp(temp_buff, L"endl")){
				is_end_code = 1;
				// get token "endl"
				scanner->GetToken();
				// get next token
				scanner->GetToken();
			}
		}
	}
	if (scanner->tok == SundCompiler::FINISHED){
		// print error
		Err->Print(state, ErrorPrinter::MODULE_LIB_REQUIRES_ENDL);
		return;
	}
	scanner->Putback();
}

LibRunner::~LibRunner()
{
}
