#pragma once
#include "../SundParser.h"
class Module;
class SundType;

class InstructionParser :
	public SundParser
{
private:
	Module * module;
	ChainExpression * PerformArray(wchar_t * array_name, ChainExpression * chain);
public:
	InstructionParser(Scanner * scanner, Module * module);
	ChainExpression * PerformInstructionExp(ScannerState * state);
	ChainExpression * PerformBitwiseIOR();
	ChainExpression * PerformAddSub();
	ChainExpression * PerformMultDiv();
	ChainExpression * PerformPostfixExp();
	ChainExpression * PerformPrimaryExp();
	~InstructionParser();
};

