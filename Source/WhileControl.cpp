#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif

#include "../Include/WhileControl.h"

WhileControl::WhileControl(void)
:ProgramControlBase()
{
	while_condition = 0;
}
WhileControl::WhileControl(int control_type)
:ProgramControlBase(control_type)
{
	while_condition = 0;
}
void WhileControl::Print(FILE * file)
{
	wchar_t while_temp[80];
	swprintf(while_temp,80, L"while_%d", this->id);
	fprintf(file,"%ls cond: ", while_temp);
	if(while_condition!=0){
		while_condition->Print(file);
	}
	//print statement
	this->PrintAll(file);
}
void WhileControl::SetWhileCondition(ChainExpression * while_condition)
{
	this->while_condition = while_condition;
}
WhileControl::~WhileControl(void)
{
}
