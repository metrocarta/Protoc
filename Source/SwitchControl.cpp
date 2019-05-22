#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif

#include "../Include/SwitchControl.h"
#include "../Include/ProgramControlBase.h"
#include "../Include/ChainExpression.h"

SwitchControl::SwitchControl(void)
:ProgramControlBase()
{
	this->switch_expr = 0;
}
SwitchControl::SwitchControl(int control_type)
:ProgramControlBase(control_type)
{
	this->switch_expr = 0;
}
void SwitchControl::Print(FILE * file)
{
	wchar_t switch_temp[80];
	swprintf(switch_temp, 80,L"switch_%d", this->id);
	fprintf(file,"%ls cond: ", switch_temp);

	this->switch_expr->Print(file);

	// print statement
	this->PrintAll(file);
}
void SwitchControl::SetSwitchExpr(ChainExpression * switch_expr)
{
	this->switch_expr = switch_expr;
}
SwitchControl::~SwitchControl(void)
{
}
