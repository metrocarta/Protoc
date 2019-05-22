#pragma once
#include <list>

using namespace std;
class ChainExpression;
class SundType;
class SundTypeBuilder;
struct sund_string;

class ProgramControlBase
{
private:
	// WHILE,DO,FOR,SWITCH,IF or something else
	int control_type;
	ChainExpression * expr;
	wchar_t name[80];
	ChainExpression * start_stmt, * end_stmt;
	//SundType * sund_type;
	// initializer
	int init_list_size;
	ChainExpression ** init_list;
	SundTypeBuilder * builder;
	// string char *s = "string";
	sund_string * string;
protected:
	// unique id
	int id;
	list<ProgramControlBase *> sequence;
public:
	enum control_types {BLOCK=0,MAIN_PROGRAM,EXPR,FUNC,IF,ELSE,WHILE,DO,FOR,SWITCH,CASE,DEFAULT,
						GOTO,LABEL,RETURN,BREAK,CONTINUE,DECL,INITIALIZER,STRING,MODULE};
	ProgramControlBase * parent;
	static ProgramControlBase * global_instance;
	ProgramControlBase(void);
	ProgramControlBase(int control_type);
	ProgramControlBase(int control_type,wchar_t * name);
	ProgramControlBase(int control_type,ChainExpression * expr);
	ProgramControlBase(int control_type,ChainExpression * expr,wchar_t * name);
	ProgramControlBase(int control_type, SundTypeBuilder * builder);
	ProgramControlBase(int control_type, SundTypeBuilder * builder,int init_list_size, ChainExpression ** init_list);
	void Add(ProgramControlBase * control);
	void SetStartStmt(ChainExpression * start_stmt);
	void SetEndStmt(ChainExpression * end_stmt);
	int  GetControlType(){ return control_type;}
	static int GenerateId(int control_type);
	void SetId(int id);
	int  GetId(){return id;}
	void SetName(wchar_t * name);
	wchar_t const * GetControlTypeName();
	void SetExpr(ChainExpression * expr);
	void SetString(wchar_t * value, int length);
	virtual void Print(FILE * file);
	void PrintAll(FILE * file);
	void PrintAll(FILE * file,wchar_t * title);
	~ProgramControlBase(void);
};
