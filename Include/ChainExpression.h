#pragma once

#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif

#include <list>
#include "../Include/Var.h"
#include "../Include/IntermediateNode.h"
#define PRINT_BUFFER_SIZE 200

using namespace std;

//class IntermediateNode;
class SundType;

class ChainExpression
{
private:
	// expr. string 
	wchar_t s_expression[200];
	list<IntermediateNode> chain;
	static wchar_t  print_buffer[PRINT_BUFFER_SIZE];
	// print out a chain
	void Out(ChainExpression * chain);
	wchar_t * GetPrintBuffer();
	static void ClearPrintBuffer();
	wchar_t const * GetOperatorName(int op, ChainExpression * chain);
	void EliminateConstants();
	void ReduceConstantsAddSub();
    int GetZeroOperand(int *zero_operand);
public:
	int kind,is_lvalue;

	enum operators{ASGN=1,AND,OR,XOR,ADD,SUB,MULT,DIV,MODULUS,INC,DEC,CONST,LT,LE,
				   GT,GE,EQ,NE,AND_AND,OR_OR,CALL,ARG,ARRAY,ARRAY_INIT,INDEX_OF,GOTO,LABEL,NEG,
				   NOT,BITWISE_NOT,INDIR,ADDRESS,SIZEOF,COMP_ADD,COMP_SUB,COMP_MULT,COMP_DIV,
				   COMP_AND,COMP_OR,BITSHIFT_LEFT,BITSHIFT_RIGHT,CAST,CONVERT_FROM};

	enum kind_operations { ADD_SUB = 0, MULT_DIV, LOGICAL_OR, LOGICAL_AND,BITWISE_IOR,BITWISE_AND,BITWISE_XOR,REL_OP,BITSHIFT };
	Var * result;
	// links to separate expression (expr,expr...,expr)
	ChainExpression * comma_chain,* postfix_chain;
	ChainExpression(void);
	ChainExpression(IntermediateNode * node);
	void AddNode(IntermediateNode * node);
	int CollectConstantVar(IntermediateNode * node);
	int ChangeConstantVar(IntermediateNode * node);
	int IsEmpty();
	int IsCompleted();
	Var * GetResult(){return result;}
	// print the complete chain
	void Print(FILE * file);
	static ChainExpression * MakeLabelChain(wchar_t const * label_name,int id);
	static ChainExpression * MakeChain(int value);
	int IsConst(int * value);
	// expr = const1 op const2 op ... const_n
	int IsConstExpr(Var * var);
	int IsZeroConstant();
	// is lvalue
	//int IsLValue();
	list<IntermediateNode> * GetChain(){ return &chain; };
	static int Calculate(int value1, int value2, int op);
	static int Calculate(int first_operand, list<IntermediateNode *> &constant_list);
	ChainExpression * ReduceConstants();
	//void GetExpressionTypeInfo(int * type, int * is_pointer);
	SundType * GetSundType();
	//void SetIsLValue(int v){ is_lvalue = v; };
	//int  GetIsLValue(){ return is_lvalue; };
	~ChainExpression(void);
};
