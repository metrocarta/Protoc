#pragma once
#include "../Var.h"

class ChainExpression;

class PropertyVar :	public Var
{
private:
	wchar_t prop_value[200];
	int prop_value_type;
	ChainExpression * chain;
public:
	enum property_types { DEC_NUMBER = 0, HEX_NUMBER };
	PropertyVar();
	PropertyVar(wchar_t * var_name, int type);
	~PropertyVar();
	int HasValue();
	wchar_t  * GetPropValue(){ return prop_value; };
	ChainExpression * GetChain(){ return chain; };
	friend class ModuleParser;
	friend class Module;
};

