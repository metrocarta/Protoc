#pragma once
#include "../ProgramControlBase.h"

class Module;
class ModuleControl : public ProgramControlBase
{
private:
	Module * module;
public:
	ModuleControl();
	ModuleControl(int control_type, wchar_t * name,Module * module);
	void Print(FILE * file);
	~ModuleControl();
};

