#pragma once
#include "ProgramControlBase.h"

class SwitchControl :	public ProgramControlBase
{
private:
	ChainExpression * switch_expr;
public:
	SwitchControl(void);
	SwitchControl(int control_type);
	void Print(FILE * file);
	void SetSwitchExpr(ChainExpression * switch_expr);
	~SwitchControl(void);
};
