#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif

#include "../Include/ForControl.h"
#include "../Include/ChainExpression.h" 

ForControl::ForControl(void)
:ProgramControlBase()
{
}
ForControl::ForControl(int control_type)
:ProgramControlBase(control_type)
{
	
}
void ForControl::SetExpr1(ChainExpression * chain1)
{
	this->chain1 = chain1;	
}
void ForControl::SetExpr2(ChainExpression * chain2)
{
	this->chain2 = chain2;
}
void ForControl::SetExpr3(ChainExpression * chain3)
{
	this->chain3 = chain3;
}
void ForControl::Print(FILE * file)
{
	wchar_t for_temp[80],temp1[80],temp2[80],temp3[80];
	swprintf(for_temp, 80,L"for_%d", this->id);
	swprintf(temp1, 80,L"expr1: ");
	swprintf(temp2, 80,L"expr2: ");
	swprintf(temp3,80, L"expr3: ");
	fprintf(file,"%ls cond: \n", for_temp);
	fprintf(file,"%ls ", temp1);
	this->chain1->Print(file);
	
	fprintf(file,"%ls ", temp2);
	this->chain2->Print(file);

	fprintf(file,"%ls ", temp3);
	this->chain3->Print(file);

	//print statement
	this->PrintAll(file);
}
ForControl::~ForControl(void)
{
}
