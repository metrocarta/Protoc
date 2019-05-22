#pragma once
#include "CodeRunner.h"
class Scanner;

class BlockRunner : public CodeRunner
{
private:
public:
	BlockRunner();
	BlockRunner(Scanner * scanner);
	void Run();
	~BlockRunner();
};

