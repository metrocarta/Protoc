#define Err ErrorPrinter::GetInstance()
#pragma once
#include "ScannerState.h"
#include "../Preprocessor/Include/Preprocessor.h"
#include <list>

using namespace std;


#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif


struct error_info {
	wchar_t message_text[200];
	//int line_counter;
};

class Scanner;

class ErrorPrinter
{
private:
	static ErrorPrinter * instance;
	Scanner * scanner;
	int line_counter;
	int num_errors,num_warnings;
	wchar_t const *  GetErrorString(int error);
public:
	list<error_info> error_list,warning_list;
	enum error_msg {
		SYNTAX, CANNOT_OPEN_FILE, UNBAL_PARENS, NO_EXP, EQUALS_EXPECTED, NOT_VAR,
		PARAM_ERR, SEMI_EXPECTED, UNBAL_BRACES, FUNC_UNDEF,
		TYPE_EXPECTED, TYPE_OR_QUALIFIER_EXPECTED, NO_SUPPORT_YET, NEST_FUNC, RET_NOCALL, PAREN_EXPECTED,
		WHILE_EXPECTED, QUOTE_EXPECTED, NOT_TEMP, FUNC_ALREADY_HAS_BODY,
		TOO_MANY_LVARS, UNDECLARED_IDENTIFIER, BAD_SUFFIX_NUMBER,VAR_IS_KEYWORD,
		NO_CONST_EXPR, UNSUP_EXPR, UNRECOGNIZED_STMT, UNRECOGNIZED_STMT_ELSE,
		ILLEGAL_CHAR, ILLEGAL_EXPR, MISS_PROTOTYPE, SYNTAX_SIZEOF, UNREACH_CODE, TOO_MANY_CHARS, INVALID_CHAR_CONSTANT,
		UNDEFINED_LABEL, REDEFINED_LABEL,NON_ASCII_CHAR, NOT_ALLOWED_POINTER, LOCAL_VAR_REDEFINITION, GLOBAL_VAR_REDEFINITION, FUNC_REDEFINITION, FUNC_REQUIRES_ANSI_STYLE,
		DIV_BY_ZERO,NOT_ENOUGH_PAR,TOO_MANY_PAR,TOO_MANY_INITIALIZERS,CANNOT_CONVERT_INIT_LIST_TO_TYPE,MISSING_SUBSCRIPT,
		ILLEGAL_RETURN_EXPR,WRONG_ARG_TYPE,
		CANNOT_CONVERT_FROM_DIF_TYPE,UNRESOLVED_EXTERN,ILLEGAL_INDIR,LEFT_SIDE_NOT_LVALUE,SUBSCRIPT_ON_NON_ARRAY,OPERANDS_HAVE_ILLEGAL_TYPES,
		CPP_REDEFINITION_OF_DEFINE, CPP_WRONG_NUM_PARAMETRS,
		CPP_SYNTAX, CPP_REUSE_ARG, CPP_INVALID_COMMAND, CPP_CANNOT_OPEN_FILE, CPP_INCLUDE_SYNTAX,
		MODULE_HEADER_NAME_OR_DEVICE, MODULE_SYNTAX, MODULE_BODY_SYNTAX, MODULE_PROPERTY_SYNTAX, MODULE_PROPERTY_VAR_REDEFINITION,MODULE_VAR_SYNTAX,
		MODULE_UNDEFINED_VAR, MODULE_CODE_REQUIRES_ENDC, MODULE_LIB_REQUIRES_ENDL, MODULE_MACRO_REQUIRES_ENDM, MODULE_VAR_NOT_IDENTIFIER,MODULE_CODE_SYNTAX,
		MODULE_LIB_SYNTAX,MODULE_UNKNOWN_MNEMONIC,MODULE_LABEL_REDEFINED,MODULE_VAR_IS_KEYWORD,MODULE_ORG_ERROR,MODULE_DB_ERROR
	};

	ErrorPrinter(Scanner * scanner);
	static ErrorPrinter * GetInstance(){return instance;}
	void Print(int error,wchar_t * s = 0);
	void Print(ScannerState * state,int error,wchar_t * s = 0);
	void CPPPrint(Scanner * ppc_scanner,Preprocessor * preprocessor, int error, wchar_t * s = 0);
	void CPPFatal(int line_count, int error, wchar_t const  *filename, wchar_t * s);
	void AddError(int error, wchar_t * s = 0);
	void AddError(ScannerState * state, int error, wchar_t * s = 0);
	void AddWarning(int line_counter,int error);
	void SetLineCounter(int line_counter);
	int PrintAll();
	~ErrorPrinter(void);
};
