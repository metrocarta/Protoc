#include "../Include/ModuleControl.h"
#include "../Include/Module.h"
#include "../Include/ModuleVar.h"
#include "../Include/PropertyVar.h"
#include "../ChainExpression.h"


ModuleControl::ModuleControl()
{
}
ModuleControl::ModuleControl(int control_type, wchar_t * name, Module * module)
:ProgramControlBase(control_type,name)
{
	this->module = module;
}
void ModuleControl::Print(FILE * file)
{
	wchar_t module_temp[200];
	//swprintf(module_temp, 200, L"MODULE: %ls,%ls", module->GetName(), module->GetDevice());
	swprintf(module_temp, 200, L"MODULE: %ls", module->GetName());
	fprintf(file, "%ls\n", module_temp);
	fprintf(file, "PROPERTIES: \n");

	list<PropertyVar> * p_prop = module->GetProperties();
	list<PropertyVar >::iterator it_prop;
	for (it_prop = p_prop->begin(); it_prop != p_prop->end(); it_prop++){
		PropertyVar  prop = (PropertyVar)(*it_prop);
		if (prop.HasValue()){
			fprintf(file, "%ls ", prop.var_name);
			fprintf(file, "%ls \n", prop.GetPropValue());
		}
		else
		if (prop.GetChain() != 0){
			fprintf(file, "%ls ", prop.var_name);
			prop.GetChain()->Print(file);
		}
		else
		if (prop.GetChain()==0)
		{
			fprintf(file, "%ls \n", prop.var_name);
		}
	}

	fprintf(file, "END_PROPERTIES \n");
	//vars
	fprintf(file, "VARIABLES: \n");
	list<ModuleVar> * p_var = module->GetVars();
	list<ModuleVar>::iterator it_var;
	for (it_var = p_var->begin(); it_var != p_var->end(); it_var++){
		ModuleVar m_var = (ModuleVar)(*it_var);
		int specifier = m_var.GetSpecifier();
		int mem_spec = m_var.GetMemSpecifier();
		if (specifier != Var::ARRAY){
			fprintf(file, "%ls %ls mem_spec:%d ", Var::GetTypeName(m_var.GetType()), m_var.var_name,mem_spec);
		}
		else{
			int array_size = m_var.size/Var::GetSize(m_var.GetType());
			fprintf(file, "%ls %ls array:%d mem_spec:%d ", Var::GetTypeName(m_var.GetType()), m_var.var_name, array_size,mem_spec);
		}
		if (m_var.chain != 0)
		    m_var.chain->Print(file);
		else fprintf(file,"\n");
	}
	fprintf(file, "END_VARIABLES \n");
}

ModuleControl::~ModuleControl()
{
}
