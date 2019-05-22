#include "../Include/Var.h"
#include "../Include/SundCompiler.h"
#include "../Include/SundTypeBuilder.h"

sund_string::sund_string()
{
	this->value = 0;
	this->length = 0;
}
sund_string::sund_string(char * value, int length)
{
	this->value = new char[length+1];
	strcpy(this->value,value);
	this->length = length;
}
// class Var
Var::Var(void)
{
	wcscpy(var_name, L"");
	wcscpy(func_name, L"");
	basic_type = 0;
	sund_type = 0;
	value = 0;
	level = Var::LOCAL;
	//func_index = -1;
	program_control = 0;
	size = 0;
	qualifier = -1;
	specifier = VARIABLE;
	storage_class = -1;
	op_code_index = -1;
	position = -1;
	//is_pointer = 0;
	is_defined = 0;
	is_called = 0;
	//func_signature = 0;
	//string = 0;
	is_deref_ptr_to_func = 0;
}
Var::Var(wchar_t * var_name, int type)
{
	wcscpy(this->var_name, var_name);
	wcscpy(func_name, L"");
	this->basic_type = type;
	sund_type = 0;
	value = 0;
	level = Var::LOCAL;
	//func_index = -1;
	program_control = 0;
	size = Var::GetSize(this->basic_type);//2;
	qualifier = -1;
	specifier = VARIABLE;
	storage_class = -1;
	op_code_index = -1;
	position = -1;
	//is_pointer = 0;
	is_defined = 0;
	is_called = 0;
	//func_signature = 0;
	//string = 0;
	is_deref_ptr_to_func = 0;
}
Var::Var(wchar_t * var_name, int type, int qualifier)
{
	wcscpy(this->var_name, var_name);
	wcscpy(func_name, L"");
	this->basic_type = type;
	sund_type = 0;
	value = 0;
	level = Var::LOCAL;
	//func_index = -1;
	program_control = 0;
	size = Var::GetSize(this->basic_type);//2;
	this->qualifier = qualifier;
	this->specifier = VARIABLE;
	storage_class = -1;
	op_code_index = -1;
	position = -1;
	//is_pointer = 0;
	is_defined = 0;
	is_called = 0;
	//func_signature = 0;
	//string = 0;
	is_deref_ptr_to_func = 0;
}
Var::Var(wchar_t * var_name, int type, int qualifier, int value)
{
	wcscpy(this->var_name, var_name);
	wcscpy(func_name, L"");
	this->basic_type = type;
	sund_type = 0;
	this->value = value;
	level = Var::LOCAL;
	//func_index = -1;
	program_control = 0;
	size = Var::GetSize(this->basic_type);//2;
	this->qualifier = qualifier;
	this->specifier = VARIABLE;
	storage_class = -1;
	op_code_index = -1;
	position = -1;
	//is_pointer = 0;
	is_defined = 0;
	is_called = 0;
	//func_signature = 0;
	//string = 0;
	is_deref_ptr_to_func = 0;
}
int Var::HasParentProgramControl(ProgramControlBase * program_control)
{
	ProgramControlBase * p_c = program_control;
	while (p_c != 0)
	{
		if (p_c == this->program_control)
			return 1;
		p_c = p_c->parent;
	}
	return 0;
}
void Var::SetValue(int value)
{
	this->value = value;
}
void Var::SetVarName(int value)
{
	wchar_t buffer[80];
	swprintf(buffer, 80, L"CONST_%d", value);
	wcscpy(this->var_name, buffer);
}
void Var::SetName(wchar_t * name)
{
	wcscpy(this->var_name, name);
}
void Var::SetSpecifier(int specifier)
{
	this->specifier = specifier;
}
void Var::SetQualifier(int qualifier)
{
	this->qualifier = qualifier;
}
void Var::SetStorageClass(int storage_class)
{
	this->storage_class = storage_class;
}
void Var::Copy(Var *var)
{
	wcscpy(this->var_name, var->var_name);
	wcscpy(this->func_name, var->func_name);
	this->basic_type = var->basic_type;
	this->sund_type = new SundType();
	this->sund_type->Copy(var->sund_type);
	this->value = var->value;
	this->level = var->level;
	//this->sign = var.sign;
	//this->func_index = var->func_index;
	this->program_control = var->program_control;
	this->size = var->size;
	this->qualifier = var->qualifier;
	this->specifier = var->specifier;
	this->storage_class = var->storage_class;
	this->op_code_index = var->op_code_index;
	this->position = var->position;
	this->is_defined = var->is_defined;
	this->is_called = var->is_called;
	this->is_deref_ptr_to_func = var->is_deref_ptr_to_func;
}
int Var::IsConst()
{
	if(qualifier==CONST){
		return 1;
	}
	else{
		return 0;
	}
}
/*int Var::IsPointer()
{
	return (this->is_pointer>0);
}*/
int Var::GetSize(int type)
{
	switch(type){
	case SundCompiler::CHAR:
	case SundCompiler::BYTE:
			return 1;
	case SundCompiler::INT:
	case SundCompiler::WORD:
			return 2;
		default:
			return 1;
	}
}
int Var::GetSize(wchar_t * type_name)
{
	if (!wcscmp(type_name, L"char")){
		return 1;
	}
	if (!wcscmp(type_name, L"byte")){
		return 1;
	}
	if (!wcscmp(type_name, L"word")){
		return 2;
	}
	if (!wcscmp(type_name, L"int")){
		return 2;
	}
	return 0;
}
wchar_t * Var::GetTypeName(int type)
{
	switch (type){
	case SundCompiler::CHAR:
		return L"char";
	case SundCompiler::BYTE:
		return L"byte";
	case SundCompiler::INT:
		return L"int";
	case SundCompiler::WORD:
		return L"word";
	default:
		return L"";
	}
}
void Var::SetFuncName(wchar_t *func_name)
{
	wcscpy(this->func_name, func_name);
}
void Var::SetFuncCalled()
{
	this->is_called = 1;
}
void Var::SetFuncDefined()
{
	this->is_defined = 1;
}
int Var::CheckFuncSignature(list<SundType *> * func_signature)
{
	list<sund_type_node *>::iterator tmp;
	int size1 = func_signature->size();
	tmp = sund_type->sund_type_list.begin();
	if ((*tmp)->type_code == SundCompiler::POINTER)
		tmp++;
	int size2 = (*tmp)->func_signature->size();
	if (size1 < size2)	return NOT_ENOUGH_PAR;
	if (size1 > size2)  return TOO_MANY_PAR;

	return 0;
}
Var::~Var(void)
{
}

