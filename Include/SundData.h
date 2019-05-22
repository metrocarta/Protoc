#pragma once
#include <list>

#include "../Include/Var.h"
#include "../Include/SundCompiler.h"

#define DataSet SundData::GetInstance()

using namespace std;

class SundType;

class SundData
{
private:
	static SundData * instance;
public:
	//enum call_func_error { NOT_ENOUGH_PAR = 1, TOO_MANY_PAR };
	list<label_type> labels; 
	list<Var *> vars;

	SundData(void);
	static SundData * GetInstance();
	int IsLabel(wchar_t * name);
	wchar_t * FindLabel(wchar_t * name);
	//is local var or func. arg
	int IsLocalVar(wchar_t * s, wchar_t * func_name);
	int IsGlobalVar(wchar_t * s);
	Var * GetPtrToFunc(wchar_t * name, wchar_t * func_name);
	void DoDefer(wchar_t * name, wchar_t * func_name);
	Var * GetFuncVar(wchar_t * name);
	Var * GetGlobalVar(wchar_t * name);
	int IsVar(wchar_t * s, wchar_t * func_name);
	int IsVarFunc(wchar_t * name);
	int IsPtrToFunc(wchar_t * ptr_to_func, wchar_t * func_name);
	int IsPtrToFuncAsArg(wchar_t * ptr_to_func, wchar_t * func_name);
	int FindVar(wchar_t * name, wchar_t * func_name, Var * var);
	int FindVar(wchar_t * name, wchar_t * func_name, Var * var, ProgramControlBase * program_control);
	//int CheckFuncSignature(wchar_t * func_name, list<SundType *> * func_signature);
	void SetFuncCalled(wchar_t *func_name);
	void SetFuncDefined(wchar_t *func_name);
	void PrintUndefinedFunc();
	~SundData(void);
};
