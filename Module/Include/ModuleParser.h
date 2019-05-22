#pragma once
#include "../SundParser.h"
//#include "../ScannerState.h"

class Module;

class ModuleParser : public SundParser
{
private:
	Module * module;
public:
	ModuleParser(Scanner * scanner,Module * module);
	ChainExpression * PerformAsgn();
	ChainExpression * PerformBitwiseAND();
	ChainExpression * PerformMultDiv();
	ChainExpression * PerformPrimaryExp();
	ChainExpression * PerformCallFunc();
	// spec. case for module
	ChainExpression * PerformModuleExp(ScannerState * state);
	~ModuleParser();
};

