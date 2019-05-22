#pragma once
#include "ProgramControlBase.h"

class ForControl: public ProgramControlBase
{
private:
	ChainExpression * chain1,* chain2, *chain3;
public:
	ForControl(void);
	ForControl(int control_type);
	void SetExpr1(ChainExpression * chain1);
	void SetExpr2(ChainExpression * chain2);
	void SetExpr3(ChainExpression * chain3);
	void Print(FILE * file);
	~ForControl(void);
};
