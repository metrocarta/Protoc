#pragma once
#include <list>
using namespace std;

class ChainExpression;
class Var;
class SundParser;


class IntermediateNode
{
private:
	int op;
	Var * var_left,* var_right;
	ChainExpression * chain_left,*chain_right; 
	//IntermediateNode * constant_node;
	list<IntermediateNode *> constant_list;
	// num. operands
	int n_operands;
public:
	int is_valid;
	SundType * result_type;
	IntermediateNode(void);
	IntermediateNode(int op);
	IntermediateNode(int op,Var * var_left,Var * var_right);
	IntermediateNode(int op,Var * var_left,Var * var_right,int n_operands);
	~IntermediateNode(void);
	// provera tipova levog i desnog operanda
	//void CheckTypes();
	int CheckTypes();
	void BuildResultType();
	friend class SundParser;
	friend class ModuleParser;
	friend class InstructionParser;
	friend class ChainExpression;
	friend class SundCompiler;
};
