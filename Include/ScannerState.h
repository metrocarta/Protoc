#pragma once

#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif

class Scanner;
class ScannerState
{
private:
	wchar_t * prog;
	wchar_t token[80];
	wchar_t token_type, tok;
public:
	ScannerState(void);
	ScannerState(wchar_t * prog, wchar_t token[], wchar_t token_type, wchar_t tok);
	ScannerState(Scanner *scanner);
	~ScannerState(void);
	wchar_t * GetCode(){ return prog; }
	wchar_t * GetToken(){ return token; }
	wchar_t   GetTokenType(){ return token_type; }
	wchar_t   GetTok(){ return tok; }
	int    IsEqual(ScannerState * state);
};
