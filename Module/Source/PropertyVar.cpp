#include "../Include/PropertyVar.h"
#include "../Include/Module.h"


PropertyVar::PropertyVar()
:Var()
{
	wcscpy(prop_value, L"");
	prop_value_type = PropertyVar::DEC_NUMBER;
}

PropertyVar::PropertyVar(wchar_t * var_name, int type)
:Var(var_name,type)
{
	wcscpy(prop_value, L"");
	level = Module::PUBLIC;
	//size = 1;
	specifier = Module::VARIABLE;
	chain = 0;
	prop_value_type = PropertyVar::DEC_NUMBER;
}
int PropertyVar::HasValue()
{
	int len = (int)wcslen(prop_value);
	if (len > 0) return 1;
	else return 0;
}

PropertyVar::~PropertyVar()
{
}
