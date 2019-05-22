#include "../Include/Preprocessor.h"
#include "../Scanner.h"
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h> 
#include "../ErrorPrinter.h"
#include "../SundCompiler.h"

Preprocessor::Preprocessor(void)
{
	define_list.clear();
	tmp_buffer_list.clear();
}
Preprocessor::Preprocessor(wchar_t  * file_inp_name, wchar_t  *file_outp_name, FILE  * outp_file)
{
	define_list.clear();
	tmp_buffer_list.clear();
	
	this->file_inp_name = file_inp_name;
	this->outp_file = outp_file;
	this->file_outp_name = file_outp_name;
}
Preprocessor::~Preprocessor(void)
{
	Dispose();
}
void Preprocessor::Dispose()
{
	for (list<struct PreprocessorHeap>::iterator it = tmp_buffer_list.begin(); it != tmp_buffer_list.end(); ++it)
	{
		delete it->buffer;
		delete it->b;
		delete it->prefix;
		delete it->token;
	}
}
int Preprocessor::Execute(wchar_t  const * file_inp_name, wchar_t  const * file_outp_name)
{
	// new scanner instance
	Scanner * scanner = new Scanner(file_inp_name);
	//outp_file = _wfopen(file_outp_name, L"w");
	FILE * inp_file = _wfopen(file_inp_name, L"r");
	if (inp_file == NULL)
		return 0;
	int token_type;
	// obtain file size:
	fseek(inp_file, 0, SEEK_END);
	int f_size = ftell(inp_file);

	struct PreprocessorHeap  tmp_object;
	// allocate memory to contain the whole file:
	tmp_object.buffer = new wchar_t[f_size];
	tmp_object.b = new wchar_t[f_size];
	tmp_object.prefix = new wchar_t[f_size];
	tmp_object.token = new wchar_t[f_size];
	tmp_buffer_list.push_back(tmp_object);
	rewind(inp_file);
	
	wchar_t wc;
	int i = 0;
	do {
		wc = fgetwc(inp_file);
		tmp_object.buffer[i] = wc;
		++i;
	} while (wc != WEOF);
	tmp_object.buffer[i-1] = '\0';
	// close inp_file
	fclose(inp_file);
	
	//is there a forbidden token
	int line_count = Scanner::SearchForbiddenToken(tmp_object.buffer,L"#line");
	if (line_count != -1){
		Err->CPPFatal(line_count, ErrorPrinter::SYNTAX, file_inp_name,L"#line");
		return 0;
	}
	int bom_offset = 0;
	if (Preprocessor::HasBOM(tmp_object.buffer))
	{
		// the input file consists BOM signature (0xEF 0xBB 0xBF)
		bom_offset = 3;
	}
	// puts line index and filename
	fprintf(outp_file,"//#line %d \"%ls\" \n",1+scanner->CountEndLine(),scanner->GetFileName());

	scanner->prog = tmp_object.buffer + bom_offset;
	scanner->p_buf = tmp_object.buffer + bom_offset;
	do{
		wchar_t dv[200];
		wcscpy(dv, L"");
		wchar_t * p1 = scanner->prog;
		this->start_ptr = p1;
		token_type = scanner->GetToken();
		//tests is define value 
		GetDefineValue(scanner->token, dv);
	
		if (token_type == SundCompiler::C_PPC_DIR)
		{
			// handles directive: #directive...#define,#include...
			HandleDirective(scanner, tmp_object.b, outp_file);
			continue;
		}
		else
		{
			scanner->GetTokenPrefix(tmp_object.prefix, p1, scanner->prog);
			if (wcslen(dv) != 0)
			{
				//token is #define value or macro
				wcscpy(tmp_object.token, scanner->token);
				scanner->GetToken();
				if (*(scanner->token) != '(')
				{
					//not macro
					scanner->Putback();
				    // substitutes define value
				    fprintf(outp_file, "%ls", dv);
				}
				else
				{
					// this is macro, evaluates macro parameters
					list<wstring> macro_parameters;
					wchar_t parameter[80];
					macro_parameters.clear();
					do{
						scanner->GetToken();
						if (*(scanner->token) != ')')
						{
							wcscpy(parameter, scanner->token);
							wstring ws(parameter);
							macro_parameters.push_back(ws);
							// get (,)
							scanner->GetToken();
						}
						else break;
					} while (*(scanner->token) == ',');
					if (*(scanner->token) != ')')
					{
						//invalid invoke macro, print error
						PrintErrorMessage(scanner,Err->CPP_SYNTAX);
						return 0;
					}
					fprintf(outp_file, "%ls", tmp_object.prefix);
					EvalMacro(scanner, tmp_object.token, dv, outp_file, macro_parameters);
				}
			}
			else
			{
				// normal token: not #define value
				Scanner::GetCodeDifference(p1, scanner->prog, tmp_object.b);
				fprintf(outp_file, "%ls", tmp_object.b);
			}
		}
	} while (scanner->tok != SundCompiler::FINISHED);

	//list<struct define_table> * ppp = &define_list;
	delete scanner;
	return 1;
}
int Preprocessor::IsExistDefineName(wchar_t * name)
{	
	for (list<struct define_table>::iterator it = define_list.begin(); it != define_list.end();++it)
	{
		if (!wcscmp(it->name, name)){
			return 1;
		}
	}
	return 0;
}
void Preprocessor::GetDefineValue(wchar_t * name, wchar_t * buff_value)
{
	for (list<struct define_table>::iterator it = define_list.begin(); it != define_list.end(); ++it)
	{
		if (!wcscmp(it->name, name)){
			wcscpy(buff_value, it->value);
			return;
		}
	}
	return ;
}
struct define_table * Preprocessor::AddNewDefineEntry(Scanner * scanner, wchar_t * name)
{
	struct define_table * define_node = new struct define_table();
	wchar_t dv[80];
	wcscpy(dv, L"");
	GetDefineValue(name, dv);
	if (wcslen(dv) != 0){
		// already defined ...print error
		PrintErrorMessage(scanner,Err->CPP_REDEFINITION_OF_DEFINE);
		return 0;
	}
	if (!IsExistDefineName(name))
	{
		wcscpy(define_node->name, name);
		wcscpy(define_node->value, L"");
		define_list.push_back(*define_node);
		return define_node;
	}
	else{
		// print an error message
		return 0;
	}
}

struct define_table * Preprocessor::AddNewDefineEntry(Scanner * scanner, wchar_t * name, wchar_t * value)
{
	struct define_table * define_node = new struct define_table();
	wchar_t dv[80];
	wcscpy(dv, L"");
	GetDefineValue(name, dv);
	if (wcslen(dv) != 0){
		// already defined ...print error
		PrintErrorMessage(scanner,Err->CPP_REDEFINITION_OF_DEFINE);
		return 0;
	}
	if (!IsExistDefineName(name))
	{
		wcscpy(define_node->name, name);
		wcscpy(define_node->value, value);
		define_list.push_back(*define_node);
		return define_node;
	}
	else{
		// print an error message
		return 0;
	}
}
void Preprocessor::HandleDefine(Scanner * scanner,wchar_t * buffer, FILE * outp_file)
{
	wchar_t * ptr = 0;
	wchar_t name[80], value[200];
	wcscpy(value, L"");
	
	// get name
	scanner->GetToken();
	wcscpy(name, scanner->token);

	//is it macro?
	scanner->GetToken();
	if (! wcscmp(scanner->token, L"(")){
		// load data into define table
		struct define_table * node = AddNewDefineEntry(scanner,name);
		//handle macro's arguments: collects arguments
		HandleMacro(scanner,name,outp_file);
		FillBlankLine(scanner,outp_file, buffer);

		return;
	}
	else scanner->Putback();

	BuildDefineValue(scanner,value);

	// load data into define table
	AddNewDefineEntry(scanner,name, value);
	FillBlankLine(scanner,outp_file, buffer);
}
void  Preprocessor::FillBlankLine(Scanner * scanner, FILE * outp_file, wchar_t * buffer)
{
	wchar_t * ptr = scanner->prog;
	// fill buffer with diff. content (this->start_ptr,ptr)
	Scanner::GetCodeDifference(this->start_ptr, ptr, buffer);
	Preprocessor::ClearBuffer(buffer);
	fprintf(outp_file, "%ls", buffer);
}
void Preprocessor::BuildDefineValue(Scanner * scanner, wchar_t * buffer)
{
	wchar_t  dv[80];
	wchar_t * ptr = 0;
	wcscpy(buffer, L"");
	do{
		wcscpy(dv, L"");
		// get define value
		scanner->GetToken();
		ptr = scanner->prog;
		GetDefineValue(scanner->token, dv);
		if (wcslen(dv) != 0){
			wcscat(buffer, dv);
		}
		else{
			wcscat(buffer, scanner->token);
		}

	} while (*ptr != '\n' && *ptr !='\0');
	// ako nema znaka za novi red a postoji znak za kraj fajla 
	if (*ptr == '\0'){
		fprintf(outp_file,"\n");
	}
}

//collects arguments
void Preprocessor::HandleMacro(Scanner * scanner, wchar_t * macro_name, FILE * outp_file)
{
	wchar_t arg[80], value[200];
	struct define_table  *node = (struct define_table *)&define_list.back();
	do{
		scanner->GetToken();
		if (*(scanner->token) != ')')
		{
			wcscpy(arg,scanner->token);
			wstring ws(arg);
			if (!DoesMacroReuseArgument(&(node->macro_args), ws)){
				// print error
				PrintErrorMessage(scanner,Err->CPP_REUSE_ARG);
				return;
			}
			node->macro_args.push_back(ws);
			// get (,)
			scanner->GetToken();
		}
		else break;
	} while (*(scanner->token) == ',');
	if (*(scanner->token) != ')'){
		// print error
		PrintErrorMessage(scanner,Err->CPP_SYNTAX);
		return;
	}
	BuildDefineValue(scanner,value);
	wcscpy(node->value, value);
}
void Preprocessor::EvalMacro(Scanner * scanner, wchar_t * macro_name, wchar_t * macro_value, FILE * outp_file, list<wstring> macro_parameters)
{
	wchar_t buff[200];
	list<wstring> * macro_args = this->GetMacroArguments(macro_name);
	/*if (macro_args == 0){
		// print error
	}*/
	if (macro_args->size() != macro_parameters.size()){
		// print error
		PrintErrorMessage(scanner,Err->CPP_WRONG_NUM_PARAMETRS);
		return;
	}
	wcscpy(buff, macro_value);
	list<wstring>::iterator it1 = macro_args->begin();
	for (list<wstring>::iterator it2 = macro_parameters.begin(); it2 != macro_parameters.end(); ++it2)
	{
		AssignParameter(scanner,buff, *it1, *it2);
		++it1;
	}
	
	fprintf(outp_file, "%ls", buff);
}
int Preprocessor::DoesMacroReuseArgument(list<wstring> *macro_args,wstring arg)
{
	for (list<wstring>::iterator it = macro_args->begin(); it != macro_args->end(); ++it)
	{
		if (*it == arg)
			return 0;
	}
	return 1;
}
// 
void Preprocessor::AssignParameter(Scanner * scanner, wchar_t * buff, wstring arg, wstring parameter)
{
	Scanner * tmp_scanner = new Scanner();
	wchar_t tmp[200];
	wcscpy(tmp, L"");
	tmp_scanner->prog = buff;
	tmp_scanner->p_buf = buff;
	//
	do{
		tmp_scanner->GetToken();
		if (!wcscmp(tmp_scanner->token, arg.c_str())){
			wcscat(tmp, parameter.c_str());
		}
		else{
			wcscat(tmp, tmp_scanner->token);
		}
	} while (tmp_scanner->tok != SundCompiler::FINISHED);

	wcscpy(buff,tmp);
	delete tmp_scanner;
}
list<wstring> * Preprocessor::GetMacroArguments(wstring macro_name)
{
	for (list<struct define_table>::iterator it = define_list.begin(); it != define_list.end(); ++it)
	{
		if (!wcscmp(it->name, macro_name.c_str()))
		{
			/*if (it->is_macro == 0)
				return 0;
			else return &(it->macro_args);*/
			return &(it->macro_args);
		}
	}
	return 0;
}
void Preprocessor::HandleInclude(Scanner * scanner)
{
	wchar_t  buff[200];
	int result;
	// get the header fname
	scanner->GetToken();
	if (scanner->token_type == SundCompiler::STRING){
		result = Execute(scanner->token, this->file_outp_name);
		if (result == 0)
		{
			// print error,  no file
			PrintErrorMessage(scanner, Err->CPP_CANNOT_OPEN_FILE,scanner->token);
			return;
		}
		// puts line index and filename
		fprintf(outp_file, "//#line %d \"%ls\" \n", 1+scanner->CountEndLine(), scanner->GetFileName());
		return;
	}
	if (scanner->token[0] == LT)
	{
		// this is "<", get the header fname
		scanner->GetToken();
		wcscpy(buff, scanner->token);
		scanner->GetToken();
		if (scanner->token[0] != GT)
		{
			// print error
			PrintErrorMessage(scanner, Err->CPP_INCLUDE_SYNTAX);
			return;
		}
		result = Execute(buff, this->file_outp_name);
		if (result == 0)
		{
			// print error,  no file
			PrintErrorMessage(scanner, Err->CPP_CANNOT_OPEN_FILE, buff);
			return;
		}
		// puts line index and filename
		fprintf(outp_file, "//#line %d \"%ls\" \n", 1+scanner->CountEndLine(), scanner->GetFileName());
	}
	else
	{
		// print error
		PrintErrorMessage(scanner, Err->CPP_INCLUDE_SYNTAX);
		return;
	}
}
void Preprocessor::PrintErrorMessage(Scanner * scanner, int err)
{
	// close outp_file
	fclose(outp_file);

	Err->CPPPrint(scanner,this,err);
}
void Preprocessor::PrintErrorMessage(Scanner * scanner, int err, wchar_t *s)
{
	// close outp_file
	fclose(outp_file);

	Err->CPPPrint(scanner,this,err, s);
}

void Preprocessor::HandleDirective(Scanner * scanner, wchar_t * buffer, FILE * outp_file)
{
	scanner->GetToken();
	if (!wcscmp(scanner->token, L"define")){
		HandleDefine(scanner,buffer, outp_file);
	}
	else
	if (!wcscmp(scanner->token, L"include")){
		HandleInclude(scanner);
	}
	else{
		// print error
		PrintErrorMessage(scanner,Err->CPP_INVALID_COMMAND);
		return;
	}
}
void Preprocessor::ClearBuffer(wchar_t *buff)
{
	//int length = sizeof(buff) / sizeof(buff[0]);
	for (wchar_t * p = buff; *p != '\0'; p++){
		if (*p!='\n')
		    *p = ' ';
	}
}
int Preprocessor::HasBOM(wchar_t  buff[])
{
	if (buff[0] == 0xEF && buff[1] == 0xBB && buff[2] == 0xBF)
		return 1;

	return 0;
}



PreprocessorMessage::PreprocessorMessage(wchar_t *msg, Preprocessor * ppc)
{ 
	wcscpy(message, msg); 
	preprocessor = ppc; 
};