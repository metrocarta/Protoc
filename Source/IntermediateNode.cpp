#include "../Include/Var.h"
#include "../Include/IntermediateNode.h"
#include "../Include/ChainExpression.h"
#include "../Include/SundTypeBuilder.h"

IntermediateNode::IntermediateNode(void)
{
	op = 0;
	var_left = var_right = 0;
	chain_left = chain_right = 0;
	n_operands = 2;
	//constant_node = 0;
	constant_list.clear();
	is_valid = 1;
	result_type = 0;
}
IntermediateNode::IntermediateNode(int op)
{
	this->op = op;
	var_left = var_right = 0;
	chain_left = chain_right = 0;
	n_operands = 2;
	//constant_node = 0;
	constant_list.clear();
	is_valid = 1;
	result_type = 0;
}
IntermediateNode::IntermediateNode(int op,Var * var_left,Var * var_right)
{
	this->op = op;
	//this->var_left =  new Var();
	//this->var_right = new Var();
	this->var_left = 0;
	this->var_right = 0;
	if(var_left!=0)
	{
		this->var_left =  new Var();
		this->var_left->Copy(var_left);
	}
	if(var_right!=0)
	{
		this->var_right = new Var();
		this->var_right->Copy(var_right);
	} 
	chain_left = chain_right = 0;
	n_operands = 2;
	//constant_node = 0;
	constant_list.clear();
	is_valid = 1;
	result_type = 0;
}
IntermediateNode::IntermediateNode(int op,Var * var_left,Var * var_right,int n_operands)
{
	this->op = op;
	this->var_left = 0;
	this->var_right = 0;
	//this->var_left =  new Var();
	//this->var_right = new Var();
	if(var_left!=0)
	{
		this->var_left =  new Var();
		this->var_left->Copy(var_left);
	}
	if(var_right!=0)
	{
		this->var_right = new Var();
		this->var_right->Copy(var_right);
	} 
	chain_left = chain_right = 0;
	this->n_operands = n_operands;
	constant_list.clear();
	is_valid = 1;
	result_type = 0;
}
IntermediateNode::~IntermediateNode(void)
{
	if (var_left != 0)
		delete var_left;
	if (var_right != 0)
		delete var_right;
}
// provera tipova levog i desnog operanda
//(: 0 isti tip,-1 ako je right_type = -1 (konstanta) a +1 ako su razliciti tipovi)
/*void IntermediateNode::CheckTypes()
{
	int type_left = 0, type_right = 0;
	int specifier_left = 0, specifier_right = 0;
	if (this->var_left != 0){
		type_left = this->var_left->basic_type;
		specifier_left = this->var_left->specifier;
	}
	if (this->chain_left != 0){
		type_left = this->chain_left->GetResult()->basic_type;
		specifier_left = this->chain_left->GetResult()->specifier;
	}
	if (this->var_right != 0){
		type_right = this->var_right->basic_type;
		specifier_right = this->var_right->specifier;
	}
	if (this->chain_right != 0){
		type_right = this->chain_right->GetResult()->basic_type;
		specifier_right = this->chain_right->GetResult()->specifier;
	}
}*/
int IntermediateNode::CheckTypes()
{
	SundType * sund_type1;
	SundType * sund_type2;
	if (this->chain_left != 0 && this->chain_right != 0)
	{
		//this->var_left
		if (this->chain_right->result == 0){
			list<IntermediateNode>::iterator iter = this->chain_right->GetChain()->begin();
			//sund_type1 = this->var_left->GetSundType();
			sund_type1 = this->chain_left->GetResult()->GetSundType();
			sund_type2 = iter->result_type;
			if (sund_type2 == 0)
				return 1;
			if (!sund_type1->IsEqual(sund_type2))
				return 0;

			return 1;
		}
		//	sund_type2 = node->chain_right->
		if (this->chain_right->result != 0 && this->chain_right->result->IsConst())
			return 1;

		//sund_type1 = this->var_left->GetSundType();
		sund_type1 = this->chain_left->GetResult()->GetSundType();
		sund_type2 = this->chain_right->result->GetSundType();
		if (!sund_type1->IsEqual(sund_type2))
			return 0;
	}
	return 1;
}
// define SundType (result_type)
void IntermediateNode::BuildResultType()
{
	if (this->var_left != 0){
		if (this->var_left->sund_type != 0){
			this->result_type = new SundType();
			this->result_type->Copy(this->var_left->sund_type);
			return;
		}
	}
	if (this->chain_left != 0){
		if (this->chain_left->result != 0 && this->chain_left->result->sund_type != 0){
			this->result_type = new SundType();
			this->result_type->Copy(this->chain_left->result->sund_type);
			return;
		}
	}
	if (this->var_right != 0){
		if (this->var_right->sund_type != 0){
			this->result_type = new SundType();
			this->result_type->Copy(this->var_right->sund_type);
			return;
		}
	}
	if (this->chain_right != 0){
		if (this->chain_right->result != 0 && this->chain_right->result->sund_type != 0){
			this->result_type = new SundType();
			this->result_type->Copy(this->chain_right->result->sund_type);
			return;
		}
	}
}

