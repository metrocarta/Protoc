#pragma once
#include "../ProgramControlBase.h"

class I8051ProgramControl :public ProgramControlBase
{
public:
	I8051ProgramControl();
	I8051ProgramControl(int control_type);
	~I8051ProgramControl();
};

