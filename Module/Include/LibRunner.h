#pragma once
class Scanner;

class LibRunner
{
	wchar_t device[200];
	Scanner * scanner;
public:
	LibRunner();
	LibRunner(Scanner * scanner);
	void Run();
	~LibRunner();
};

