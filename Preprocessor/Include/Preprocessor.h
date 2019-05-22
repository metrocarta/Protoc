#pragma once

#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif

#include <stdio.h>
#include <list>
#include <string>

using namespace std;

struct define_table{
	//int is_macro;//0 no,1 yes
	wchar_t name[80];
	wchar_t value[200];
	list<wstring> macro_args;
};
class Scanner;
// pomocna struktura za skladistenje privremenih buffera
struct PreprocessorHeap
{
	wchar_t * buffer, *b,* token,* prefix;
};
class Preprocessor
{
private:
	wchar_t * start_ptr;
	wchar_t * file_inp_name, * file_outp_name;
	list<struct define_table> define_list;
	list<struct PreprocessorHeap> tmp_buffer_list;
	//struct PreprocessorHeap * curr_heap;
	FILE  * outp_file;
	struct define_table * AddNewDefineEntry(Scanner * scanner, wchar_t * name);
	struct define_table * AddNewDefineEntry(Scanner * scanner, wchar_t * name, wchar_t * value);
	void HandleDirective(Scanner * scanner, wchar_t * buffer, FILE * outp_file);
	void HandleDefine(Scanner * scanner, wchar_t * buffer, FILE * outp_file);
	void BuildDefineValue(Scanner * scanner, wchar_t * buffer);
	//collects arguments
	void HandleMacro(Scanner * scanner, wchar_t * macro_name, FILE * outp_file);
	// evaluates macro
	void EvalMacro(Scanner * scanner, wchar_t * macro_name, wchar_t * macro_value, FILE * outp_file, list<wstring> macro_parameters);
	void AssignParameter(Scanner * scanner, wchar_t * buff, wstring arg, wstring parameter);
	list<wstring> * GetMacroArguments(wstring macro_name);
	void FillBlankLine(Scanner * scanner, FILE * outp_file, wchar_t * buffer);
	void HandleInclude(Scanner * scanner);
	int DoesMacroReuseArgument(list<wstring> *macro_args, wstring arg);

	// fill buffer with space char. (' ')
	static void ClearBuffer(wchar_t *buff);
	void PrintErrorMessage(Scanner * scanner, int err);
	void PrintErrorMessage(Scanner * scanner, int err,wchar_t *s);
	// private constructor
	Preprocessor(void);
	void Dispose();
public: 
	Preprocessor(wchar_t  * file_inp_name, wchar_t * file_outp_name, FILE  * outp_file);
	~Preprocessor(void);
	int Execute(wchar_t const * file_inp_name, wchar_t const * file_outp_name);
	int IsExistDefineName(wchar_t * name);
	void  GetDefineValue(wchar_t * name, wchar_t * buff_value);
	//Byte Order Marker BOM
	static int HasBOM(wchar_t buff[]);
};

//
class PreprocessorMessage
{
public:
	wchar_t  message[200];
	Preprocessor * preprocessor;
	PreprocessorMessage(wchar_t *msg, Preprocessor * ppc);
};
