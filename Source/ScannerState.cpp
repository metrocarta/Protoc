#include "../Include/ScannerState.h"
#include "../Include/Scanner.h"
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h> 


ScannerState::ScannerState(void)
{
}

ScannerState::ScannerState(wchar_t * prog, wchar_t token[], wchar_t token_type, wchar_t tok)
{
	this->prog = prog;
	wcscpy(this->token, token);
	this->token_type = token_type;
	this->tok = tok;
}
ScannerState::ScannerState(Scanner *scanner)
{
	this->prog = scanner->prog;
	wcscpy(this->token, scanner->token);
	this->token_type = scanner->token_type;
	this->tok = scanner->tok;
}
int ScannerState::IsEqual(ScannerState * state)
{
	if (!wcscmp(this->prog, state->prog)){
		if (!wcscmp(this->token, state->token)){
			if(this->token_type == state->token_type){
				if(this->tok == state->tok){
					return 1;
				}
			}
		}
	}
	return 0;
}
ScannerState::~ScannerState(void)
{
}
