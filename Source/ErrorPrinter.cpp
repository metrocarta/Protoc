#include "../Include/ErrorPrinter.h"
#include "../Include/Scanner.h"
#include <stdio.h>

ErrorPrinter * ErrorPrinter::instance = 0;

ErrorPrinter::ErrorPrinter(Scanner * scanner)
{
	this->scanner = scanner;
	this->line_counter = 0;
	this->num_errors = 0;
	this->num_warnings = 0;
	this->error_list.clear();
	this->warning_list.clear();
	instance = this;
}
wchar_t const *  ErrorPrinter::GetErrorString(int error)
{
	static wchar_t const * e[] = { L"syntax error", L"cannot open input file:",L"unbalanced parentheses",
                    L"no expression present",L"equals sign expected",
                    L"not a variable",L"parameter error",L"semicolon (;) expected",
                    L"unbalanced braces",L"function undefined",
					L"type specifier expected", L"type or qualifier specifier expected", L"keyword not supported",
                    L"too many nested function calls",
                    L"return without call",L"parentheses expected",
                    L"while expected",L"closing quote expected",
                    L"not a string ",L"function already has a body",L"too many local variables",L"undeclared identifier",
					L"bad suffix on number",L"indentifier is a keyword",L"integer expression must be constant",
					L"unsupported expression",L"unrecognized statement",L"unrecognized statement (else)",
					L"illegal character",L"illegal expression",L"missing prototype",L"syntax error (sizeof operator)",
					L"unreachable code", L"too many characters",L"invalid character constant",L"undefined label",L"label redefined", L"not ascii character", L"the pointers are not allowed", 
					L"local variable redefinition",
					L"global variable redefinition", L"function redefinition", L"function requires ANSI-style prototype",
					L"division by zero",  L"not enough parameters", L"too many parameters", L"too many initializers", L"cannot convert from 'initializer-list' to type",
					L"missing subscript",L"illegal return expression",L"wrong argument type", L"illegal type conversion", L"unresolved function", L"illegal indirection", L"left side of expression not an lvalue", L"subscript on non-array", L"operands have illegal types",
					L"CPP: macro redefinition",
					L"CPP: wrong number macro parameters", L"CPP: Syntax error", L"CPP: macro reuses argument",
					L"CPP: invalid preprocessor command", L"CPP: cannot open input file:", L"CPP: #include syntax error",
					L"MODULE: syntax error: header name or device",  L"MODULE: header syntax error",
					L"MODULE: body syntax error", L"MODULE: directive @property: syntax error", L"MODULE: property var redefinition",
					L"MODULE: directive @var: syntax error", L"MODULE: undeclared identifier", L"MODULE: directive @code requires @endc directive", L"MODULE: directive @lib requires @endl directive",
					L"MODULE: directive @macro requires @endm directive", L"MODULE: @var is not identifier", L"MODULE: @code syntax error", L"MODULE: @lib syntax error",L"MODULE: unknown mnemonic",
					L"MODULE: label redefined",L"MODULE: identifier is a keyword",L"MODULE: invalid org argument",L"MODULE: invalid db argument"
                    };
	return e[error];
}
void ErrorPrinter::Print(int error, wchar_t * s)
{
	wchar_t msg[200];
	int line_count = scanner->GetErrorPosition();

	if(s == 0){
		swprintf(msg,200, L"%ls(%d): %ls \n", scanner->curr_file_name,line_count,GetErrorString(error));
	}
	else{
		swprintf(msg, 200, L"%ls(%d): '%ls' : %ls\n", scanner->curr_file_name,line_count, s,GetErrorString(error));
	}
	throw msg;
}
void ErrorPrinter::Print(ScannerState * state,int error,wchar_t * s)
{
	if(state != NULL) {
		this->scanner->Revert(*state);
	}
	this->Print(error,s);
}
void ErrorPrinter::CPPPrint(Scanner * ppc_scanner, Preprocessor * preprocessor, int error, wchar_t * s)
{
	wchar_t msg[200];
	int line_count = (1 + ppc_scanner->CountEndLine());
	if(s == 0){
		swprintf(msg, 200, L"%ls(%d): %ls \n", ppc_scanner->GetFileName(), line_count, GetErrorString(error));
	}
	else{
		swprintf(msg, 200, L"%ls(%d): '%ls' : %ls\n", ppc_scanner->GetFileName(), line_count, s, GetErrorString(error));
	}

	throw new PreprocessorMessage(msg,preprocessor);
}
void ErrorPrinter::CPPFatal(int line_count, int error, wchar_t const * filename, wchar_t * s)
{
	wchar_t msg[200];
	/*if (s == 0){
		swprintf(msg, 200, L"%ls(%d): %ls \n", filename, line_count, GetErrorString(error));
	}
	else{
		swprintf(msg, 200, L"%ls(%d): '%ls' : %ls\n", filename, line_count, s, GetErrorString(error));
	}*/
	swprintf(msg, 200, L"%ls(%d): '%ls' : %ls\n", filename, line_count, s, GetErrorString(error));
	throw msg;
}
void ErrorPrinter::SetLineCounter(int line_counter)
{
	this->line_counter = line_counter;
}

void ErrorPrinter::AddWarning(int line_counter,int error)
{
	error_info info;
	swprintf(info.message_text, 80, L"%ls in line: %d.\n", GetErrorString(error), line_counter);
	//info.line_counter = line_counter;
	this->warning_list.push_back(info);
}
void ErrorPrinter::AddError(int error, wchar_t * s)
{
    error_info info;
	int line_count = scanner->GetErrorPosition();
	if (s == 0){
		swprintf(info.message_text, 200, L"%ls(%d): %ls \n", scanner->curr_file_name, line_count, GetErrorString(error));
	}
	else{
		swprintf(info.message_text, 200, L"%ls(%d): '%ls' : %ls\n", scanner->curr_file_name, line_count, s, GetErrorString(error));
	}
	/*if (line_count != -1){
		if (s == 0){
			swprintf(info.message_text, 200, L"%ls(%d): %ls \n", scanner->curr_file_name, line_count, GetErrorString(error));
		}
		else{
			swprintf(info.message_text, 200, L"%ls(%d): '%ls' : %ls\n", scanner->curr_file_name, line_count, s, GetErrorString(error));
		}
	}
	else
	{
		if (s == 0){
			swprintf(info.message_text, 200, L"%ls \n", GetErrorString(error));
		}
		else{
			swprintf(info.message_text, 200, L"'%ls' : %ls\n", s, GetErrorString(error));
		}
	}*/
	this->error_list.push_back(info);
}
void ErrorPrinter::AddError(ScannerState * state, int error, wchar_t * s)
{
	ScannerState * tmp = new ScannerState(this->scanner);
	this->scanner->Revert(*state);
	this->AddError(error, s);
	this->scanner->Revert(*tmp);
}
int ErrorPrinter::PrintAll()
{
	return 0;
}
ErrorPrinter::~ErrorPrinter(void)
{
}
