
#pragma once

class ProgramControlBase;
class ChainExpression;

class IfControl : public ProgramControlBase
{
private:
	ChainExpression * if_condition;
	ProgramControlBase * else_control;
public:
	void Print(FILE * file);
	IfControl(void);
	IfControl(int control_type);
	void SetIfCondition(ChainExpression * if_condition);
	void SetElseControl(ProgramControlBase * else_control);
	~IfControl(void);
};
