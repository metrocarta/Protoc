#pragma once
#include "../Var.h"
class ChainExpression;
class ModuleVar : public Var
{
private:
public:
	//Var * var;
	ChainExpression * chain;
	ModuleVar();
	~ModuleVar();
};

