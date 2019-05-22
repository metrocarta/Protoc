#pragma once
#include "ModuleParser.h"

#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif
#include <list>

using namespace std;

class Scanner;
class PropertyVar;
class ModuleControl;
class ProgramControlBase;
class ModuleVar;
struct label_type;


class Module
{
private:
	Scanner * scanner;
	wchar_t name[200];
	wchar_t device[200];
	wchar_t super_module_name[200];
	ModuleParser * moduleParser;
	list<PropertyVar> properties;
	list<ModuleVar> vars;
	list<label_type> labels;
	static Module * instance;
	void DeclProperty();
	void HandleDirective(int tok);
	void HandleProperty();
	void HandleVar();
	void EvaluateVarMemSpecifier(ModuleVar * moduleVar);
	void HandleCode();
	void HandleMacro();
	void HandleBlock();
	void HandleLib();
	int CheckIsNumberNextToken(int * value);
	void SkipSemicolon();
	int IsPropertyVar(wchar_t * var_name);
	friend class ModuleParser;
public:
	enum  level_types { PUBLIC = 0, PRIVATE, PROTECTED };
	enum  specifiers { VARIABLE = 0, ARRAY};
	Module();
	Module(Scanner * scanner);
	ProgramControlBase  * DoStatement();
	wchar_t * GetName(){ return name; };
	wchar_t * GetDevice(){ return device; };
	list<PropertyVar> * GetProperties(){ return &properties; };
	list<ModuleVar> *GetVars(){ return &vars; };
	int GetPropertyVarData(wchar_t * var_name, PropertyVar * var);
	int GetModuleVarData(wchar_t * var_name, ModuleVar * var);
	int IsExistLabel(wchar_t * name);
	void DeclLabel(wchar_t * name);
	static Module * GetInstance();
	~Module();
};

