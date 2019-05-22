#include "../Include/ModuleParser.h"
#include "../Include/Module.h"
#include "../Include/PropertyVar.h"
#include "../ScannerState.h"
#include "../SundCompiler.h"
#include "../SundData.h"
#include "../IntermediateNode.h"
#include "../ChainExpression.h"
#include "../ErrorPrinter.h"


ModuleParser::ModuleParser(Scanner * scanner,Module * module)
:SundParser(scanner)
{
	this->module = module;
}
ChainExpression * ModuleParser::PerformModuleExp(ScannerState * state)
{
	scanner->GetToken();
	if (!*(scanner->token)){
		Err->Print(ErrorPrinter::NO_EXP);
		return 0;
	}
	if (*(scanner->token) == ';'){
		// empty expression
		return 0;
	}

	ChainExpression * chain_result = PerformBitwiseIOR();
	
	if (chain_result->IsEmpty() && chain_result->GetResult() == 0){
		return 0;
	}

	// return last token read to input stream
	scanner->Putback();

	return chain_result;
}
ChainExpression * ModuleParser::PerformAsgn()
{
	//return SundParser::PerformAsgn();

	PropertyVar prop;
	// holds name of var receiving the assigment
	wchar_t temp[ID_LEN];
	int temp_tok;
	wchar_t op;
	if (scanner->token_type == SundCompiler::IDENTIFIER)
	{
		
		// if a property var, see if assigment
		if (! module->IsPropertyVar(scanner->token))
		{
			wcscpy(temp, scanner->token);
			wcscpy(prop.var_name, scanner->token);
			temp_tok = scanner->token_type;
			//op = *(scanner->token);
			scanner->GetToken();
			op = *(scanner->token);
			ChainExpression * asgn_chain = new ChainExpression(), *chain;
			// is an assigment
			if (op == '=')
			{
				scanner->GetToken();
				PropertyVar * var = new PropertyVar();
				module->GetPropertyVarData(scanner->token, var);
				IntermediateNode * asgn_node = new IntermediateNode(ChainExpression::ASGN, var, 0);
				
				chain = PerformBitwiseIOR();// PerformAddSub();// PerformAsgn();
				prop.chain = chain;
				if (chain->result != 0){
					swprintf(prop.prop_value,80, L"%ls", chain->result->var_name);
					prop.value = chain->result->value;
				}
				module->properties.push_back(prop);
				asgn_node->chain_right = chain;
				
				asgn_chain->AddNode(asgn_node);
				return asgn_chain;
			}
			else
			{
				// not '=', case: property var without definition
				chain = 0;
				IntermediateNode * asgn_node = new IntermediateNode(ChainExpression::ASGN, 0, 0);
				prop.chain = chain;
				module->properties.push_back(prop);
				asgn_node->chain_right = chain;
				asgn_chain->AddNode(asgn_node);
				return asgn_chain;
			}
		}
		else{
			// exist property var: print error
			Err->Print(ErrorPrinter::MODULE_PROPERTY_VAR_REDEFINITION, scanner->token);
			return 0;
		}
	}
	else{
		// not identifier,print error
		Err->Print(ErrorPrinter::MODULE_PROPERTY_SYNTAX, scanner->token);
		return 0;
	}
}
ChainExpression * ModuleParser::PerformBitwiseAND()
{
	ChainExpression * chain_left = 0, *chain_right = 0;
	ChainExpression * chain_result = new ChainExpression();
	chain_result->kind = ChainExpression::BITWISE_AND;
	wchar_t op;
	chain_left = PerformAddSub(); //PerformRelOp();
	op = *(scanner->token);
	while ((op = *(scanner->token)) == '&')
	{
		IntermediateNode * node = new IntermediateNode();
		Var * im_result = 0;
		scanner->GetToken();
		chain_right = PerformAddSub(); //PerformRelOp();
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
			//delete im_result;
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
		if (!chain_result->ChangeConstantVar(node) || !Compiler->GetConstantFoldingEnabled())
		{
			chain_result->AddNode(node);
		}
	}

	if (chain_result->IsEmpty()){
		delete chain_result;
		return chain_left;
	}
	else{
		chain_result->ReduceConstants();
		return chain_result;
	}
}

// multiple or divide two factors
ChainExpression * ModuleParser::PerformMultDiv()
{
	ChainExpression * chain_left = 0, *chain_right = 0;
	ChainExpression * chain_result = new ChainExpression();
	wchar_t op;
	chain_left = PerformPrimaryExp(); //PerformUnaryOp();//PerformUnaryPlusMinus();
	int is_mult, is_div;
	ScannerState * s = new ScannerState(scanner);
	while ((op = *(scanner->token)) == '*' || op == '/' || op == '%')
	{
		IntermediateNode * node = new IntermediateNode();
		Var * im_result = 0;
		is_mult = 0;
		is_div = 0;
		scanner->GetToken();
		chain_right = PerformPrimaryExp(); //PerformUnaryOp();//PerformUnaryPlusMinus();
		switch (op) //  multiple or divide or modulus
		{
		case '*':
			node->op = ChainExpression::MULT;
			is_mult = 1;
			break;
		case '/':
			node->op = ChainExpression::DIV;
			is_div = 1;
			break;
		case '%':
			node->op = ChainExpression::MODULUS;
			break;
		}
		if (chain_result->GetChain()->size() == 0){
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
				Err->Print(s, ErrorPrinter::DIV_BY_ZERO);
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
			Err->Print(s, ErrorPrinter::DIV_BY_ZERO);
			return 0;
		}
		// constant folding
		if (!chain_result->ChangeConstantVar(node) || !Compiler->GetConstantFoldingEnabled())
		{
			chain_result->AddNode(node);
		}
	}
	if (chain_result->IsEmpty()){
		delete chain_result;
		return chain_left;
	}
	else{
		chain_result->ReduceConstants();
		return chain_result;
	}
}

ChainExpression * ModuleParser::PerformPrimaryExp()
{
	//return SundParser::PerformPrimaryExp();
	ChainExpression * r_chain = new ChainExpression();
	//Var * temp_right_op = new Var();
	PropertyVar * temp_right_op = new PropertyVar();
	Var * cnst_var;
	wchar_t name[80], buffer[80];
	char char_value;
	//int d_result, f_result;
	int prop_result;
	switch (scanner->token_type){
	case SundCompiler::IDENTIFIER:
		wcscpy(name, scanner->token);
		//prop_result = module->IsPropertyVar(name);
		prop_result = module->GetPropertyVarData(name, temp_right_op);
		if (prop_result == 0){
			// print error
			Err->Print(ErrorPrinter::MODULE_UNDEFINED_VAR, name);
			return 0;
		}
		r_chain->result = new Var();
		r_chain->result->Copy(temp_right_op);

		scanner->GetToken();
		return r_chain;
	case SundCompiler::NUMBER:
		cnst_var = new Var((wchar_t *)"", -1, Var::CONST);
		cnst_var->SetValue(_wtoi(scanner->token));
		swprintf(buffer, 80, L"CONST_%s", scanner->token);
		cnst_var->SetName(buffer);
		r_chain->result = new Var();
		r_chain->result->Copy(cnst_var);
		scanner->GetToken();
		return r_chain;
		case SundCompiler::HEX_NUMBER:
		cnst_var = new Var((wchar_t *)"", -1, Var::CONST);
		cnst_var->SetValue(H(scanner->token));
		swprintf(buffer, 80, L"CONST_0x%s", scanner->token);
		cnst_var->SetName(buffer);
		r_chain->result = new Var();
		r_chain->result->Copy(cnst_var);
		scanner->GetToken();
		return r_chain;
		case SundCompiler::B_NUMBER:
		cnst_var = new Var((wchar_t *)"", -1, Var::CONST);
		cnst_var->SetValue(B(scanner->token));
		swprintf(buffer, 80, L"CONST_0b%s", scanner->token);
		cnst_var->SetName(buffer);
		r_chain->result = new Var();
		r_chain->result->Copy(cnst_var);
		scanner->GetToken();
		return r_chain;
		case SundCompiler::CHARACTER:
		cnst_var = new Var((wchar_t *)"", -1, Var::CONST);
		char_value = GetCharacterValue(scanner->token);
		cnst_var->SetValue(char_value);//*(scanner->token+1)
		//swprintf(buffer,80, L"CONST_'%c'", *(scanner->token + 1));
		swprintf(buffer, 80, L"CONST_%s", scanner->token);
		cnst_var->SetName(buffer);
		r_chain->result = new Var();
		r_chain->result->Copy(cnst_var);
		scanner->GetToken();
		return r_chain;
		case SundCompiler::DELIMITER:
		// process parenthezis expression eg. (expr)
		if ((*(scanner->token) == '('))
		{
			ChainExpression * chain = 0;
			scanner->GetToken();
			chain = PerformBitwiseIOR();//PerformAddSub();//PerformAsgn();
			if (*(scanner->token) != ')'){
				Err->Print(this->scannerState, ErrorPrinter::PAREN_EXPECTED);
			}
			scanner->GetToken();
			return chain;
		}
		Err->Print(this->scannerState, ErrorPrinter::UNSUP_EXPR);
		return 0;
	default:
		if (*(scanner->token) == ')'){
			return r_chain;
		}
		else{
			Err->Print(this->scannerState, ErrorPrinter::SYNTAX);
			return r_chain;
		}
	}
}
ChainExpression * ModuleParser::PerformCallFunc()
{
	return SundParser::PerformCallFunc();
}
ModuleParser::~ModuleParser()
{
}
