#pragma once
#include "ScannerState.h"
#define VAR_NAME_LEN 80

// see: http://en.cppreference.com/w/c/language/operator_precedence

// avoid use strcpy_s,strcat_s ...
#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif

class Scanner;
class ChainExpression;
class Var;
class ProgramControlBase;
class IntermediateNode;

class SundParser
{
private:
	int block_id;
	// c_operator is C operator token eg. '+','-'
protected:
	ScannerState * scannerState;
	int H(wchar_t * s);
	int B(wchar_t * s);
	wchar_t func_name[VAR_NAME_LEN], identifier[VAR_NAME_LEN];
	Scanner *scanner;
	ChainExpression * postfix_chain;
	ChainExpression * PerformExp(int do_separate_expr = 0);
	virtual ChainExpression * PerformAsgn();
	ChainExpression * PerformLogicalOR();
	ChainExpression * PerformLogicalAND();

	virtual ChainExpression * PerformBitwiseIOR();
	virtual ChainExpression * PerformBitwiseXOR();
	virtual ChainExpression * PerformBitwiseAND();

	ChainExpression * PerformRelOp();
	ChainExpression * PerformBitshift();
	virtual ChainExpression * PerformAddSub();
	virtual ChainExpression * PerformMultDiv();
	// Prefix increment and decrement ,Unary plus and minus,Logical NOT and bitwise NOT
	// type case, Indirection (dereference),Address-of,sizeof 
	virtual ChainExpression * PerformUnaryOp();
	// () [] -> . ,posfix ++,--
	ChainExpression * PerformPostfixExp();
	virtual ChainExpression * PerformPrimaryExp();
	virtual ChainExpression * PerformCallFunc();
	virtual ChainExpression * PerformArray(wchar_t * array_name,ChainExpression * chain);
	//postfix increment and decrement 
	void DoPostfixIncDec(Var * var);
	ChainExpression * PerformSizeof();
	// validate number (decimal,hex, binary or boolean)
	int Validate(wchar_t * s,int type);
	// get number range
	int GetRange(wchar_t * s);
	char GetCharacterValue(wchar_t * s);
public:
	//SundParser(void);
	ProgramControlBase * program_control;
	SundParser(Scanner * scanner);
	~SundParser(void);
	// if do_separate_expr == 1 function handles separate expression (expr,expr...,expr)
	ChainExpression * PerformExp(ScannerState * scannerState,int do_separate_expr = 0);
	void SetFuncName(wchar_t * func_name);
	wchar_t * GetFuncName(){ return func_name; };
};
