#include "../Include/SundData.h"
#include "../Include/Var.h"
#include "../Include/SundTypeBuilder.h"
#include "../Include/ErrorPrinter.h"

SundData * SundData::instance = NULL;

SundData::SundData(void)
{
	labels.clear();
	vars.clear();

	instance = this;
}
int SundData::IsLocalVar(wchar_t * s, wchar_t * func_name)
{
	// check local and args
	list<Var *>::iterator it;
	for (it = vars.begin(); it != vars.end(); it++){
		//if (fi == (*it)->func_index){
		if (!wcscmp(func_name, (*it)->func_name)){
			if (!wcscmp(s, (*it)->var_name)){
				return 1;
			}
		}
	}
	return 0;
}
int SundData::IsGlobalVar(wchar_t * s)
{
	// check global vars
	list<Var *>::iterator it;
	for (it = vars.begin(); it != vars.end(); it++){
		if ((*it)->level == Var::GLOBAL){
			if (!wcscmp(s, (*it)->var_name)){
				return 1;
			}
		}
	}
	return 0;
}
Var * SundData::GetGlobalVar(wchar_t * name)
{
	// get global vars
	static list<Var *>::iterator it;
	for (it = vars.begin(); it != vars.end(); it++){
		if ((*it)->level == Var::GLOBAL){
			if (!wcscmp(name, (*it)->var_name)){
				return (*it);
			}
		}
	}
	return 0;
}
Var * SundData::GetPtrToFunc(wchar_t * name, wchar_t * func_name)
{
	list<Var *>::iterator it;
	Var * result = 0;
	for (it = vars.begin(); it != vars.end(); it++){
		// is local ptr to function
		if (!wcscmp(name, (*it)->var_name) && !wcscmp(func_name, (*it)->func_name) && (*it)->sund_type->IsPtrToFunc()){
			//return (*it);  
			result = new Var();
			result->Copy(*it);
			return result;
		}
		// is global ptr to function
		if (!wcscmp(name, (*it)->var_name) && !wcscmp(L"", (*it)->func_name) && (*it)->sund_type->IsPtrToFunc()){ 
			result = new Var();
			result->Copy(*it);
			return result;
		}
	}
	return result;
}
void SundData::DoDefer(wchar_t * name, wchar_t * func_name)
{
	list<Var *>::iterator it;
	Var * result = 0;
	for (it = vars.begin(); it != vars.end(); it++){
		// is local ptr to function
		if (!wcscmp(name, (*it)->var_name) && !wcscmp(func_name, (*it)->func_name) && (*it)->sund_type->IsPtrToFunc()){
			(*it)->is_deref_ptr_to_func = 1;
			//(*it)->GetSundType()->sund_type_list.pop_back();
		}
		// is global ptr to function
		if (!wcscmp(name, (*it)->var_name) && !wcscmp(L"", (*it)->func_name) && (*it)->sund_type->IsPtrToFunc()){
			(*it)->is_deref_ptr_to_func = 1;
			//(*it)->GetSundType()->sund_type_list.pop_back();
		}
	}
}
Var * SundData::GetFuncVar(wchar_t * name)
{
	list<Var *>::iterator it;
	Var * result = 0;
	for (it = vars.begin(); it != vars.end(); it++){
		if (!wcscmp(name, (*it)->var_name) && (*it)->sund_type->IsFunc()){
			result = new Var();
			result->Copy(*it);
			return result;
		}
	}
	return 0;
}
int SundData::IsVarFunc(wchar_t * name)
{
	list<Var *>::iterator it;
	for (it = vars.begin(); it != vars.end(); it++){
		if (!wcscmp(name, (*it)->var_name) && (*it)->sund_type->IsFunc()){
			return 1;
		}
	}
	return 0;
}
int SundData::IsPtrToFunc(wchar_t * ptr_to_func, wchar_t * func_name)
{
	list<Var *>::iterator it;
	for (it = vars.begin(); it != vars.end(); it++){
		if (!wcscmp(ptr_to_func, (*it)->var_name) && !wcscmp((*it)->func_name,func_name)){
			return 1;
		}
	}
	return 0;
}
int SundData::IsPtrToFuncAsArg(wchar_t * ptr_to_func, wchar_t * func_name)
{
	return 0;
}

void SundData::SetFuncCalled(wchar_t *func_name)
{
	list<Var *>::iterator it;
	for (it = vars.begin(); it != vars.end(); it++){
		if (!wcscmp(func_name, (*it)->var_name) && (*it)->sund_type->IsFunc()){
			(*it)->SetFuncCalled();
		}
	}
}
void SundData::SetFuncDefined(wchar_t *func_name)
{
	list<Var *>::iterator it;
	for (it = vars.begin(); it != vars.end(); it++){
		if (!wcscmp(func_name, (*it)->var_name) && (*it)->sund_type->IsFunc()){
			(*it)->SetFuncDefined();
		}
	}
}
void SundData::PrintUndefinedFunc()
{
	list<Var *>::iterator it;
	int i = 0;
	for (it = vars.begin(); it != vars.end(); it++,i++){
		if ((*it)->sund_type->IsFunc() && (*it)->is_called==1 && (*it)->is_defined==0){
			Err->AddError(ErrorPrinter::UNRESOLVED_EXTERN, (*it)->var_name);
		}
	}
}
int SundData::IsVar(wchar_t * s, wchar_t * func_name)
{
	// check local and args
	list<Var *>::iterator it;
	for (it = vars.begin(); it != vars.end(); it++){
		if (!wcscmp(func_name, (*it)->func_name)){
			if (!wcscmp(s, (*it)->var_name)){
				return 1;
			}
		}
	}
	// check global vars
	for (it = vars.begin(); it != vars.end(); it++){
		if ((*it)->level == Var::GLOBAL){
			if (!wcscmp(s, (*it)->var_name)){
				return 1;
			}
		}
	}
	return 0;
}

int SundData::FindVar(wchar_t * name, wchar_t * func_name, Var * var)
{
	int result = 0;
	// check local and args
	list<Var *>::iterator it;
	for (it = vars.begin(); it != vars.end(); it++){
		if (!wcscmp(func_name, (*it)->func_name)){
			if (!wcscmp(name, (*it)->var_name)){
				var->Copy(*it);
				return 1;
			}
		}
	}
	// check global vars
	for (it = vars.begin(); it != vars.end(); it++){
		if ((*it)->level == Var::GLOBAL){
			if (!wcscmp(name, (*it)->var_name)){
				var->Copy(*it);
				return 1;
			}
		}
	}
	return result;
}
int SundData::FindVar(wchar_t * name, wchar_t * func_name, Var * var, ProgramControlBase * program_control)
{
	int result = 0;
	// check local and args
	list<Var *>::iterator it;
	for (it = vars.begin(); it != vars.end(); it++){
		if (!wcscmp(func_name, (*it)->func_name)){
			if (!wcscmp(name, (*it)->var_name)){
				//check program_control
				if ((*it)->level == Var::ARG || (*it)->HasParentProgramControl(program_control))
				{
					var->Copy(*it);
					return 1;
				}
			}
		}
	}
	// check global vars
	for (it = vars.begin(); it != vars.end(); it++){
		if ((*it)->level == Var::GLOBAL){
			if (!wcscmp(name, (*it)->var_name)){
				var->Copy(*it);
				return 1;
			}
		}
	}
	return result;
}
/*int SundData::CheckFuncSignature(wchar_t * func_name, list<SundType *> * func_signature)
{
	list<Var *>::iterator it;
	int i = 0;
	for (it = vars.begin(); it != vars.end(); it++, i++){
		if (!wcscmp((*it)->var_name, func_name)){
			int size1 = func_signature->size();
			list<sund_type_node *>::iterator tmp_it = (*it)->sund_type->sund_type_list.begin();
			int size2 = (*tmp_it)->func_signature->size();
			if (size1 < size2)	return NOT_ENOUGH_PAR;
			if (size1 > size2)  return TOO_MANY_PAR;
			return 0;
		}
	}
	return 0;
}*/
int SundData::IsLabel(wchar_t * name)
{
	list<struct label_type>::iterator it;
	for(it = labels.begin();it!=labels.end();it++){
		wchar_t * s = it->label_name;
		if (!wcscmp(s, name)){
			return 1;
		}
	}
	return  0;
}
wchar_t * SundData::FindLabel(wchar_t * name)
{
	list<struct label_type>::iterator it;
	for(it = labels.begin();it!=labels.end();it++)
	{
		if (!wcscmp(it->label_name, name)){
			return it->start;
		}
	}
	return NULL;
}
SundData * SundData::GetInstance()
{
	return instance;
}

SundData::~SundData(void)
{
	/*list<Var *>::iterator it;
	for (it = vars.begin(); it != vars.end(); it++)
	{
		Var * vt = *it;
		delete vt;
	}*/
	//vars.clear();
}
