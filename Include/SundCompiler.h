#pragma once

#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h> 
#include <stack>
#include <list>

#include "Scanner.h"
#include "../Preprocessor/Include/Preprocessor.h"
#include "../Include/Var.h"
#include "ProgramControlBase.h"

using namespace std;

#define Compiler SundCompiler::GetInstance()
#define DO_SEPARATE_EXPR 1
#define ID_LEN 80


class Scanner;
class SundParser;
class ChainExpression;
class ProgramControlBase;

struct commands {
	wchar_t command[20];
	wchar_t tok;
};

struct label_type {
	wchar_t label_name[ID_LEN];
	wchar_t func_name[ID_LEN];
	wchar_t * start;
};

enum double_ops {LT=1,LE,GT,GE,EQ,NE};

class SundCompiler
{
private:
	int stmt_id;
	struct commands *table;
	void DeclLabel(wchar_t * name, wchar_t * func_name);
	static SundCompiler * instance;
	Scanner * scanner;
	SundParser * sundParser;
	Preprocessor * preprocessor;
	ProgramControlBase * global_statement,* main_program;
	int constant_folding_enabled, propagation_enabled;
	void Init();
	void DeclGlobal(int type, int qualifier, int storage_class, int type_qualifier);
	// push func. parameters to stack (vars)
	void PushParams(wchar_t * func_name);
	void DeclLocal(ProgramControlBase *program_control, int storage_class, int type_qualifier, wchar_t * func_name);
	ChainExpression * DoStatement(ProgramControlBase *program_control, int brace, wchar_t * func_name = 0);
	ProgramControlBase * PerformWhile(ProgramControlBase *program_control);
	ProgramControlBase * PerformDo(ProgramControlBase *program_control);
	ProgramControlBase * PerformIf(ProgramControlBase *program_control);
	ProgramControlBase * PerformFor(ProgramControlBase *program_control);
	ProgramControlBase * PerformSwitch(ProgramControlBase *program_control);
	ProgramControlBase * PerformCase(ProgramControlBase *program_control);
	ProgramControlBase * PerformDefault(ProgramControlBase *program_control);
	ProgramControlBase * PerformGoto(ProgramControlBase *program_control);
	ProgramControlBase * PerformBreak(ProgramControlBase *program_control);
	ProgramControlBase * PerformContinue(ProgramControlBase *program_control);
	ProgramControlBase * PerformReturn(ProgramControlBase *program_control);
	
	void Compile(void);
	void Tokenize();
	int IsQualifier(int qualifier);
	void WriteBOM(FILE * file);
	int IsClassOrStructOrUnion(wchar_t *t);
	void BypassFuncArgs();
	int IsFuncDef();
	void DeclFunction(int type, int qualifier, int storage_class, int type_qualifier);
	enum false_true { FALSE = 0, TRUE = 1 };
public:
	enum tok_types {
		DELIMITER, ILLEGAL_CHAR, IDENTIFIER, NUMBER, HEX_NUMBER, B_NUMBER, KEYWORD, TEMP, STRING, BLOCK,
		C_PPC_DIR, MODULE_DIR, LABEL_DELIMITER, INCREMENT, DECREMENT, COMP_ADD, COMP_SUB, COMP_MULT,
		COMP_DIV, COMP_AND, COMP_OR, AND_AND, OR_OR, NOT, BITWISE_NOT, CHARACTER,
		BITSHIFT_LEFT, BITSHIFT_RIGHT, CAST, SIZEOF, NEGATION
	};
	enum tokens {
		CHAR = 1, INT, BYTE, WORD, BIT, POINTER,POINTER_TO_FUNCTION,ARRAY,FUNCTION, IF, ELSE, FOR, DO, WHILE, SWITCH, RETURN, EOL,
		GOTO, BREAK, CONTINUE, FINISHED, VOID, CASE, DEFAULT, TRY, THROW, CATCH,
		ORG, DB, DW, DT, LABEL, INLINE, STATIC, AUTO, REGISTER, CONST, VOLATILE, MODULE, EXTENDS,DEVICE,PROPERTY, 
		VAR, CODE, ENDC, MACRO, ENDM, CODE_BLOCK, ENDB,	IMPORT,LIB,ENDLIB, END
	};
	SundCompiler(void);
	SundCompiler(Scanner * scanner,SundParser * sundParser);
	static SundCompiler * GetInstance();
	wchar_t * GetCode();
	wchar_t * GetProgramBuffer();
	struct commands * GetTable(){return table;};
	Scanner * GetScanner(){return scanner;}
	static int IsStorageClass(int type);
	static int IsRegularType(int type);
	static int IsTypeQualifier(int qualifier);
	void Run(wchar_t  *fname);
	void AssignMainProgram(ProgramControlBase * main_program);
	void AssignGlobalStatement(ProgramControlBase * global_statement);
	void EnableConstantFolding(int en){ constant_folding_enabled = en; }
	int GetConstantFoldingEnabled() { return constant_folding_enabled; }
	void EnablePropagation(int en){ propagation_enabled = en; }
	int GetPropagationEnabled(){ return propagation_enabled; }
	~SundCompiler(void);
};

