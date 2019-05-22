#pragma once
#include "Scanner.h"
#define TOKEN_MAX_SIZE 80

class SundCompiler;
class ScannerState;

class Scanner
{
private:
	wchar_t const * file_name;
	int Iswhite(wchar_t c);
	int Isdelim(wchar_t c);
	int LookUp(wchar_t *s);
	// checks (type)
	int IsCastOperator(wchar_t * p_token);
public:
	wchar_t * prog;
	wchar_t * p_buf;
	wchar_t token[TOKEN_MAX_SIZE];
	wchar_t token_type, tok;
	wchar_t curr_file_name[200];
	Scanner(void);
	Scanner(wchar_t const * file_name);
	Scanner(wchar_t * prog, wchar_t * p_buf);
	~Scanner(void);
	int GetToken(void);
	void Putback(void);
	// reads the next token but it doesn't move prog pointer
	void ReadNextToken(wchar_t * buff);
	void Revert(ScannerState state);
	wchar_t const * GetTokenType();
	wchar_t const * GetTok();
	int IsEndOfLine();
	int CountEndLine();
	// returns error position and curent filename
	int GetErrorPosition();
	// obtain string between two position (p1,p2)
	static void GetCodeDifference(wchar_t * p1, wchar_t * p2, wchar_t * buffer);
	void GetTokenPrefix(wchar_t * buff, wchar_t * p1, wchar_t * p2);
	// print token description
	void Print();
	wchar_t const * GetFileName(){ return file_name;};
	// search forbidden token eg. #token
	static int SearchForbiddenToken(wchar_t * buff,wchar_t * token);
};
