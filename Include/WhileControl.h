#pragma once
#include "ProgramControlBase.h"
#include "ChainExpression.h"


class WhileControl: public ProgramControlBase
{
private:
	ChainExpression * while_condition;
public:
	WhileControl(void);
	WhileControl(int control_type);
	void Print(FILE * file);
	void SetWhileCondition(ChainExpression * while_condition);
	~WhileControl(void);
};
