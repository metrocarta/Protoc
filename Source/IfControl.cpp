#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif

#include "../Include/ProgramControlBase.h"
#include "../Include/IfControl.h"
#include "../Include/ChainExpression.h"


IfControl::IfControl(void)
:ProgramControlBase()
{
	if_condition = 0;
	else_control = 0;
}
IfControl::IfControl(int control_type)
:ProgramControlBase(control_type)
{
	if_condition = 0;
	else_control = 0;
}
void IfControl::Print(FILE * file)
{
	wchar_t if_temp[80],else_temp[80];
	swprintf(if_temp,80, L"if_%d", this->id);
	swprintf(else_temp, 80,L"else_%d", this->id);
	fprintf(file,"%ls cond: ", if_temp);
	
	if(if_condition!=0){
		if_condition->Print(file);
	}
	this->PrintAll(file);
	if(else_control!=0){
		fprintf(file,"%s: \n", else_temp);
		else_control->PrintAll(file);
	}
}
void IfControl::SetIfCondition(ChainExpression * if_condition)
{
	this->if_condition = if_condition;
}
void IfControl::SetElseControl(ProgramControlBase * else_control)
{
	this->else_control = else_control;
}
IfControl::~IfControl(void)
{
}
