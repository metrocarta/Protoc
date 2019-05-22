#pragma once

#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif

#include <string.h> 
#include <stack>
#include <list>

using namespace std;

#define VAR_NAME_LEN 80
class SundCompiler;
class ProgramControlBase;
class SundType;

struct sund_string{
	char * value;
	int length;
	sund_string();
	sund_string(char * value,int length);
};
class Var
{
private:
	// proto_c extension
	int op_code_index, position;
protected:
	int basic_type;
	SundType * sund_type;
	int qualifier;
	int value;
	int level;		//GLOBAL,LOCAL,ARG
	int specifier;
	int storage_class;//AUTO,STATIC,REGISTER
	int is_defined,is_called;
	//list<FuncSignature *> * func_signature;// pointer to function type (*fname)()
	int HasParentProgramControl(ProgramControlBase * program_control);
public:
	enum call_func_error { NOT_ENOUGH_PAR = 1, TOO_MANY_PAR, WRONG_ARG_TYPE };
	wchar_t var_name[VAR_NAME_LEN];
	// var is declared in func body (func_name)
	wchar_t func_name[VAR_NAME_LEN];
	// is dereferenced pointer to function?
	int is_deref_ptr_to_func;
	ProgramControlBase * program_control;
	int size;
	enum level_types {GLOBAL,LOCAL,ARG};
	enum  qualifiers {CONST,VOLATILE};
	enum  specifiers {VARIABLE=0,FUNCTION,ARRAY,LABEL};
	Var(void);
	Var(wchar_t * var_name,int type);
	Var(wchar_t * var_name, int type, int qualifier);
	Var(wchar_t * var_name, int type, int qualifier, int value);
	void SetValue(int value);
	int  GetValue(){ return value; }
	void SetVarName(int value);
	void SetName(wchar_t * name);
	void SetSpecifier(int specifier);
	int  GetSpecifier(){return specifier;}
	void SetQualifier(int qualifier);
	void SetStorageClass(int storage_class);
	int IsConst();
	void Copy(Var *var);
	static int GetSize(int type);
	static int GetSize(wchar_t * type_name);
	int GetType(){ return basic_type; };
	static wchar_t * GetTypeName(int type);
	int GetMemSpecifier(){ return op_code_index; };
	SundType * GetSundType(){ return sund_type; };
	void SetFuncName(wchar_t *func_name);
	void SetFuncCalled();
	void SetFuncDefined();
	int CheckFuncSignature(list<SundType *> * func_signature);
	~Var(void);
	friend class SundCompiler;
	friend class SundData;
	friend class IntermediateNode;
	friend class Module;
	friend class ModuleParser;
	friend class SundTypeBuilder;
};
