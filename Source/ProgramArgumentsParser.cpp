//#include <tchar.h>
#include "../Include/ProgramArgumentsParser.h"

ProgramArgumentsParser * ProgramArgumentsParser::instance = NULL;
ProgramArgumentsParser::ProgramArgumentsParser(int argc, _TCHAR* argv[])
{
	this->platform = 0;
	this->argc = argc;
	this->argv = argv;
	instance = this;
	this->Parse();

}
/*
ProgramArgumentsParser::ProgramArgumentsParser(int argc, _TCHAR* argv[], wchar_t *  platform)
{
	this->platform = 0;
	this->argc = argc;
	this->argv = argv;

	this->Parse();

	if (this->platform != 0){
		wcscpy(platform,this->platform);
	}
	else{
		wcscpy(platform, L"");
	}
}*/
ProgramArgumentsParser::~ProgramArgumentsParser()
{
}

int ProgramArgumentsParser::IsSpecifier(wchar_t * s)
{
	wchar_t f_char = s[0];
	if (f_char == '-'){
		return 1;
	}
	return 0;
}
void ProgramArgumentsParser::Parse()
{
	for (int i = 1; i < this->argc; i++)
	{
		wchar_t * s = this->argv[i];
		if (IsSpecifier(s))
		{
			if (!wcscmp(s+1,L"src"))
			{
				CollectSources(i);
			}
			else
			if (!wcscmp(s + 1, L"p"))
			{
				this->platform = GetPlatform(i);
			}
			else
			if (!wcscmp(s + 1, L"h"))
			{
				PrintHelpMessage();
				break;
			}
		}
		else{
			PrintErrorInfoMessage();
			break;
		}
	}
}
void  ProgramArgumentsParser::PrintErrorInfoMessage()
{
	wchar_t * msg = L"ProtoC compiler V1.0.0\n"
		            L"Copyright (c) 2014,2015 Metrocarta. All right reserved.\n"
					L"Try 'protoC -help' for more information.\n";

	throw msg;
}
void  ProgramArgumentsParser::PrintHelpMessage()
{
	wchar_t * msg = L"ProtoC compiler V1.0.0\n"
		L"Copyright (c) 2014,2015 Metrocarta. All right reserved.\n"
		L"Try 'protoC -help' for more information.\n";

	throw msg;
}
void ProgramArgumentsParser::CollectSources(int &index)
{
	int i = ++index;
	while (i < this->argc && !IsSpecifier(this->argv[i])){
		src_list.push_back(this->argv[i]);
		i = ++index;
	}
	--index;
}
wchar_t *  ProgramArgumentsParser::GetPlatform(int &index)
{
	int i = ++index;
	return this->argv[i];
}
wchar_t * ProgramArgumentsParser::GetTopSrcList()
{
	std::list<wchar_t *>::iterator it = this->src_list.begin();
	return *it;
}
ProgramArgumentsParser * ProgramArgumentsParser::GetInstance()
{
	return instance;
}