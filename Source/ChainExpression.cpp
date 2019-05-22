#include "../Include/ChainExpression.h"
#include "../Include/IntermediateNode.h"
#include "../Include/SundCompiler.h"
#include "../Include/SundTypeBuilder.h"
#include <stdio.h>

wchar_t  ChainExpression::print_buffer[];

ChainExpression::ChainExpression(void)
{
	this->is_lvalue = 0;
	this->result = 0;
	wcscpy(this->s_expression, L"");
	wcscpy(print_buffer, L"");
	this->chain.clear();
	this->comma_chain = 0;
	this->postfix_chain = 0;
}
ChainExpression::ChainExpression(IntermediateNode * node)
{
	this->is_lvalue = 0;
	this->result = 0;
	wcscpy(this->s_expression, L"");
	wcscpy(print_buffer, L"");
	this->chain.clear();
	this->comma_chain = 0;
	this->postfix_chain = 0;
	// add a node
	this->AddNode(node);
}
wchar_t * ChainExpression::GetPrintBuffer()
{
	return print_buffer;
}

void ChainExpression::AddNode(IntermediateNode * node)
{
	this->chain.push_back(*node);
}
int ChainExpression::CollectConstantVar(IntermediateNode * node)
{
	int tmp;
	list<IntermediateNode>::iterator it;
	for (it = chain.begin(); it != chain.end(); it++)
	{
		IntermediateNode * c_node = &(*it);
		if ((chain.size() == 0 && c_node->chain_left->IsConst(&tmp)) || c_node->chain_right->IsConst(&tmp))
		{
			// constant node found, then update constant_node!
			c_node->constant_list.push_back(node);
			return 1;
		}
	}
	//if (chain.size() == 0)
		//return 0;

	return 0;
}
int ChainExpression::ChangeConstantVar(IntermediateNode * node)
{
	int i = 0, tmp;
	int size = (int)chain.size();
	list<IntermediateNode>::iterator it;
	if (size == 0) 
		return 0;

	it = chain.end(); it--;
	if (size == 1){
		if (it->chain_left->IsConst(&tmp) || it->chain_right->IsConst(&tmp)){
			if (node->chain_right->IsConst(&tmp)){
				it->constant_list.push_back(node);
				return 1;
			}
			else return 0;
		}
		else return 0;
	}
	// size>1
	if (it->chain_right->IsConst(&tmp) && node->chain_right->IsConst(&tmp)){
		it->constant_list.push_back(node);
		return 1;
	}

	return 0;
}

wchar_t const * ChainExpression::GetOperatorName(int op, ChainExpression * chain)
{
	static wchar_t s[80];
	switch(op){
		case ASGN:
			return L"asgn";
		case AND:
			return L"and";
		case OR:
			return L"or";
		case XOR:
			return L"xor";
		case ADD:
			return L"add";
		case SUB:
			return L"sub";
		case INC:
			return L"inc";
		case DEC:
			return L"dec";
		case CONST:
			return L"const";
		case LT:
			return L"lt";
		case LE:
			return L"le";
		case GT:
			return L"gt";
		case GE:
			return L"ge";
		case EQ:
			return L"eq";
		case NE:
			return L"ne";
		case AND_AND:
			return L"and-and";
		case OR_OR:
			return L"or-or";
		case MULT:
			return L"mult";
		case DIV:
			return L"div";
		case MODULUS:
			return L"mod";
		case CALL:
			if (chain->result->is_deref_ptr_to_func)
				return L"call indir";
			else return L"call";
		case ARG:
			return L"arg";
		case ARRAY:
			return L"array";
		case ARRAY_INIT:
			return L"array-init";
		case INDEX_OF:
			return L"index-of";
		case GOTO:
			return L"goto";
		case LABEL:
			return L"label";
		case NEG:
			return L"neg";
		case NOT:
			return L"not";
		case BITWISE_NOT:
			return L"bitwise-not";
		case INDIR:
			return L"indir";
		case ADDRESS:
			return L"addr";
		case SIZEOF:
			return L"sizeof";
		case COMP_ADD:
			return L"comp-add";
		case COMP_SUB:
			return L"comp-sub";
		case COMP_MULT:
			return L"comp-mult";
		case COMP_DIV:
			return L"comp-div";
		case COMP_AND:
			return L"comp-and";
		case COMP_OR:
			return L"comp-or";
		case BITSHIFT_LEFT:
			return L"bitshift-left";
		case BITSHIFT_RIGHT:
			return L"bitshift-right";
		case CAST:
			swprintf(s, 80, L"cast %ls", chain->GetResult()->var_name);
			return s;
		default:
			return L"Unknown operator";
	}
}
ChainExpression * ChainExpression::MakeLabelChain(wchar_t const * label_name, int id)
{
	Var * label_var = new Var();
	wchar_t s[80];
	swprintf(s,80, L"end_%ls_%d", label_name, id);
	label_var->SetName((wchar_t *)s);
	label_var->SetSpecifier(Var::LABEL);
	IntermediateNode * label_node = new IntermediateNode(ChainExpression::LABEL);
	ChainExpression * chain_result = new ChainExpression(label_node);
	chain_result->result = new Var();
	chain_result->result->Copy(label_var);
	delete label_var;
	return chain_result;
}
ChainExpression * ChainExpression::MakeChain(int value)
{
	ChainExpression * c = new ChainExpression();
	c->result = new Var();
	c->result->SetValue(value);
	c->result->SetVarName(value);
	return c;
}
void ChainExpression::Print(FILE * file)
{
	ChainExpression * chain = this;
	if (chain == 0)
		return;
	
    ClearPrintBuffer();

	if(chain->IsEmpty()){
		if(chain->GetResult()){
			// print  result: eg. identifier;
			wcscat(print_buffer, chain->GetResult()->var_name);
		}
		fprintf(file,"%ls\n", print_buffer);
		return;
	}
	
	Out(chain);
	if(chain->postfix_chain!=0){
		wcscat(print_buffer, L" postfix ");
	   Out(chain->postfix_chain);
	}
	ChainExpression * curr_chain = chain->comma_chain;
	while(curr_chain != 0){
		  wcscat(print_buffer, L" comma ");
		  Out(curr_chain);
		  if(curr_chain->postfix_chain!=0){
			  wcscat(print_buffer, L" postfix ");
			 Out(curr_chain->postfix_chain);
		  }
		  curr_chain = curr_chain->comma_chain;
	}
	
	fprintf(file,"%ls\n", print_buffer);
}
int ChainExpression::IsConstExpr(Var * var)
{
	if (this == 0)
		return 0;

	if (IsEmpty()){
		return 0;
	}

	if (chain.size() == 1)
	{
		int value1, value2;
		wchar_t buffer[80];
		list<IntermediateNode>::iterator it = chain.begin();
		if (it->chain_left->IsConst(&value1) && it->chain_right->IsConst(&value2)){
			// constant, not expression
			int first_operand = Calculate(value1, value2, it->op);
			int result_value = Calculate(first_operand, it->constant_list);
			// assign var object
			var->SetValue(result_value);
			swprintf(buffer, 80, L"CONST_%d", result_value);
			var->SetName(buffer);
			return 1;
		}
	}
	return 0;
}
/*int ChainExpression::IsLValue()
{
	int tmp;
	if (this == 0)
	    return 0;
	if (this->chain.size() > 0)
		return 0;
	if (this->result == 0)
		return 0;
	if (this->IsConst(&tmp))
		return 0;
	return 1;
}*/
/*void ChainExpression::GetExpressionTypeInfo(int * type, int * is_pointer)
{
	if (this->chain.size() == 0){
		*type = this->result->GetType();
		//*is_pointer = this->result->is_pointer;
		return;
	}

	list<IntermediateNode>::iterator it = this->chain.begin();
	*type = it->chain_left->result->GetType();
	//*is_pointer = it->chain_left->result->is_pointer;
}*/
SundType * ChainExpression::GetSundType()
{
	if (this->chain.size() == 0){
		return result->GetSundType();
	}

	list<IntermediateNode>::iterator it = this->chain.begin();
	if (result == 0) {
		return new SundType(SundCompiler::INT);
	}
	else return result->GetSundType();
}
void ChainExpression::Out(ChainExpression * chain)
{
	if(chain->IsEmpty()){
        // print  result
		wcscat(print_buffer, chain->GetResult()->var_name);
		wcscat(print_buffer, L" ");
		return;
	}
	
	for(list<IntermediateNode>::iterator it=chain->chain.begin();it!=chain->chain.end();it++)
	{
		if (!it->is_valid)
			continue;
		
	    // print a node: operator name
		wcscat(print_buffer, this->GetOperatorName(it->op,chain));
		wcscat(print_buffer, L" ");
	    // left operand
		if(it->chain_left == 0){
			if(it->var_left!=0){
			  // print a variable
			  wcscat(print_buffer, it->var_left->var_name);
			}
		}
		else{
			// recursion
			Out(it->chain_left);
		}
		if(it->n_operands==2){
			wcscat(print_buffer, L",");
		}
		else{
			wcscat(print_buffer, L" ");
		}

		// and right operand
		if(it->n_operands==2){
			if(it->chain_right == 0){
				// print a variable
				wcscat(print_buffer, it->var_right->var_name);
			}
			else{
				// recursion
				Out(it->chain_right);
			}
		}
	}
}
int ChainExpression::Calculate(int value1, int value2, int op)
{
	switch (op){
	case ADD:
		return (value1 + value2);
	case SUB:
		return (value1 - value2);
	case MULT:
		return (value1 * value2);
	case DIV:
		return (value1 / value2);
	case MODULUS:
		return (value1 % value2);
	case AND:
		return (value1 & value2);
	case OR:
		return (value1 | value2);
	case XOR:
		return (value1 ^ value2);
	case LT:
		return (value1 < value2);
	case LE:
		return (value1 <= value2);
	case GT:
		return (value1 > value2);
	case GE:
		return (value1 >= value2);
	case EQ:
		return (value1 == value2);
	case NE:
		return (value1 != value2);
	case OR_OR:
		return (value1 || value2);
	case AND_AND:
		return (value1 && value2);
	case BITSHIFT_LEFT:
		return (value1 << value2);
	case BITSHIFT_RIGHT:
		return (value1 >> value2);
	default:
		return 0;
	}
}
int ChainExpression::Calculate(int first_operand, list<IntermediateNode *> &constant_list)
{
	int i_result = first_operand;
	for (list<IntermediateNode *>::iterator it = constant_list.begin(); it != constant_list.end();it++)
	{
		IntermediateNode * c_node = (*it);
		i_result = Calculate(i_result,c_node->chain_right->result->GetValue(),c_node->op);
		i_result = i_result;
	}
	return i_result;
}
void ChainExpression::ClearPrintBuffer()
{
	wcscpy(print_buffer, L"");
}
int ChainExpression::IsEmpty()
{
	int length = (int)chain.size();
	return (length==0);
}
int ChainExpression::IsCompleted()
{
	int length = (int)chain.size();
	if(length == 0){
		return 0;
	}

	list<IntermediateNode>::iterator it  = chain.begin();
	std::advance(it,length-1);
	if(it->chain_right==0 && it->chain_right==0){
	   return 0;
	}
	else{
	   return 1;
	}
}
int ChainExpression::IsConst(int * value)
{
	if (this == 0)
		return 0;
	if (this->result == 0)
		return 0;

	if (this->result->IsConst()){
		*value = this->result->GetValue();
		return 1;
	}

	return 0;
}
int ChainExpression::IsZeroConstant()
{
	int tmp;
	if (IsConst(&tmp)){
		if (tmp == 0)
			return 1;
	}
	return 0;
}
ChainExpression * ChainExpression::ReduceConstants()
{
	if (kind == ADD_SUB)
	{
		ReduceConstantsAddSub();
		return this;
	}
	EliminateConstants();
	return this;
}
void ChainExpression::EliminateConstants()
{
	int zero_operand = 0;

	for (list<IntermediateNode>::iterator it = chain.begin(); it != chain.end(); it++)
	{
		int is_left = 0;
		if (it->constant_list.size() > 0){
			int v1, v2, first_operand = 0, result = 0;
			if (it->chain_left->IsConst(&v1)){
				first_operand = v1;
				is_left = 1;
			}
			if (it->chain_right->IsConst(&v2)){
				if (is_left){
					first_operand = Calculate(first_operand, v2, it->op);
				}
				else{
					if (GetZeroOperand(&zero_operand)){
						first_operand = Calculate(zero_operand, v2, it->op);
					}
					else first_operand = v2;
				}
			}
			result = Calculate(first_operand, it->constant_list);
			if (is_left){
				it->chain_left->result->SetValue(result);
				it->chain_left->result->SetVarName(result);
			}
			else{
				it->chain_right->result->SetValue(result);
				it->chain_right->result->SetVarName(result);
			}
			it->constant_list.clear();
		}
	}
}
void ChainExpression::ReduceConstantsAddSub()
{
	// first pass
	EliminateConstants();
	if (!Compiler->GetConstantFoldingEnabled())
		return;

	int zero_operand = 0;
	// second pass
	ChainExpression * p = 0;
	int i = 0, tmp;
	for (list<IntermediateNode>::iterator it = chain.begin(); it != chain.end(); it++)
	{
		if (i++ == 0){
			//left
			if (p == 0){
				if (it->chain_left->IsConst(&tmp)){
					// assign p
					p = it->chain_left;
				}
			}
		}
		//right
		if (it->chain_right->IsConst(&tmp)){
			if (p == 0){
				// assign p
				int first_operand;
				p = it->chain_right;
				if (GetZeroOperand(&zero_operand)){
					first_operand = Calculate(zero_operand, it->chain_right->result->GetValue(), it->op);
					p->result->SetValue(first_operand);
					p->result->SetVarName(first_operand);
				}
			}
			else{
				// update p, chain_right
				int v1 = p->result->GetValue();
				int v2 = it->chain_right->result->GetValue();
				int res_value = ChainExpression::Calculate(v1, v2, it->op);
				p->result->SetValue(res_value);
				p->result->SetVarName(res_value);
				it->chain_right->result->SetValue(0);
				it->chain_right->result->SetVarName(0);
				// set node as invalid
				it->is_valid = 0;
			}
		}
	}
}
int ChainExpression::GetZeroOperand(int *zero_operand)
{
	if (kind == ADD_SUB)
	{
		*zero_operand = 0;
		return 1;
	}
	if (kind == MULT_DIV)
	{
		*zero_operand = 1;
		return 1;
	}
	return 0;
}
ChainExpression::~ChainExpression(void)
{
}
