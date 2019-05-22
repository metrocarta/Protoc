#include "../Include/Module.h"
#include "../Include/ModuleControl.h"
#include "../Include/PropertyVar.h"
#include "../Include/CodeRunner.h"
#include "../Include/MacroRunner.h"
#include "../Include/BlockRunner.h"
#include "../Include/LibRunner.h"
#include "../Include/ModuleVar.h"
#include "../Include/ISA.h"
#include "../Scanner.h"
#include "../ErrorPrinter.h"
#include "../SundCompiler.h"
#include "../ProgramArgumentsParser.h"
#include <stdio.h>
#include <wchar.h>
#include "../Var.h"

Module * Module::instance = NULL;
Module::Module()
{
	this->properties.clear();
	this->vars.clear();
	this->labels.clear();
	wcscpy(name, L"");
	wcscpy(device, L"");
	wcscpy(super_module_name, L"");
	instance = this;
}
Module::Module(Scanner * scanner)
{
	this->scanner = scanner;
	wcscpy(name, L"");
	wcscpy(device, L"");
	wcscpy(super_module_name, L"");
	instance = this;
	this->moduleParser = new ModuleParser(scanner,this);
	this->properties.clear();
	this->vars.clear();
	this->labels.clear();

	// handles code: module_name
	scanner->GetToken();
	if (scanner->token_type != SundCompiler::IDENTIFIER){
		// print error
		Err->Print(ErrorPrinter::MODULE_HEADER_NAME_OR_DEVICE, scanner->token);
		return;
	}
	wcscpy(name,scanner->token);
	// handles code: super_module_name, if exist
	scanner->GetToken();
	if (*scanner->token == '{'){
		scanner->Putback();
		return;
	}
	if (scanner->tok != SundCompiler::EXTENDS){
		// print error
		Err->Print(ErrorPrinter::MODULE_HEADER_NAME_OR_DEVICE, scanner->token);
		return;
	}
	scanner->GetToken();
	if (scanner->token_type != SundCompiler::IDENTIFIER){
		// print error
		Err->Print(ErrorPrinter::MODULE_HEADER_NAME_OR_DEVICE, scanner->token);
		return;
	}
	wcscpy(super_module_name, scanner->token);

	
}
ProgramControlBase  *  Module::DoStatement()
{
	ModuleControl * module_control = new ModuleControl(ProgramControlBase::MODULE,name,this);
	// handles '{'
	scanner->GetToken();
	if (*(scanner->token) != '{'){
		// print error
		Err->Print(ErrorPrinter::MODULE_BODY_SYNTAX, scanner->token);
		return 0;
	}

	while (*(scanner->token) != '}' && scanner->tok != SundCompiler::FINISHED)
	{
		scanner->GetToken();
		if (scanner->token_type == SundCompiler::MODULE_DIR)
		{
			// '@' , now get module directive
			scanner->GetToken();
			HandleDirective(scanner->tok);
		}
		else
			if (*(scanner->token) != '}' && scanner->tok != SundCompiler::FINISHED)
		{
			// print error : no '@'
			Err->Print(ErrorPrinter::MODULE_BODY_SYNTAX, scanner->token);
			return 0;
		}
	}
	if (*(scanner->token) == '}'){
		// skip token 
		scanner->GetToken();
	}
	if (scanner->tok == SundCompiler::FINISHED){
		// print error
		Err->Print(ErrorPrinter::MODULE_BODY_SYNTAX, scanner->token);
		return 0;
	}
	//list<PropertyVar> * p = (list<PropertyVar> *)&properties;
	//p = 0;
	list<ModuleVar> * p = (list<ModuleVar> *)&vars;
	p = 0;
	return module_control;
}
void Module::SkipSemicolon()
{
	if (*scanner->token == ';'){
		// skip semicolon
		scanner->GetToken();
	}
}
void Module::HandleDirective(int tok)
{
	switch (tok){
	case SundCompiler::PROPERTY:
		HandleProperty();
		SkipSemicolon();
		break;
	case SundCompiler::VAR:
		HandleVar();
		SkipSemicolon();
		break;
	case SundCompiler::CODE:
		HandleCode();
		break;
	case SundCompiler::MACRO:
		HandleMacro();
		break;
	case SundCompiler::CODE_BLOCK:
		break;
	case SundCompiler::LIB:
		break;
	default:
		break;
	}
}
void Module::HandleProperty()
{
	scanner->GetToken();
	if (!SundCompiler::IsRegularType(scanner->tok)){
		// print error
		Err->Print(ErrorPrinter::MODULE_PROPERTY_SYNTAX, scanner->token);
		return;
	}
	DeclProperty();
}
void Module::HandleVar()
{
	wchar_t temp[80];
	int counter = 0;
	do{
		ModuleVar *moduleVar = new ModuleVar();
		scanner->GetToken();
		if (!SundCompiler::IsRegularType(scanner->tok)){
			// print error
			Err->Print(ErrorPrinter::MODULE_VAR_SYNTAX, scanner->token);
			return;
		}
		moduleVar->basic_type = scanner->tok;
		moduleVar->value = 0;
		moduleVar->size = Var::GetSize(scanner->tok);
		moduleVar->program_control = 0;
		// is next token '<' ?
		scanner->GetToken();
		if (*(scanner->token) != LT){
			// print error
			return;
		}
		// eval. memory type and address data
		EvaluateVarMemSpecifier(moduleVar);

		// get next token, is identifier?
		scanner->GetToken();
		if (scanner->token_type != SundCompiler::IDENTIFIER){
			//  print error
			Err->Print(ErrorPrinter::MODULE_VAR_NOT_IDENTIFIER, scanner->token);
			return;
		}
		wcscpy(moduleVar->var_name, scanner->token);
		scanner->ReadNextToken(temp);
		if (*temp == '[')
		{
			// array
			int array_length;
			scanner->GetToken();
			if (!CheckIsNumberNextToken(&array_length)){
				// print error
				Err->Print(ErrorPrinter::MODULE_VAR_SYNTAX, scanner->token);
				return;
			}
			moduleVar->SetSpecifier(Var::ARRAY);
			moduleVar->size = moduleVar->size*array_length;
			if (*scanner->token != ']'){
				// print error
				Err->Print(ErrorPrinter::MODULE_VAR_SYNTAX, scanner->token);
				return;
			}
		}
		scanner->GetToken();
		this->vars.push_back(*moduleVar);
		counter++;
	} while (*scanner->token==',');
	scanner->Putback();
}
void Module::EvaluateVarMemSpecifier(ModuleVar * moduleVar)
{
	int value = -1;
	moduleVar->chain = 0;
	if (!CheckIsNumberNextToken(&value)){
		// print error
		Err->Print(ErrorPrinter::MODULE_VAR_SYNTAX, scanner->token);
		return;
	}
	moduleVar->op_code_index = value;

	if (*(scanner->token) == GT){
	    //scanner->GetToken();
		return;
	}
	else
	if (*(scanner->token) != ','){
		// print error
		Err->Print(ErrorPrinter::MODULE_VAR_SYNTAX, scanner->token);
		return ;
	}
	else{
		// type var<number,expr>
		ScannerState * state = new ScannerState(scanner);
		moduleVar->chain = moduleParser->PerformModuleExp(state);
		scanner->GetToken();
		if (*(scanner->token) != GT){
			// print error
			Err->Print(ErrorPrinter::MODULE_VAR_SYNTAX);
			return;
		}
	}
}
int Module::CheckIsNumberNextToken(int * value)
{
	int result = 1,ret_value;
	ISA * isa = ISA::CreateISAObject(ProgramArgumentsParser::GetInstance()->GetPlatform());
	PropertyVar var;
	scanner->GetToken();
	int op_code_type = isa->GetOpCodeGroupIndex(scanner->token);
	if(op_code_type) {
		*value = op_code_type;
		scanner->GetToken();
		return result;
	}
	switch (scanner->token_type){
	case SundCompiler::NUMBER:
		*value = _wtoi(scanner->token);
		scanner->GetToken();
		break;
	case SundCompiler::HEX_NUMBER:
		*value = wcstoul(scanner->token, 0, 16);
		scanner->GetToken();
		break;
	default:
		ret_value = GetPropertyVarData(scanner->token, &var);
		if (!var.HasValue() || !ret_value){
			return 0;
		}
		*value = var.value;
		scanner->GetToken();
		break;
	}
	return result;
}
void Module::HandleCode()
{
	CodeRunner * codeRunner = new CodeRunner(scanner);
	codeRunner->Run();
}
void Module::HandleLib()
{
	LibRunner * libRunner = new LibRunner(scanner);
	libRunner->Run();
}
void Module::HandleMacro()
{
	MacroRunner * macroRunner = new MacroRunner(scanner);
	macroRunner->Run();
}
void Module::HandleBlock()
{
	BlockRunner * blockRunner = new BlockRunner(scanner);
	blockRunner->Run();
}
void Module::DeclProperty()
{
	do{
		ScannerState * state = new ScannerState(scanner);
		ChainExpression * chain = moduleParser->PerformExp(state);
		scanner->GetToken();
		
	} while (*(scanner->token) == ',' && *(scanner->token) != '\n' && *(scanner->token) != '\0');
	if (*(scanner->token) == '\0'){
		// print error
		Err->Print(ErrorPrinter::MODULE_PROPERTY_SYNTAX);
		return;
	}
	scanner->Putback();
}
int Module::IsPropertyVar(wchar_t * var_name)
{
	list<PropertyVar >::iterator it;
	for (it = properties.begin(); it != properties.end(); it++){
		if (!wcscmp(var_name,it->var_name)){
			return 1;
		}
	}
	return 0;
}
int Module::GetPropertyVarData(wchar_t * var_name, PropertyVar * var)
{
	list<PropertyVar >::iterator it;
	for (it = properties.begin(); it != properties.end(); it++){
		if (!wcscmp(var_name, it->var_name)){
			var->Copy((Var *)&(*it));
			wcscpy(var->prop_value,it->prop_value);
			var->prop_value_type = it->prop_value_type;
			return 1;
		}
	}
	return 0;
}
int Module::GetModuleVarData(wchar_t * var_name, ModuleVar * var)
{
	list<ModuleVar >::iterator it;
	for (it = vars.begin(); it != vars.end(); it++) {
		if (!wcscmp(var_name, it->var_name)) {
			var->Copy((Var *)&(*it));
			//wcscpy(var->prop_value, it->prop_value);
			//var->prop_value_type = it->prop_value_type;
			return 1;
		}
	}
	return 0;
}
Module * Module::GetInstance()
{
	return instance;
}
void Module::DeclLabel(wchar_t * name)
{
	// label definition
	struct label_type label_node;
	wcscpy(label_node.label_name, name);
	wcscpy(label_node.func_name, L"");
	if (IsExistLabel(name)) {
		// print error
		Err->Print(ErrorPrinter::MODULE_LABEL_REDEFINED, name);
		return;
	}
	this->labels.push_back(label_node);
}
int Module::IsExistLabel(wchar_t * name)
{
	list<label_type>::iterator it;
	for (it = this->labels.begin(); it != this->labels.end(); it++) {
		if (!wcscmp(name, (it)->label_name)) {
			return 1;
		}
	}
	return 0;
}
Module::~Module()
{
}
