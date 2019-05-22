#include "../Include/SundParser.h"
#include "../Include/Scanner.h"
#include "../Include/ErrorPrinter.h"
#include "../Include/ScannerState.h"
#include "../Include/ChainExpression.h"
#include "../Include/SundCompiler.h"
#include "../Include/IntermediateNode.h"
#include "../Include/SundData.h"
#include "../Include/SundTypeBuilder.h"


/*SundParser::SundParser(void)
{
	
}*/
SundParser::SundParser(Scanner * scanner)
{
	this->postfix_chain = 0;
	this->scanner = scanner;
}
SundParser::~SundParser(void)
{
}
ChainExpression * SundParser::PerformExp(ScannerState * scannerState,int do_separate_expr)
{
	this->scannerState = scannerState;
	
	return PerformExp(do_separate_expr);
}
// entry point into parser
ChainExpression * SundParser::PerformExp(int do_separate_expr)
{
	scanner->GetToken();
    if(!*(scanner->token)){
		Err->Print(ErrorPrinter::NO_EXP);
		return 0;
    }
	if(*(scanner->token)==';'){
		// empty expression
		return 0;
    }
	
	ChainExpression * chain_result = PerformAsgn();
	
	if(chain_result->IsEmpty() && chain_result->GetResult()==0){
		return 0;
	}
	if(postfix_chain!=0){
		chain_result->postfix_chain = postfix_chain;
		// reset postfix_chain;
		postfix_chain = 0;
	}
	if(do_separate_expr!=0)
	{
		ChainExpression * curr_chain = chain_result; 
		while(*(scanner->token) == ','){
			  scanner->GetToken();
			  ChainExpression * comma_chain = PerformAsgn();
			  if(postfix_chain!=0){
				comma_chain->postfix_chain = postfix_chain;
				// reset postfix_chain;
				postfix_chain = 0;
			  }
			  curr_chain->comma_chain = comma_chain;
			  curr_chain = comma_chain;
		}
	}
	
	// return last token read to input stream
	scanner->Putback();	
	
	return chain_result;
}

// process an assigment expression
ChainExpression * SundParser::PerformAsgn()
{
	// holds name of var receiving the assigment
	ScannerState * state = new ScannerState(scanner);
	//wchar_t temp[ID_LEN];
	wchar_t asgn_ops[] = { '=', SundCompiler::COMP_ADD, SundCompiler::COMP_SUB, SundCompiler::COMP_MULT, SundCompiler::COMP_DIV, SundCompiler::COMP_AND, SundCompiler::COMP_OR, 0 };
	int temp_tok = 0;
	wchar_t op;
	
	ChainExpression * chain_result = PerformLogicalOR();
	//assigment
	while (wcschr(asgn_ops, op = *(scanner->token)))
	{
		if (chain_result->is_lvalue)
		{
			IntermediateNode * asgn_node;
			ChainExpression * asgn_chain = new ChainExpression(), *chain;
			scanner->GetToken();
			// obtain left operand
			//Var * temp_left_op = new Var();
			//DataSet->FindVar(chain_result->GetResult()->var_name, this->func_name, temp_left_op);//temp
			switch (op){
			case '=':
				//asgn_node = new IntermediateNode(ChainExpression::ASGN, temp_left_op, 0);
				asgn_node = new IntermediateNode(ChainExpression::ASGN);
				break;
			case SundCompiler::COMP_ADD:
				//asgn_node = new IntermediateNode(ChainExpression::COMP_ADD, temp_left_op, 0);
				asgn_node = new IntermediateNode(ChainExpression::COMP_ADD);
				break;
			case SundCompiler::COMP_SUB:
				//asgn_node = new IntermediateNode(ChainExpression::COMP_SUB, temp_left_op, 0);
				asgn_node = new IntermediateNode(ChainExpression::COMP_SUB);
				break;
			case SundCompiler::COMP_MULT:
				//asgn_node = new IntermediateNode(ChainExpression::COMP_MULT, temp_left_op, 0);
				asgn_node = new IntermediateNode(ChainExpression::COMP_MULT);
				break;
			case SundCompiler::COMP_DIV:
				//asgn_node = new IntermediateNode(ChainExpression::COMP_DIV, temp_left_op, 0);
				asgn_node = new IntermediateNode(ChainExpression::COMP_DIV);
				break;
			case SundCompiler::COMP_AND:
				//asgn_node = new IntermediateNode(ChainExpression::COMP_AND, temp_left_op, 0);
				asgn_node = new IntermediateNode(ChainExpression::COMP_AND);
				break;
			case SundCompiler::COMP_OR:
				//asgn_node = new IntermediateNode(ChainExpression::COMP_OR, temp_left_op, 0);
				asgn_node = new IntermediateNode(ChainExpression::COMP_OR);
				break;
			}
			//delete temp_left_op;
			asgn_node->chain_left = chain_result;
			chain = PerformAsgn();
			asgn_node->chain_right = chain;
			switch (asgn_node->CheckTypes()){
			case SundType::NOT_EQUAL:
				Err->AddError(state, ErrorPrinter::CANNOT_CONVERT_FROM_DIF_TYPE, L"=");//temp chain_result->GetResult()->var_name
				break;
			case SundType::CAST:
				break;
			}
			asgn_chain->AddNode(asgn_node);
			return asgn_chain;
		}
		else{
			//print error:lvalue not valid
			Err->AddError(state, ErrorPrinter::LEFT_SIDE_NOT_LVALUE);
			break;
		}
	}
	return chain_result;
}
ChainExpression * SundParser::PerformLogicalOR()
{
	ChainExpression * chain_left = 0,* chain_right = 0;
	ChainExpression * chain_result = new ChainExpression();
	chain_result->kind = ChainExpression::LOGICAL_OR;
	wchar_t op;
	chain_left = PerformLogicalAND();
	op=*(scanner->token);
	while ((op = *(scanner->token)) == SundCompiler::OR_OR)
	{
		  IntermediateNode * node = new IntermediateNode();
		  Var * im_result = 0;
		  scanner->GetToken();
		  chain_right = PerformLogicalAND();
		  node->op = ChainExpression::OR_OR;
		  if (chain_result->GetChain()->size() == 0){
			  int v1, v2;
			  if (chain_left->IsConst(&v1) && chain_right->IsConst(&v2))
			  {
				  if (Compiler->GetConstantFoldingEnabled()){
					  // update chain_left, when constant folding is enabled
					  int res_value = ChainExpression::Calculate(v1, v2, node->op);
					  chain_left->result->SetValue(res_value);
					  chain_left->result->SetVarName(res_value);
					  delete node;
					  continue;
				  }
			  }
		     node->chain_left = chain_left;
		     im_result = 0;
		  }
		  else{
		     node->chain_left = 0;
		     im_result = new Var();
		     im_result->SetName((wchar_t *)L"ir");
		     node->var_left = new Var();
		     node->var_left->Copy(im_result);
		     //delete im_result;
		  }
		  node->chain_right = chain_right;
		  
		  if (!chain_result->ChangeConstantVar(node) || !Compiler->GetConstantFoldingEnabled())
		  {
			  chain_result->AddNode(node);
		  }
	}
	
	if(chain_result->IsEmpty()){
		delete chain_result;
		return chain_left;
	}
	else{
		return chain_result->ReduceConstants();
	}
}
ChainExpression * SundParser::PerformLogicalAND()
{
	ChainExpression * chain_left = 0,* chain_right = 0;
	ChainExpression * chain_result = new ChainExpression();
	chain_result->kind = ChainExpression::LOGICAL_AND;
	wchar_t op;
	chain_left = PerformBitwiseIOR();
	op=*(scanner->token);
	while ((op = *(scanner->token)) == SundCompiler::AND_AND)
	{
		  IntermediateNode * node = new IntermediateNode();
		  Var * im_result = 0;
		  scanner->GetToken();
		  chain_right = PerformBitwiseIOR();
		  node->op = ChainExpression::AND_AND;
		  if (chain_result->GetChain()->size() == 0){
			  int v1, v2;
			  if (chain_left->IsConst(&v1) && chain_right->IsConst(&v2))
			  {
				  if (Compiler->GetConstantFoldingEnabled()){
					  // update chain_left, when constant folding is enabled
					  int res_value = ChainExpression::Calculate(v1, v2, node->op);
					  chain_left->result->SetValue(res_value);
					  chain_left->result->SetVarName(res_value);
					  delete node;
					  continue;
				  }
			  }
		     node->chain_left = chain_left;
		     im_result = 0;
		  }
		  else{
		     node->chain_left = 0;
		     im_result = new Var();
		     im_result->SetName((wchar_t *)L"ir");
		     node->var_left = new Var();
		     node->var_left->Copy(im_result);
		     //delete im_result;
		  }
		  node->chain_right = chain_right;
		  
		  if (!chain_result->ChangeConstantVar(node) || !Compiler->GetConstantFoldingEnabled())
		  {
			  chain_result->AddNode(node);
		  }
	}
	
	if(chain_result->IsEmpty()){
		delete chain_result;
		return chain_left;
	}
	else{
		return chain_result->ReduceConstants();
		//return chain_result;
	}
}
ChainExpression * SundParser::PerformBitwiseIOR()
{
	ChainExpression * chain_left = 0,* chain_right = 0;
	ChainExpression * chain_result = new ChainExpression();
	chain_result->kind = ChainExpression::BITWISE_IOR;
	wchar_t op;
	chain_left = PerformBitwiseXOR();//
	op=*(scanner->token);
	while((op=*(scanner->token))=='|')
	{
		IntermediateNode * node = new IntermediateNode();
		Var * im_result = 0;
		scanner->GetToken();
		chain_right = PerformBitwiseXOR();
		node->op = ChainExpression::OR;
		if (chain_result->GetChain()->size() == 0){
			int v1, v2;
			if (chain_left->IsConst(&v1) && chain_right->IsConst(&v2))
			{
				if (Compiler->GetConstantFoldingEnabled()){
					// update chain_left, when constant folding is enabled
					int res_value = ChainExpression::Calculate(v1, v2, node->op);
					chain_left->result->SetValue(res_value);
					chain_left->result->SetVarName(res_value);
					delete node;
					continue;
				}
			}
		   node->chain_left = chain_left;
		   im_result = 0;
		}
		else{
		   node->chain_left = 0;
		   im_result = new Var();
		   im_result->SetName((wchar_t *)L"ir");
		   node->var_left = new Var();
		   node->var_left->Copy(im_result);
		   //delete im_result;
		}
		node->chain_right = chain_right;
		
		if (!chain_result->ChangeConstantVar(node) || !Compiler->GetConstantFoldingEnabled())
		{
			chain_result->AddNode(node);
		}
		switch (node->CheckTypes()){
		case SundType::NOT_EQUAL:
			Err->AddError(ErrorPrinter::OPERANDS_HAVE_ILLEGAL_TYPES, L"|");
			break;
		case SundType::CAST:
			break;
		}
	}

	if(chain_result->IsEmpty()){
		delete chain_result;
		return chain_left;
	}
	else{
		return chain_result->ReduceConstants();
		//return chain_result;
	}
}
ChainExpression * SundParser::PerformBitwiseXOR()
{
	ChainExpression * chain_left = 0,* chain_right = 0;
	ChainExpression * chain_result = new ChainExpression();
	chain_result->kind = ChainExpression::BITWISE_XOR;
	wchar_t op;
	chain_left = PerformBitwiseAND();//
	op=*(scanner->token);
	while((op=*(scanner->token))=='^')
	{
		IntermediateNode * node = new IntermediateNode();
		Var * im_result = 0;
		scanner->GetToken();
		chain_right = PerformBitwiseAND();//
		node->op = ChainExpression::XOR;
		if (chain_result->GetChain()->size() == 0){
			int v1, v2;
			if (chain_left->IsConst(&v1) && chain_right->IsConst(&v2))
			{
				if (Compiler->GetConstantFoldingEnabled()){
					// update chain_left, when constant folding is enabled
					int res_value = ChainExpression::Calculate(v1, v2, node->op);
					chain_left->result->SetValue(res_value);
					chain_left->result->SetVarName(res_value);
					delete node;
					continue;
				}
			}
		   node->chain_left = chain_left;
		   im_result = 0;
		}
		else{
		   node->chain_left = 0;
		   im_result = new Var();
		   im_result->SetName((wchar_t *)L"ir");
		   node->var_left = new Var();
		   node->var_left->Copy(im_result);
		   //delete im_result;
		}
		node->chain_right = chain_right;
		
		if (!chain_result->ChangeConstantVar(node) || !Compiler->GetConstantFoldingEnabled())
		{
			chain_result->AddNode(node);
		}
		switch (node->CheckTypes()){
		case SundType::NOT_EQUAL:
			Err->AddError(ErrorPrinter::OPERANDS_HAVE_ILLEGAL_TYPES, L"^");
			break;
		case SundType::CAST:
			break;
		}
	}

	if(chain_result->IsEmpty()){
		delete chain_result;
		return chain_left;
	}
	else{
		return chain_result->ReduceConstants();
	}
}
ChainExpression * SundParser::PerformBitwiseAND()
{
	ChainExpression * chain_left = 0,* chain_right = 0;
	ChainExpression * chain_result = new ChainExpression();
	chain_result->kind = ChainExpression::BITWISE_AND;
	wchar_t op;
	//int i = 0;
	chain_left = PerformRelOp();
	op=*(scanner->token);
	while((op=*(scanner->token))=='&')
	{
		IntermediateNode * node = new IntermediateNode();
		Var * im_result = 0;
		scanner->GetToken();
		chain_right = PerformRelOp();
		node->op = ChainExpression::AND;
		if (chain_result->GetChain()->size() == 0){
			int v1, v2;
			if (chain_left->IsZeroConstant() || chain_right->IsZeroConstant()){
				//dead expr : value&0 or 0&value or 0&0 Expression Simplification
				chain_left->result->SetQualifier(Var::CONST);
				chain_left->result->SetValue(0);
				chain_left->result->SetVarName(0);
				delete node;
				continue;
			}
			if (chain_left->IsConst(&v1) && chain_right->IsConst(&v2))
			{
				if (Compiler->GetConstantFoldingEnabled()){
					// update chain_left, when constant folding is enabled
					int res_value = ChainExpression::Calculate(v1, v2, node->op);
					chain_left->result->SetValue(res_value);
					chain_left->result->SetVarName(res_value);
					delete node;
					continue;
				}
			}
		   node->chain_left = chain_left;
		   im_result = 0;
		}
		else{
		   node->chain_left = 0;
		   im_result = new Var();
		   im_result->SetName((wchar_t *)L"ir");
		   node->var_left = new Var();
		   node->var_left->Copy(im_result);
		}
		node->chain_right = chain_right;
		if (chain_right->IsZeroConstant())
		{
			//dead expr : value&0 Expression Simplification
			chain_left->result->SetQualifier(Var::CONST);
			chain_left->result->SetValue(0);
			chain_left->result->SetVarName(0);
			delete node;
			continue;
		}
		// build node result_type
		node->BuildResultType();
		// constant folding
		if (!chain_result->ChangeConstantVar(node) || !Compiler->GetConstantFoldingEnabled())
		{
			chain_result->AddNode(node);
		}
		switch (node->CheckTypes()){
		case SundType::NOT_EQUAL:
			Err->AddError(ErrorPrinter::OPERANDS_HAVE_ILLEGAL_TYPES, L"&");
			break;
		case SundType::CAST:
			break;
		}
	}

	if(chain_result->IsEmpty()){
		delete chain_result;
		return chain_left;
	}
	else{
		return chain_result->ReduceConstants();
	}
}
// process relational operators
ChainExpression * SundParser::PerformRelOp()
{
	ChainExpression * chain_left = 0,* chain_right = 0;
	ChainExpression * chain_result = 0;
	wchar_t relops[] = { LT, LE, GT, GE, EQ, NE, 0 };
	wchar_t op, s_op[5];
	ScannerState * state = new ScannerState(scanner);
	chain_left = PerformBitshift();//PerformAddSub();
	op=*(scanner->token);
	if (wcschr(relops, op))
	{
		IntermediateNode * node = new IntermediateNode();
		scanner->GetToken();
		chain_right = PerformBitshift();//PerformAddSub();
		node->chain_left = chain_left;
		node->chain_right = chain_right;
		
		switch(op){
			case LT:
				node->op = ChainExpression::LT;
				wcscpy(s_op, L"<");
			break;
			case LE:
				node->op = ChainExpression::LE;
				wcscpy(s_op, L"<=");
			break;
			case GT:
				node->op = ChainExpression::GT;
				wcscpy(s_op, L">");
			break;
			case GE:
				node->op = ChainExpression::GE;
				wcscpy(s_op, L">=");
			break;
			case EQ:
				node->op = ChainExpression::EQ;
				wcscpy(s_op, L"==");
			break;
			case NE:
				node->op = ChainExpression::NE;
				wcscpy(s_op, L"!=");
			break;
		}
		int v1, v2;
		if (chain_left->IsConst(&v1) && chain_right->IsConst(&v2))
		{
			if (Compiler->GetConstantFoldingEnabled()){
				// update chain_left, when constant folding is enabled
				int res_value = ChainExpression::Calculate(v1, v2, node->op);
				chain_left->result->SetValue(res_value);
				chain_left->result->SetVarName(res_value);
				delete node;
				//continue;
				return chain_left;
			}
		}
		chain_result = new ChainExpression();
		// constant folding
		/*if (!chain_result->ChangeConstantVar(node) || !Compiler->GetConstantFoldingEnabled())
		{
			chain_result->AddNode(node);
		}*/
		// build node result_type
		node->BuildResultType();
		chain_result->AddNode(node);
		switch (node->CheckTypes()){
		case SundType::NOT_EQUAL:
			Err->AddError(state,ErrorPrinter::OPERANDS_HAVE_ILLEGAL_TYPES, s_op);
			break;
		case SundType::CAST:
			break;
		}
	}
	if(chain_right == 0){
		return chain_left;
	}
	else{
		chain_result->kind = ChainExpression::REL_OP;
		return chain_result->ReduceConstants();
	}
}
ChainExpression * SundParser::PerformBitshift()
{
	ChainExpression * chain_left = 0,* chain_right = 0;
	ChainExpression * chain_result = new ChainExpression();
	chain_result->kind = ChainExpression::BITSHIFT;
	wchar_t op,s_op[5];
	ScannerState * state = new ScannerState(scanner);
	chain_left = PerformAddSub();
	op=*(scanner->token);
	while ((op = *(scanner->token)) == SundCompiler::BITSHIFT_LEFT || op == SundCompiler::BITSHIFT_RIGHT)
	{
		IntermediateNode * node = new IntermediateNode();
		Var * im_result = 0;
		scanner->GetToken();
		chain_right = PerformAddSub();
		switch (op)	// bitshift
		{
		case SundCompiler::BITSHIFT_LEFT:
			node->op = ChainExpression::BITSHIFT_LEFT;
			wcscpy(s_op, L"<<");
			break;
		case SundCompiler::BITSHIFT_RIGHT:
			node->op = ChainExpression::BITSHIFT_RIGHT;
			wcscpy(s_op, L">>");
			break;
		}
		if (chain_result->GetChain()->size() == 0){
			int v1, v2;
			if (chain_left->IsConst(&v1) && chain_right->IsConst(&v2))
			{
				if (Compiler->GetConstantFoldingEnabled()){
					// update chain_left, when constant folding is enabled
					int res_value = ChainExpression::Calculate(v1, v2, node->op);
					chain_left->result->SetValue(res_value);
					chain_left->result->SetVarName(res_value);
					delete node;
					continue;
				}
			}
		   node->chain_left = chain_left;
		   im_result = 0;
		}
		else{
		   node->chain_left = 0;
		   im_result = new Var();
		   im_result->SetName((wchar_t *)L"ir");
		   node->var_left = new Var();
		   node->var_left->Copy(im_result);
		   //delete im_result;
		}
		node->chain_right = chain_right;
		// build node result_type
		node->BuildResultType();
		// constant folding
		if (!chain_result->ChangeConstantVar(node) || !Compiler->GetConstantFoldingEnabled()){
			chain_result->AddNode(node);
		}
		switch (node->CheckTypes()){
		case SundType::NOT_EQUAL:
			Err->AddError(state, ErrorPrinter::OPERANDS_HAVE_ILLEGAL_TYPES, s_op);
			break;
		case SundType::CAST:
			break;
		}
    }
	if(chain_result->IsEmpty()){
		delete chain_result;
		return chain_left;
	}
	else{
		return chain_result->ReduceConstants();
	}
}
// add or subcract two terms
ChainExpression * SundParser::PerformAddSub()
{
	//int value1, value2;
	ChainExpression * chain_left = 0,* chain_right = 0;
	ChainExpression * chain_result = new ChainExpression();
	chain_result->kind = ChainExpression::ADD_SUB;
	wchar_t op;
	
	chain_left = PerformMultDiv();
	op=*(scanner->token);
	while((op=*(scanner->token))=='+'||op=='-')
	{
		IntermediateNode * node = new IntermediateNode();
		Var * im_result = 0;
		scanner->GetToken();
		chain_right = PerformMultDiv();
		switch (op)	// add or sub
		{
		case '-':
			node->op = ChainExpression::SUB;
			break;
		case '+':
			node->op = ChainExpression::ADD;
			break;
		}
		if (chain_result->GetChain()->size() == 0){
			int v1, v2;
			if (chain_left->IsConst(&v1) && chain_right->IsConst(&v2))
			{
				if (Compiler->GetConstantFoldingEnabled()){
					// update chain_left, when constant folding is enabled
					int res_value = ChainExpression::Calculate(v1, v2, node->op);
					chain_left->result->SetValue(res_value);
					chain_left->result->SetVarName(res_value);
					delete node;
					continue;
				}
			}
			else
			if (chain_left->IsZeroConstant() && !chain_right->IsConst(&v2)){
				// left operand = 0, right is not constant
				delete chain_left;
				chain_left = chain_right;
				continue;
			}
		    node->chain_left = chain_left;
		    im_result = 0;
		}
		else{
		   node->chain_left = 0;
		   im_result = new Var();
		   im_result->SetName((wchar_t *)L"ir");
		   node->var_left = new Var();
		   node->var_left->Copy(im_result);
		   //delete im_result;
		}
		node->chain_right = chain_right;
		if (chain_right->IsZeroConstant()){
			// op+0
			delete chain_right;
			continue;
		}
		// build node result_type
		node->BuildResultType();
		// constant folding
		if (!chain_result->ChangeConstantVar(node) || !Compiler->GetConstantFoldingEnabled()){
		    chain_result->AddNode(node);
		}
    }
	if(chain_result->IsEmpty()){
		delete chain_result;
		return chain_left;
	}
	else{
		return chain_result->ReduceConstants();
	}
}

// multiple or divide two factors
ChainExpression * SundParser::PerformMultDiv()
{
	ChainExpression * chain_left = 0,* chain_right = 0;
	ChainExpression * chain_result = new ChainExpression();
	chain_result->kind = ChainExpression::MULT_DIV;
	wchar_t op;
	wchar_t s_op[3];
	chain_left = PerformUnaryOp();//PerformUnaryPlusMinus();
	int is_mult,is_div;
	ScannerState * s = new ScannerState(scanner);
	while((op=*(scanner->token))=='*'||op=='/'||op=='%')
	{
		    IntermediateNode * node = new IntermediateNode();
			Var * im_result = 0;
			is_mult = 0;
			is_div = 0;
			scanner->GetToken();
			chain_right = PerformUnaryOp();//PerformUnaryPlusMinus();
			switch (op) //  multiple or divide or modulus
			{
			case '*':
				node->op = ChainExpression::MULT;
				is_mult = 1;
				wcscpy(s_op,L"*");
				break;
			case '/':
				node->op = ChainExpression::DIV;
				is_div = 1;
				wcscpy(s_op, L"/");
				break;
			case '%':
				node->op = ChainExpression::MODULUS;
				wcscpy(s_op, L"%");
				break;
			}
			if(chain_result->GetChain()->size() == 0){
				int v1, v2;
				if (is_mult && (chain_left->IsZeroConstant() || chain_right->IsZeroConstant())){
					//dead expr : value*0 or 0*value or 0*0
					chain_left->result->SetQualifier(Var::CONST);
					chain_left->result->SetValue(0);
					chain_left->result->SetVarName(0);
					delete node;
					continue;
				}
				if (is_div && chain_right->IsZeroConstant()){
					Err->Print(s,ErrorPrinter::DIV_BY_ZERO);
					return 0;
				}
				if (chain_left->IsConst(&v1) && chain_right->IsConst(&v2))
				{
					if (Compiler->GetConstantFoldingEnabled()){
					    // update chain_left, when constant folding is enabled
						int res_value = ChainExpression::Calculate(v1, v2, node->op);
						chain_left->result->SetValue(res_value);
						chain_left->result->SetVarName(res_value);
						delete node;
					    continue;
					}
				}
				node->chain_left = chain_left;
				im_result = 0;
			}
			else{
				node->chain_left = 0;
				im_result = new Var();
				im_result->SetName((wchar_t *)L"ir");
				node->var_left = new Var();
				node->var_left->Copy(im_result);
				//delete im_result;
			}
			node->chain_right = chain_right;
			if (is_mult && chain_right->IsZeroConstant())
			{
				//dead expr : value*0 
				chain_left->result->SetQualifier(Var::CONST);
				chain_left->result->SetValue(0);
				chain_left->result->SetVarName(0);
				delete node;
				continue;
			}
			if (is_div && chain_right->IsZeroConstant()){
				Err->Print(s,ErrorPrinter::DIV_BY_ZERO);
				return 0;
			}
			// build node result_type
			node->BuildResultType();
			// constant folding
			if (!chain_result->ChangeConstantVar(node) || !Compiler->GetConstantFoldingEnabled())
			{
				chain_result->AddNode(node);
			}
			switch (node->CheckTypes()){
			case SundType::NOT_EQUAL:
				Err->AddError(ErrorPrinter::OPERANDS_HAVE_ILLEGAL_TYPES, s_op);
				break;
			case SundType::CAST:
				break;
			}
    }
	
	if(chain_result->IsEmpty()){
		delete chain_result;
		return chain_left;
	}
	else{
		return chain_result->ReduceConstants();
		//return chain_result;
	}
}
ChainExpression * SundParser::PerformUnaryOp()
{
	ChainExpression * chain_postfix = 0;
	ChainExpression * chain_result = new ChainExpression();
	Var * cast_info_var = 0;
	wchar_t unary_ops[] = { '+', '-', '*', '&', '~', '!', SundCompiler::INCREMENT, SundCompiler::DECREMENT, 0 };
	wchar_t op = '\0', ch = *(scanner->token);
	ScannerState * s = new ScannerState(scanner);
	if (!wcscmp(scanner->token, L"sizeof")){
		return PerformSizeof();
	}
	else
		if (scanner->token_type == SundCompiler::CAST){
		// (type): cast (convert value to temporary value of type)
		cast_info_var = new Var();
		cast_info_var->SetName((wchar_t *)scanner->token);
		scanner->GetToken();
		op = SundCompiler::CAST;
	}
	else
	if (wcschr(unary_ops, ch))
	{
		op=*(scanner->token);
		scanner->GetToken();
    }
	chain_postfix =  PerformPostfixExp();
	IntermediateNode * node;
	if(op)
	{
		if(op!='+'){
			//IntermediateNode * node;// = new IntermediateNode();
			//node->n_operands = 1;
			Var * im_result = new Var();
			im_result->SetName((wchar_t *)L"ir");
			switch(op){
				case '-':
				node = new IntermediateNode();
				node->n_operands = 1;
				node->op = ChainExpression::NEG;
				chain_result->result = new Var();
				chain_result->result->Copy(im_result);
				//delete im_result;
				break;
				case '!':
				node = new IntermediateNode();
				node->n_operands = 1;
				node->op = ChainExpression::NOT;
				chain_result->result = new Var();
				chain_result->result->Copy(im_result);
				//delete im_result;
				break;
				case '~':
			    node = new IntermediateNode();
				node->n_operands = 1;
				node->op = ChainExpression::BITWISE_NOT;
				chain_result->result = new Var();
				chain_result->result->Copy(im_result);
				//delete im_result;
				break;
				case SundCompiler::INCREMENT:
			    node = new IntermediateNode();
				node->n_operands = 1;
				node->op = ChainExpression::INC;
				chain_result->result = new Var();
				chain_result->result->Copy(im_result);
				//delete im_result;
				break;
				case SundCompiler::DECREMENT:
				node = new IntermediateNode();
				node->n_operands = 1;
				node->op = ChainExpression::DEC;
				chain_result->result = new Var();
				chain_result->result->Copy(im_result);
				//delete im_result;
				break;
				case SundCompiler::CAST:
				node = new IntermediateNode();
				node->n_operands = 1;
				node->op = ChainExpression::CAST;
				chain_result->result = new Var();
				chain_result->result->Copy(cast_info_var);
				//delete cast_info_var;
				break;
				case '*':
				node = new IntermediateNode();
				node->n_operands = 1;
				node->op = ChainExpression::INDIR;
				chain_result->result = new Var();
				//chain_result->result->Copy(im_result);
				if (chain_postfix->GetResult()->GetSundType()->IsPtrToFunc()){
					//chain_postfix->GetResult()->is_deref_ptr_to_func = 1;
					DataSet->DoDefer(this->identifier,this->func_name);
				}
				if (chain_postfix->GetResult()->GetSundType()->GetTopTypeCode() == SundCompiler::POINTER){
					chain_postfix->GetResult()->GetSundType()->sund_type_list.pop_back();
					chain_result->result->Copy(chain_postfix->GetResult());
				}
				else{
					// print error
					Err->Print(s,ErrorPrinter::ILLEGAL_INDIR);
					return 0;
				}
				chain_result->is_lvalue = 1;
				//delete im_result;
				break;
				case '&':
				node = new IntermediateNode();
				node->n_operands = 1;
				node->op = ChainExpression::ADDRESS;

				chain_result->result = new Var();
				chain_postfix->GetResult()->GetSundType()->sund_type_list.push_front(new sund_type_node(SundCompiler::POINTER, -1));
				chain_result->result->Copy(chain_postfix->GetResult());

				//chain_result->result = new Var();
				//chain_result->result->Copy(im_result);
				//delete im_result;
				break;
				default:
				node = new IntermediateNode();
				break;
			}
			node->chain_left = chain_postfix;
			chain_result->AddNode(node);
		}
    }

	if(chain_result->IsEmpty()){
		delete chain_result;
		return chain_postfix;
	}
	else{
		return chain_result;
	}
}

// () [] -> . ,posfix ++,--
ChainExpression * SundParser::PerformPostfixExp()
{
	wchar_t temp[80];
	wchar_t op;
	wcscpy(temp, scanner->token);
	
	ChainExpression * chain_result = PerformPrimaryExp();
	if ((op = *(scanner->token)) == '[' || (op = *(scanner->token)) == SundCompiler::INCREMENT || (op = *(scanner->token)) == SundCompiler::DECREMENT || (op = *(scanner->token)) == '('){
		switch(op){
			case '[':
			chain_result = PerformArray(temp,chain_result);
			chain_result->is_lvalue = 1;
			scanner->GetToken();
			break;
			case '(':
			chain_result = PerformCallFunc();
			scanner->GetToken();
			break;
			case SundCompiler::INCREMENT:
			case SundCompiler::DECREMENT:
			Var * temp_right_op = chain_result->result;//new Var();
			/*if(DataSet->GetVar(temp,this->func_index,temp_right_op)==0)
			{
				// print error
				Err->Print(this->scannerState,ErrorPrinter::SYNTAX,"postfix expression");
				return 0;
			}*/
			if(postfix_chain==0)
			{
				// create ChainExpression object
				postfix_chain = new ChainExpression();
			}
			DoPostfixIncDec(temp_right_op);
			scanner->GetToken();
			break;
		}
		
	}

	/*if(chain_result->IsEmpty()){
		delete chain_result;
		return chain_primary;
	}
	else{
		return chain_result;
	}*/
	return chain_result;
}
// process  right value:identifier (var or function),constant
ChainExpression * SundParser::PerformPrimaryExp()
{
	ChainExpression * r_chain = new ChainExpression();
	ProgramControlBase * new_program_control = new ProgramControlBase(ProgramControlBase::STRING);
	int string_id = -1;
	Var * temp_right_op = new Var(); 
	Var * cnst_var; 
	wchar_t name[80], buffer[80];
	char char_value;
	int d_result;// , f_result;
	switch(scanner->token_type){
	case SundCompiler::IDENTIFIER:
		wcscpy(name, scanner->token);
		wcscpy(this->identifier, scanner->token);
		d_result = DataSet->FindVar(name, this->func_name, temp_right_op, this->program_control);
		//d_result = DataSet->GetVar(name,this->func_index,temp_right_op,this->program_control);
		//f_result = DataSet->GetFunc(name);
		if (d_result == 0) // && f_result==-1)
		{
			wchar_t temp[80];
			scanner->ReadNextToken(temp);
			if (*temp == '('){
				// identifier is maybe function
				Err->Print(ErrorPrinter::FUNC_REQUIRES_ANSI_STYLE, scanner->token);
				return 0;
			}
			// print error
			Err->Print(this->scannerState, ErrorPrinter::UNDECLARED_IDENTIFIER,name);
			return 0;
		}
		
		r_chain->result = new Var();
		r_chain->result->Copy(temp_right_op);
		scanner->GetToken();
		r_chain->is_lvalue = 1;
		return r_chain;
	case SundCompiler::NUMBER:
		cnst_var = new Var((wchar_t *)"",-1,Var::CONST);
		cnst_var->SetValue(_wtoi(scanner->token));
		swprintf(buffer,80, L"CONST_%s", scanner->token);
		cnst_var->SetName(buffer);
		wcscpy(this->identifier, buffer);
		//cnst_var->SetQualifier(Var::CONST);
	    r_chain->result = new Var();
		r_chain->result->Copy(cnst_var);
		scanner->GetToken();
		r_chain->is_lvalue = 0;
		return r_chain;
	case SundCompiler::HEX_NUMBER:
		cnst_var = new Var((wchar_t *)"",-1,Var::CONST);
		cnst_var->SetValue(H(scanner->token));
		swprintf(buffer,80, L"CONST_0x%s", scanner->token);
		cnst_var->SetName(buffer);
		wcscpy(this->identifier, buffer);
		//cnst_var->SetQualifier(Var::CONST);
	    r_chain->result = new Var();
		r_chain->result->Copy(cnst_var);
		scanner->GetToken();
		r_chain->is_lvalue = 0;
		return r_chain;
	case SundCompiler::B_NUMBER:
		cnst_var = new Var((wchar_t *)"",-1,Var::CONST);
		cnst_var->SetValue(B(scanner->token));
		swprintf(buffer,80, L"CONST_0b%s", scanner->token);
		cnst_var->SetName(buffer);
		wcscpy(this->identifier, buffer);
		//cnst_var->SetQualifier(Var::CONST);
	    r_chain->result = new Var();
		r_chain->result->Copy(cnst_var);
		scanner->GetToken();
		r_chain->is_lvalue = 0;
		return r_chain;
	case SundCompiler::CHARACTER:
		cnst_var = new Var((wchar_t *)"",-1,Var::CONST);
		char_value = GetCharacterValue(scanner->token);
		cnst_var->SetValue(char_value);
		swprintf(buffer, 80, L"CONST_%s", scanner->token);
		cnst_var->SetName(buffer);
		wcscpy(this->identifier, buffer);
		//cnst_var->SetQualifier(Var::CONST);
		r_chain->result = new Var();
		r_chain->result->Copy(cnst_var);
		scanner->GetToken();
		r_chain->is_lvalue = 0;
		return r_chain;
	case SundCompiler::STRING:
		new_program_control->parent = program_control;
		string_id = ProgramControlBase::GenerateId(ProgramControlBase::STRING);
		new_program_control->SetId(string_id);
		new_program_control->SetString(scanner->token, wcslen(scanner->token));
		ProgramControlBase::global_instance->Add(new_program_control);
		cnst_var = new Var((wchar_t *)"", -1, Var::CONST);
		cnst_var->SetValue(0);
		swprintf(buffer, 80, L"string_%d", string_id);
		cnst_var->SetName(buffer);
		wcscpy(this->identifier, buffer);
		r_chain->result = new Var();
		r_chain->result->Copy(cnst_var);
		scanner->GetToken();
		r_chain->is_lvalue = 0;
		return r_chain;
	case SundCompiler::KEYWORD:
		//r_chain->result = new Var();
		Err->Print(new ScannerState(scanner), ErrorPrinter::VAR_IS_KEYWORD, scanner->token);
		return 0;
	case SundCompiler::DELIMITER:
		// process parenthezis expression eg. (expr)
		if((*(scanner->token)=='('))
		{
			ChainExpression * chain = 0;
			scanner->GetToken();
			chain = PerformAsgn();
			if(*(scanner->token)!=')'){
				Err->Print(this->scannerState,ErrorPrinter::PAREN_EXPECTED);
			}
			scanner->GetToken();
			return chain;
		}
		Err->Print(this->scannerState,ErrorPrinter::UNSUP_EXPR);
        return 0;
		default:
		if(*(scanner->token)==')'){
			return r_chain;
		}
		else{
			Err->Print(this->scannerState,ErrorPrinter::SYNTAX);
			r_chain->is_lvalue = 0;
			return r_chain;
		}
    }
}
// process the call function. 
ChainExpression * SundParser::PerformCallFunc()
{
	ChainExpression * func_chain = new ChainExpression();
	list<SundType *> func_signature;
	wchar_t temp[80];
	Var * v = DataSet->GetPtrToFunc(this->identifier, this->func_name);
	if (v == 0)
	{
		// get function!
		v = DataSet->GetFuncVar(this->identifier);
		if (v == 0)
		{
			Err->Print(this->scannerState, ErrorPrinter::MISS_PROTOTYPE, this->identifier);
			return 0;
		}
		else{
			// set is_called = 1
			DataSet->SetFuncCalled(this->identifier);
		}
	}
	else{
		// ptr to func found!
		if (v->is_deref_ptr_to_func)
		{
			v->GetSundType()->GetSundTypeList()->pop_front();
		}
	}
	/*if (v != 0){
		// ptr to func found!
		if (!v->is_deref_ptr_to_func)
		{
			Err->Print(this->scannerState, ErrorPrinter::MISS_PROTOTYPE, this->identifier);//func_name
			return 0;
		}
	}
	else{
		// get function!
		v = DataSet->GetFuncVar(this->identifier);
		if (v == 0)
		{
			Err->Print(this->scannerState, ErrorPrinter::MISS_PROTOTYPE, this->identifier);//func_name
			return 0;
		}
		else{
			// set is_called = 1
			DataSet->SetFuncCalled(this->identifier);//func_name
		}
	}*/
	/*if (v == 0){
		v = DataSet->GetFuncVar(func_name);//func_name
	}
	if (v == 0)
	{
		Err->Print(this->scannerState, ErrorPrinter::MISS_PROTOTYPE, this->identifier);//func_name
		return 0;
	}
	else{
		// set is_called = 1
		DataSet->SetFuncCalled(this->identifier);//func_name
	}*/
	
	IntermediateNode * func_name_node = new IntermediateNode(ChainExpression::CALL,v,0,1);//func_var
	func_chain->AddNode(func_name_node);
	int i = 0;
	func_signature.clear();
	do{
		int type = 0;
		scanner->ReadNextToken(temp);
		if (*temp == ')' && i==0)
		{
			// this is call func(), get token 
			scanner->GetToken();
			break;
		}
		IntermediateNode * arg_node = new IntermediateNode(ChainExpression::ARG,0,0,1);
		arg_node->chain_left = PerformExp();
		SundType * new_st = new SundType();
		new_st->Copy(arg_node->chain_left->GetSundType());
		func_signature.push_back(new_st);
		if(arg_node->chain_left!=0)
		{
			func_chain->AddNode(arg_node);
		}
		scanner->GetToken();
		i++;
	}while(*(scanner->token)==',');
	
	if(*(scanner->token)!=')')
	{
		Err->Print(this->scannerState,ErrorPrinter::PAREN_EXPECTED);
		return 0;
	}
	
	switch (v->CheckFuncSignature(&func_signature)){
	case Var::NOT_ENOUGH_PAR:
		Err->AddError(ErrorPrinter::NOT_ENOUGH_PAR, this->identifier);
		break;
	case Var::TOO_MANY_PAR:
		Err->AddError(ErrorPrinter::TOO_MANY_PAR, this->identifier);
		break;
	default:
		break;
	}
	func_chain->result = new Var();
	std::list<sund_type_node *>::iterator iter = v->GetSundType()->sund_type_list.begin();
	/*if ((*iter)->type_code == SundCompiler::POINTER){
		//(*iter)->type_code = SundCompiler::POINTER_TO_FUNCTION;
		iter++;
	}*/
	// remove FUNCTION node
	v->GetSundType()->sund_type_list.erase(iter);

	func_chain->result->Copy(v);
	return func_chain;
}
// process array
ChainExpression * SundParser::PerformArray(wchar_t * array_name,ChainExpression * chain)
{
	ChainExpression * chain_result = new ChainExpression();
	Var * var_temp = new Var();
	//if(DataSet->GetVar(array_name,this->func_index,var_temp)!=0)
	if (DataSet->FindVar(array_name, this->func_name, var_temp) != 0)
	{
		Var * array_var = new Var();
		array_var->SetName(array_name);
		array_var->SetSpecifier(Var::ARRAY);
		IntermediateNode * array_name_node = new IntermediateNode(ChainExpression::ARRAY,0,0,1);
		array_name_node->chain_left = chain;
		// copy var to result also
		chain_result->result = new Var();
		chain_result->result->Copy(array_var);
		IntermediateNode * index_node = new IntermediateNode(ChainExpression::INDEX_OF,0,0,1);
		ChainExpression * chain = PerformExp();
		if(chain == 0)
		{
			Err->Print(this->scannerState,ErrorPrinter::SYNTAX);
		    return 0;
		}
		index_node->chain_left = chain;
		chain_result->AddNode(array_name_node);
		chain_result->AddNode(index_node);
		scanner->GetToken();
		if(*(scanner->token)!=']')
		{
			Err->Print(this->scannerState,ErrorPrinter::SYNTAX);
			return 0;
		}
		if (var_temp->GetSundType()->GetBottomTypeCode() == SundCompiler::ARRAY){
			var_temp->GetSundType()->sund_type_list.pop_front();
			chain_result->result->Copy(var_temp);
		}
		else{
			// print error
			Err->Print(ErrorPrinter::SUBSCRIPT_ON_NON_ARRAY);
			return 0;
		}
	}
	else
	{
		Err->Print(this->scannerState,ErrorPrinter::SYNTAX);
		return 0;
	}
	return chain_result; 
}
//postfix increment and decrement 
void SundParser::DoPostfixIncDec(Var * var)
{
	IntermediateNode * node = new IntermediateNode();
	node->n_operands = 1;
	wchar_t op = *(scanner->token);
	node->chain_left = 0;
	node->var_left = new Var();
	node->var_left->Copy(var);
	switch(op){
	case SundCompiler::INCREMENT:
			node->op = ChainExpression::INC;
			break;
	case SundCompiler::DECREMENT:
			node->op = ChainExpression::DEC;
			break;
	}
	postfix_chain->AddNode(node);
}
ChainExpression * SundParser::PerformSizeof()
{
	ChainExpression * result_chain = new ChainExpression();
	wchar_t buffer[80];
	int value = 0;
	// checks next token
	scanner->GetToken();
	if (scanner->token_type == SundCompiler::CAST){
		//value = 1;
		value = Var::GetSize(scanner->token);
		// sizeof(type)
		Var * cnst_var = new Var((wchar_t *)"",-1,Var::CONST);
		cnst_var->SetValue(value);
		swprintf(buffer,80, L"CONST_%d", value);
		cnst_var->SetName(buffer);
		result_chain->result = new Var();
		result_chain->result->Copy(cnst_var);
		scanner->GetToken();
		return result_chain;
	}
	if(*(scanner->token)!='('){
		Err->Print(this->scannerState,ErrorPrinter::SYNTAX);
		return 0;
	}
	// checks next token: expr 
	scanner->GetToken();
	if (scanner->token_type == SundCompiler::KEYWORD){
		switch(scanner->tok){
		case SundCompiler::CHAR:
		case SundCompiler::BYTE:
			value = 1;
			break;
		case SundCompiler::INT:
		case SundCompiler::WORD:
			value = 2;
			break;
		}
	}
	else
		if (scanner->token_type == SundCompiler::IDENTIFIER){
		Var * temp_var = new Var(); 
		//if(DataSet->GetVar(scanner->token,this->func_index,temp_var)!=0)
		if (DataSet->FindVar(scanner->token, this->func_name, temp_var) != 0)
		{
			value = temp_var->size;
		}
	}
	scanner->GetToken();
	
	if(*(scanner->token)!=')' || value == 0){
		Err->Print(this->scannerState,ErrorPrinter::SYNTAX_SIZEOF);
		return 0;
	}

	Var * cnst_var = new Var((wchar_t *)"",-1,Var::CONST);
	cnst_var->SetValue(value);
	swprintf(buffer,80, L"CONST_%d", value);
	cnst_var->SetName(buffer);
	result_chain->result = new Var();
	result_chain->result->Copy(cnst_var);
	scanner->GetToken();

	return result_chain;
}

int SundParser::H(wchar_t * s)
{
	int result = wcstol(s, NULL, 16);
	return result;
}
int SundParser::B(wchar_t * s)
{
	int length = (int)wcslen(s), sum = 0, k = 1;
	for(int i=0;i<length;i++) 
	{
		wchar_t ch = s[length - i - 1];
		int sign;
		if(ch == '1'){
			sign = 1;
		}
		else
		if(ch == '0'){
			sign = 0;
		}
		else
		{
			// temporary
			sign = 0;
		}
		sum = sum+sign*k;
		k = 2*k;
	}
	return sum;
}
int SundParser::Validate(wchar_t * s, int type)
{
	return 0;
}
int SundParser::GetRange(wchar_t * s)
{
	return 0;
}
char SundParser::GetCharacterValue(wchar_t * s)
{
	int len = (int)wcslen(s)-2;
	if (len>2){
		Err->AddError(ErrorPrinter::TOO_MANY_CHARS, s);
		return 0;
	}
	else
	if (len == 2){
		if (s[1] != '\\'){
			Err->AddError(ErrorPrinter::INVALID_CHAR_CONSTANT, s);
			return 0;
		}
		// white spaces
		switch (s[2]){
		case 't':
			return 0x09;
		case 'v':
			return 0x0b;
		case 'n':
			return 0x0a;
		case 'f':
			return 0x0c;
		case 'r':
			return 0x0d;
		default:
			Err->AddError(ErrorPrinter::INVALID_CHAR_CONSTANT, s);
			return 0;
		}
	}
	else return (char)s[1];
}
void SundParser::SetFuncName(wchar_t * func_name)
{
	wcscpy(this->func_name,func_name);
}



