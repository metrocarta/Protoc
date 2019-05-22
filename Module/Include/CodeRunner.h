#pragma once
class Scanner;
class ModuleParser;

class CodeRunner
{
private:
protected:
	Scanner * scanner;
	wchar_t device[200];
public:
	CodeRunner();
	CodeRunner(Scanner * scanner);
	void Run();
	~CodeRunner();
};

