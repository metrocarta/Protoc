#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif

#include "../Include/DoControl.h"

DoControl::DoControl(void)
:ProgramControlBase()
{
	while_condition = 0;
}
DoControl::DoControl(int control_type)
:ProgramControlBase(control_type)
{
	while_condition = 0;
}
void DoControl::Print(FILE * file)
{
	wchar_t do_temp[80],while_temp[80];
	swprintf(do_temp,80, L"do_%d", this->id);
	fprintf(file,"%ls\n", do_temp);
	//print statement
	this->PrintAll(file);
	if(while_condition!=0){
		swprintf(while_temp,80, L"while_%d cond: ", this->id);
		fprintf(file,"%ls ", while_temp);
		while_condition->Print(file);
	}
}
void DoControl::SetWhileCondition(ChainExpression * while_condition)
{
	this->while_condition = while_condition;
}
DoControl::~DoControl(void)
{
}

