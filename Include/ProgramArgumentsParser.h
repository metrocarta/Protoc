#pragma once
#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif

#include <list>
#include <tchar.h>
using namespace std;
class ProgramArgumentsParser
{
private:
	int argc;
	_TCHAR ** argv;
	wchar_t * platform;
	list<wchar_t *> src_list;
	static ProgramArgumentsParser * instance;
	// string "-spec" eg. -p filename.c
	int IsSpecifier(wchar_t * s);
	void Parse();
	void  PrintErrorInfoMessage();
	void  PrintHelpMessage();
	void CollectSources(int &index);
	wchar_t * GetPlatform(int &index);
public:
	ProgramArgumentsParser(int argc, _TCHAR* argv[]);
	//ProgramArgumentsParser(int argc, _TCHAR* argv[],wchar_t *  platform);
	~ProgramArgumentsParser();
	wchar_t * GetTopSrcList();
	wchar_t * GetPlatform(){ return platform; };
	static ProgramArgumentsParser * GetInstance();
};

