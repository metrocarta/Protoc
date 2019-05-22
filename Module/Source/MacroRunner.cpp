#include "../Include/MacroRunner.h"
#include "../Scanner.h"
#include "../SundCompiler.h"
#include "../ErrorPrinter.h"



MacroRunner::MacroRunner()
:CodeRunner()
{
}

MacroRunner::MacroRunner(Scanner * scanner)
: CodeRunner(scanner)
{
	
}
void MacroRunner::Run()
{
	int is_end_code = 0;
	ScannerState * state = new ScannerState(scanner);
	while (!is_end_code && scanner->tok != SundCompiler::FINISHED)
	{
		scanner->GetToken();
		if (scanner->token_type == SundCompiler::MODULE_DIR)
		{
			// '@' , then check is endm dir.
			wchar_t temp_buff[80];
			scanner->ReadNextToken(temp_buff);
			if (!wcscmp(temp_buff, L"endm")){
				is_end_code = 1;
				// get token "endc"
				scanner->GetToken();
				// get next token
				scanner->GetToken();
			}
		}
	}
	if (scanner->tok == SundCompiler::FINISHED){
		// print error
		Err->Print(state, ErrorPrinter::MODULE_MACRO_REQUIRES_ENDM);
		return;
	}
	scanner->Putback();
}

MacroRunner::~MacroRunner()
{
}
