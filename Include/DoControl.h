#pragma once

#include "ProgramControlBase.h"
#include "ChainExpression.h"


class DoControl : public ProgramControlBase
{
private:
	ChainExpression * while_condition;
public:
	DoControl(void);
	DoControl(int control_type);
	void Print(FILE * file);
	void SetWhileCondition(ChainExpression * while_condition);
	~DoControl(void);
};
