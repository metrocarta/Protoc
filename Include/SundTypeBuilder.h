#pragma once
#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif
//#define PRINT_BUFFER_SIZE 200
#include <stdio.h>
#include <list>
#include "SundCompiler.h"
using namespace std;
class Scanner;
class ScannerState;
class Var;

/*
	declarator:
		pointer direct-declarator {suffix-declarator}
		direct-declarator:
			identifier
			'('declarator')'
		suffix-declarator:
			'['[constant-expression]']'
			'('[parameter-list]')'
		pointer:{*}
*/
class SundType;
//class SundCompiler;

struct sund_type_node{
	// pointer,array,function
	int type_code;
	int array_size;
	list<SundType *> * func_signature;
	sund_type_node(){ type_code = -1; array_size = -1; func_signature = 0; };
	sund_type_node(int t_code,int a_size){ type_code = t_code; array_size = a_size; func_signature = 0; };
};

class SundType {
private:
	list<sund_type_node *> sund_type_list;
	static list<sund_type_node *> * PushBack(list<sund_type_node *> * lst1, list<sund_type_node *> * lst2);
public:
	enum type_equal { NOT_EQUAL=0,EQUAL,CAST};
	//the basic type: char,byte,word,int,bit
	int basic_type;
	SundType();
	SundType(int basic_type);
	void Add(sund_type_node * type_node);
	void AddToFront(sund_type_node * type_node);
	static SundType * Merge(SundType * sund_type1, SundType * sund_type2);
	static wchar_t * GetBasicType(int type);
	static wchar_t * GetTypeCode(int type_code,int array_size);
	SundType * Copy(SundType * type);
	list<int> * GetArrayList();
	list<sund_type_node *> * GetSundTypeList(){return &sund_type_list;}
	int HasSubscript();
	void SetTopArraySize(int new_array_size);
	int IsFunc();
	int IsPtrToFunc();
	int IsEqual(SundType * type);
	int GetTopTypeCode();
	int GetBottomTypeCode();
	void Print(FILE * file);
	//void RemoveFunctionNode(){ sund_type_list.remove_if(IsFunctionNode); }
	friend class SundTypeBuilder;
	friend class SundInitializer;
	friend class SundData;
	friend class SundParser;
	friend class InstructionParser;
	friend class SundCompiler;
	friend class Var;
};
class SundTypeBuilder
{
private:
	int level,type;
	Var * var;
	SundType * PerformDecl(Scanner * scanner);
	SundType *  PerformSuffixDecl(Scanner * scanner);
	SundType * PerformFunction(Scanner * scanner);
	SundType * PerformArray(Scanner * scanner);
public:
	enum type_level {GLOBAL,LOCAL,PARAMS};
	SundType * sund_type;
	SundTypeBuilder(int level);
	// type: the basic C type
	SundTypeBuilder(int level,int type);
	void AssignVar(Var * v);
	SundType * Build(Scanner * scanner);
	SundType * GetSundType(){ return sund_type; }
	void Print(FILE * file);
	~SundTypeBuilder();
};

