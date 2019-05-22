#pragma once
#include "CodeRunner.h"
class Scanner;

class MacroRunner : public CodeRunner
{
private:
public:
	MacroRunner();
	MacroRunner(Scanner * scanner);
	void Run();
	~MacroRunner();
};

