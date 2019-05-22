// ProtoC.cpp : Defines the entry point for the console application.
//

//#include "stdafx.h"

/*
a) #include "StdAfx.h"

Project->Project Settings (Alt + F7)

Project-Settings-Dialog:
C++ -> Category: Precompiled Headers -> Precompiled Headers  --> disable
*/
//#define WINVER 0x0500
//#include <windows.h>

#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif
#include <tchar.h>
#include <stdio.h>
#include "Include/SundCompiler.h"
#include "Include/SundData.h"
#include "Include/SundParser.h"
#include "Include/ErrorPrinter.h"
#include "Include/ProgramControlBase.h"
#include "Include/ControlCounter.h"
#include "Include/ProgramArgumentsParser.h"

#include <iostream>
#include <locale>
#include <wchar.h>

//#include "./Intel8051/I8051ProgramControl.h"

#include "Device.h"
//#define PROTO_C_HELLO_MESSAGE L"Proto C Compiler version 1.0 Metrocarta Dragan Golubovic"

void PrintErrors();
int _tmain(int argc, _TCHAR* argv[])
{

	try
	{
		wchar_t * lst_fname = L"out.lst";
		FILE *lst_file = _wfopen(lst_fname, L"w");
		//FILE * lst_file =  stdout;

		//wchar_t platform[80];
		ProgramArgumentsParser * pap = new ProgramArgumentsParser(argc, argv);
		Scanner *scanner = new Scanner();
		SundData * sundData = new SundData();
		ErrorPrinter * errorPrinter = new ErrorPrinter(scanner);
		ControlCounter * controlCounter = new ControlCounter();
		ProgramControlBase * m_p, *glob_stat;

		//generic processor 
		m_p = new ProgramControlBase(ProgramControlBase::MAIN_PROGRAM);
		glob_stat = new ProgramControlBase();

		ProgramControlBase::global_instance = glob_stat;

		SundParser * sundParser = new SundParser(scanner);
		SundCompiler * sundCompiler = new SundCompiler(scanner, sundParser);
		sundCompiler->AssignMainProgram(m_p);
		sundCompiler->AssignGlobalStatement(glob_stat);
		sundCompiler->Run(pap->GetTopSrcList());
		//ScannerState * state = new ScannerState(scanner);
		
		// print all items
		glob_stat->PrintAll(lst_file, L"GLOBAL STATEMENTS:\n");
		m_p->PrintAll(lst_file, L"MAIN PROGRAM:\n");

		if (lst_file != stdout){
			fclose(lst_file);
		}
		int n_errors = (int)Err->error_list.size();
		if (!n_errors){
			wprintf(L"Finished successfully\n\n");
		}
		else{
			PrintErrors();
			wprintf(L"\n");
			wprintf(L"Finished with %d error(s)\n\n", n_errors);
		}

		delete sundCompiler;
		delete sundParser;
		delete m_p;
		delete controlCounter;
		delete errorPrinter;
		delete sundData;
		delete scanner;
	}
	catch (wchar_t * e)
	{
		PrintErrors();
		wprintf(L"%ls\n", e);
		wprintf(L"\n");
		wprintf(L"Finished with %d error(s)\n\n", 1 + (int)Err->error_list.size());
	}
	catch (PreprocessorMessage * e)
	{
		delete e->preprocessor;
		wprintf(L"%ls\n", e->message);
	}
	return 0;
	return 0;
}
void PrintErrors()
{
	for (list<struct error_info>::iterator it = Err->error_list.begin(); it != Err->error_list.end(); ++it)
	{
		wprintf(L"%ls", it->message_text);
	}
}

