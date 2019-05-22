#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif

#include "../Include/ProgramControlBase.h"
#include "../Include/ChainExpression.h"
#include "../Include/ControlCounter.h"
#include "../Include/SundTypeBuilder.h"

ProgramControlBase * ProgramControlBase::global_instance = 0;

ProgramControlBase::ProgramControlBase(void)
{
	this->control_type = BLOCK;
	this->expr = 0;
	wcscpy(this->name, L"");
	this->parent = 0;
	sequence.clear();
	string = 0;
}
ProgramControlBase::ProgramControlBase(int control_type)
{
	this->control_type = control_type;
	this->expr = 0;
	wcscpy(this->name, L"");
	this->parent = 0;
	sequence.clear();
	string = 0;
}
ProgramControlBase::ProgramControlBase(int control_type,ChainExpression * expr)
{
	this->control_type = control_type;
	this->expr = expr;
	wcscpy(this->name, L"");
	this->parent = 0;
	sequence.clear();
	string = 0;
}
ProgramControlBase::ProgramControlBase(int control_type,wchar_t * name)
{
	this->control_type = control_type;
	this->expr = 0;
	wcscpy(this->name, name);
	this->parent = 0;
	sequence.clear();
	string = 0;
}
ProgramControlBase::ProgramControlBase(int control_type, ChainExpression * expr, wchar_t * name)
{
	this->control_type = control_type;
	this->expr = expr;
	wcscpy(this->name, name);
	this->parent = 0;
	sequence.clear();
	string = 0;
}

ProgramControlBase::ProgramControlBase(int control_type, SundTypeBuilder * builder)
{
	this->control_type = control_type;
	this->expr = 0;
	wcscpy(this->name, L"");
	this->parent = 0;
	sequence.clear();
	this->builder = builder;	// sund_type->Copy(sund_type);
	string = 0;
}
ProgramControlBase::ProgramControlBase(int control_type, SundTypeBuilder * builder,int init_list_size, ChainExpression ** init_list)
{
	this->control_type = control_type;
	this->expr = 0;
	wcscpy(this->name, L"");
	this->parent = 0;
	sequence.clear();
	this->builder = builder;
	this->init_list_size = init_list_size;
	this->init_list = init_list;
	string = 0;
}
void ProgramControlBase::SetId(int id)
{
	this->id = id;
}
int ProgramControlBase::GenerateId(int control_type)
{
	switch(control_type){
		case IF:
			return (ProgramControlCounter->if_counter)++;
		case WHILE:
			return (ProgramControlCounter->while_counter)++;
		case DO:
			return (ProgramControlCounter->do_counter)++;
		case SWITCH:
			return (ProgramControlCounter->switch_counter)++;
		case FOR:
			return (ProgramControlCounter->for_counter)++;
		case BLOCK:
			return (ProgramControlCounter->block_counter)++;
		case STRING:
			return (ProgramControlCounter->string_counter)++;
	}
	return -1;
}
void ProgramControlBase::SetName(wchar_t * name)
{
	wcscpy(this->name, name);
}
void ProgramControlBase::SetExpr(ChainExpression * expr)
{
	this->expr = expr;
}
void ProgramControlBase::Add(ProgramControlBase * control)
{
	sequence.push_back(control);
}
void ProgramControlBase::PrintAll(FILE * file)
{
	list<ProgramControlBase *>::iterator it;
	for (it = sequence.begin(); it != sequence.end(); it++)
	{
		(*it)->Print(file);
	}
}
void ProgramControlBase::PrintAll(FILE * file,wchar_t * title)
{
	// first, print a title
	fprintf(file, "%ls",title);
	this->PrintAll(file);
}
void ProgramControlBase::Print(FILE * file)
{
	wchar_t temp[80];
	switch(control_type){
		case EXPR:
			if(this->expr!=0)
			{
				this->expr->Print(file);
			}
			break;
		case FUNC:
			fprintf(file,"function: %ls \n", this->name);
			break;
		case RETURN:
			fprintf(file,"return: ");
			if(this->expr!=0)
			{
				this->expr->Print(file);
			}
			break;
		case CASE:
			swprintf(temp,80, L"case_%d ", 1);
			fprintf(file,"%ls", temp);
			if(this->expr!=0){
				this->expr->Print(file);
			}
			break;
		case DEFAULT:
			swprintf(temp,80, L"default_%d \n", 1);
			fprintf(file,"%ls", temp);
			break;
		case BREAK:
			swprintf(temp,80, L"break_%d %ls\n", 1, name);
			fprintf(file,"%s", temp);
			return;
		case CONTINUE:
			swprintf(temp,80, L"continue_%d %ls\n", 1, name);
			fprintf(file,"%ls", temp);
			return;
		case GOTO:
			swprintf(temp,80, L"goto_%d %ls\n", 1, name);
			fprintf(file,"%ls", temp);
			return;
		case LABEL:
			swprintf(temp,80, L"label_%d %ls\n", 1, name);
			fprintf(file,"%ls", temp);
			return;
		case DECL:
			if (this->builder != 0){
				this->builder->Print(file);
			}
			return;
		case INITIALIZER:
			if (this->builder != 0 && this->init_list!= 0){
				this->builder->Print(file);
				for (int i = 0; i < this->init_list_size;i++){
					fprintf(file,"%d: ",i);
					if (*(this->init_list + i) != 0)
					   (*(this->init_list + i))->Print(file);
					else fprintf(file, "0\n");
				}
			}
			return;
		case STRING:
			fprintf(file,"string_%d\n",id);
			for (int i = 0; i < this->string->length; i++){
				fprintf(file, "%c %d\n", this->string->value[i], this->string->value[i]);
			}
			fprintf(file,"0\n");
			return;
		default:
			break;
	}
	// print whole sequence
	this->PrintAll(file);
}
wchar_t const * ProgramControlBase::GetControlTypeName()
{
	switch(control_type){
		case BLOCK:
			return L"block";
		case MAIN_PROGRAM:
			return L"main program";
		case EXPR:
			return L"expression";
		case FUNC:
			return L"function";
		case IF:
			return L"if";
		case ELSE:
			return L"else";
		case WHILE:
			return L"while";
		case DO:
			return L"do";
		case FOR:
			return L"for";
		case SWITCH:
			return L"switch";
		case CASE:
			return L"case";
		case DEFAULT:
			return L"default";
		case GOTO:
			return L"goto";
		case LABEL:
			return L"label";
		case RETURN:
			return L"return";
		case BREAK:
			return L"break";
		case CONTINUE:
			return L"continue";
		default:
			return L"Unknown type";
	}
}
void ProgramControlBase::SetString(wchar_t * value, int length)
{
	this->string = new sund_string();
	this->string->value = new char[length + 1];
	this->string->length = length;
	wcstombs(this->string->value, value, length);
}
void ProgramControlBase::SetStartStmt(ChainExpression * start_stmt)
{
	this->start_stmt = start_stmt;
}
void ProgramControlBase::SetEndStmt(ChainExpression * end_stmt)
{
	this->end_stmt = end_stmt;
}
ProgramControlBase::~ProgramControlBase(void)
{
}
