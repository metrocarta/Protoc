#include "../Include/InstructionParser.h"
#include "../Include/ModuleParser.h"
#include "../Include/Module.h"
#include "../Include/ModuleVar.h"
#include "../Include/PropertyVar.h"
#include "../Scanner.h"
#include "../ScannerState.h"
#include "../SundCompiler.h"
#include "../SundData.h"
#include "../SundTypeBuilder.h"
#include "../IntermediateNode.h"
#include "../ChainExpression.h"
#include "../ErrorPrinter.h"
#include "../Var.h"
#include "../ProgramArgumentsParser.h"
#include "../Include/ISA.h"





InstructionParser::InstructionParser(Scanner * scanner, Module * module)
:SundParser(scanner)
{
	this->module = module;
}
ChainExpression * InstructionParser::PerformInstructionExp(ScannerState * state)
{
	scanner->GetToken();
	if (!*(scanner->token)) {
		Err->Print(ErrorPrinter::NO_EXP);
		return 0;
	}
	if (*(scanner->token) == ';') {
		// empty expression
		return 0;
	}

	//ChainExpression * chain_result = PerformBitwiseIOR();
	ChainExpression * chain_result = PerformAddSub();

	if (chain_result->IsEmpty() && chain_result->GetResult() == 0) {
		return 0;
	}

	// return last token read to input stream
	scanner->Putback();

	return chain_result;
}
ChainExpression * InstructionParser::PerformBitwiseIOR()
{
	ChainExpression * chain_left = 0, *chain_right = 0;
	ChainExpression * chain_result = new ChainExpression();
	chain_result->kind = ChainExpression::BITWISE_IOR;
	wchar_t op;
	chain_left = PerformBitwiseXOR();
	op = *(scanner->token);
	while ((op = *(scanner->token)) == '|')
	{
		IntermediateNode * node = new IntermediateNode();
		Var * im_result = 0;
		scanner->GetToken();
		chain_right = PerformBitwiseXOR();
		node->op = ChainExpression::OR;
		if (chain_result->GetChain()->size() == 0) {
			int v1, v2;
			if (chain_left->IsConst(&v1) && chain_right->IsConst(&v2))
			{
				if (Compiler->GetConstantFoldingEnabled()) {
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
		else {
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
		switch (node->CheckTypes()) {
		case SundType::NOT_EQUAL:
			Err->AddError(ErrorPrinter::OPERANDS_HAVE_ILLEGAL_TYPES, L"|");
			break;
		case SundType::CAST:
			break;
		}
	}

	if (chain_result->IsEmpty()) {
		delete chain_result;
		return chain_left;
	}
	else {
		return chain_result->ReduceConstants();
	}
}
// add or subcract two terms
ChainExpression * InstructionParser::PerformAddSub()
{
	//int value1, value2;
	ChainExpression * chain_left = 0, *chain_right = 0;
	ChainExpression * chain_result = new ChainExpression();
	chain_result->kind = ChainExpression::ADD_SUB;
	wchar_t op;

	chain_left = PerformMultDiv();
	op = *(scanner->token);
	while ((op = *(scanner->token)) == '+' || op == '-')
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
		if (chain_result->GetChain()->size() == 0) {
			int v1, v2;
			if (chain_left->IsConst(&v1) && chain_right->IsConst(&v2))
			{
				if (Compiler->GetConstantFoldingEnabled()) {
					// update chain_left, when constant folding is enabled
					int res_value = ChainExpression::Calculate(v1, v2, node->op);
					chain_left->result->SetValue(res_value);
					chain_left->result->SetVarName(res_value);
					delete node;
					continue;
				}
			}
			else
				if (chain_left->IsZeroConstant() && !chain_right->IsConst(&v2)) {
					// left operand = 0, right is not constant
					delete chain_left;
					chain_left = chain_right;
					continue;
				}
			node->chain_left = chain_left;
			im_result = 0;
		}
		else {
			node->chain_left = 0;
			im_result = new Var();
			im_result->SetName((wchar_t *)L"ir");
			node->var_left = new Var();
			node->var_left->Copy(im_result);
			//delete im_result;
		}
		node->chain_right = chain_right;
		if (chain_right->IsZeroConstant()) {
			// op+0
			delete chain_right;
			continue;
		}
		// build node result_type
		node->BuildResultType();
		// constant folding
		if (!chain_result->ChangeConstantVar(node) || !Compiler->GetConstantFoldingEnabled()) {
			chain_result->AddNode(node);
		}
	}
	if (chain_result->IsEmpty()) {
		delete chain_result;
		return chain_left;
	}
	else {
		return chain_result->ReduceConstants();
	}
}

// multiple or divide two factors
ChainExpression * InstructionParser::PerformMultDiv()
{
	ChainExpression * chain_left = 0, *chain_right = 0;
	ChainExpression * chain_result = new ChainExpression();
	chain_result->kind = ChainExpression::MULT_DIV;
	wchar_t op;
	wchar_t s_op[3];
	//chain_left = PerformUnaryOp();//PerformUnaryPlusMinus();
	chain_left = PerformPostfixExp();// PerformPrimaryExp();
	int is_mult, is_div;
	ScannerState * s = new ScannerState(scanner);
	while ((op = *(scanner->token)) == '*' || op == '/' || op == '%')
	{
		IntermediateNode * node = new IntermediateNode();
		Var * im_result = 0;
		is_mult = 0;
		is_div = 0;
		scanner->GetToken();
		chain_right = PerformPostfixExp();// PerformPrimaryExp();//		PerformUnaryOp();//PerformUnaryPlusMinus();
		switch (op) //  multiple or divide or modulus
		{
		case '*':
			node->op = ChainExpression::MULT;
			is_mult = 1;
			wcscpy(s_op, L"*");
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
		if (chain_result->GetChain()->size() == 0) {
			int v1, v2;
			if (is_mult && (chain_left->IsZeroConstant() || chain_right->IsZeroConstant())) {
				//dead expr : value*0 or 0*value or 0*0
				chain_left->result->SetQualifier(Var::CONST);
				chain_left->result->SetValue(0);
				chain_left->result->SetVarName(0);
				delete node;
				continue;
			}
			if (is_div && chain_right->IsZeroConstant()) {
				Err->Print(s, ErrorPrinter::DIV_BY_ZERO);
				return 0;
			}
			if (chain_left->IsConst(&v1) && chain_right->IsConst(&v2))
			{
				if (Compiler->GetConstantFoldingEnabled()) {
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
		else {
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
		if (is_div && chain_right->IsZeroConstant()) {
			Err->Print(s, ErrorPrinter::DIV_BY_ZERO);
			return 0;
		}
		// build node result_type
		node->BuildResultType();
		// constant folding
		if (!chain_result->ChangeConstantVar(node) || !Compiler->GetConstantFoldingEnabled())
		{
			chain_result->AddNode(node);
		}
		switch (node->CheckTypes()) {
		case SundType::NOT_EQUAL:
			Err->AddError(ErrorPrinter::OPERANDS_HAVE_ILLEGAL_TYPES, s_op);
			break;
		case SundType::CAST:
			break;
		}
	}

	if (chain_result->IsEmpty()) {
		delete chain_result;
		return chain_left;
	}
	else {
		return chain_result->ReduceConstants();
		//return chain_result;
	}
}
ChainExpression * InstructionParser::PerformPostfixExp()
{
	wchar_t temp[80];
	wchar_t op;
	wcscpy(temp, scanner->token);

	ChainExpression * chain_result = PerformPrimaryExp();
	if ((op = *(scanner->token)) == '[' || (op = *(scanner->token)) == SundCompiler::INCREMENT || (op = *(scanner->token)) == SundCompiler::DECREMENT || (op = *(scanner->token)) == '(') {
		switch (op) {
		case '[':
			chain_result = PerformArray(temp, chain_result);
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
			if (postfix_chain == 0)
			{
				// create ChainExpression object
				postfix_chain = new ChainExpression();
			}
			DoPostfixIncDec(temp_right_op);
			scanner->GetToken();
			break;
		}

	}

	return chain_result;
}
ChainExpression * InstructionParser::PerformArray(wchar_t * array_name, ChainExpression * chain)
{
	ChainExpression * chain_result = new ChainExpression();
	Var * var_temp = new Var();
	int var_result = module->GetModuleVarData(array_name, (ModuleVar*)var_temp);
	ISA * isa = ISA::CreateISAObject(ProgramArgumentsParser::GetInstance()->GetPlatform());
	int op_code_type = isa->GetOpCodeGroupIndex(array_name);
	if (var_result != 0 || op_code_type)
	{
		Var * array_var = new Var();
		array_var->SetName(array_name);
		array_var->SetSpecifier(Var::ARRAY);
		IntermediateNode * array_name_node = new IntermediateNode(ChainExpression::ARRAY, 0, 0, 1);
		array_name_node->chain_left = chain;
		// copy var to result also
		chain_result->result = new Var();
		chain_result->result->Copy(array_var);
		IntermediateNode * index_node = new IntermediateNode(ChainExpression::INDEX_OF, 0, 0, 1);
		ChainExpression * chain = PerformInstructionExp(new ScannerState(scanner));// PerformExp();
		if (chain == 0)
		{
			Err->Print(this->scannerState, ErrorPrinter::SYNTAX);
			return 0;
		}
		index_node->chain_left = chain;
		chain_result->AddNode(array_name_node);
		chain_result->AddNode(index_node);
		scanner->GetToken();
		if (*(scanner->token) != ']')
		{
			Err->Print(new ScannerState(scanner), ErrorPrinter::SYNTAX);
			return 0;
		}
		if (var_temp->GetSpecifier() == Var::ARRAY || op_code_type) {
			//var_temp->GetSundType()->sund_type_list.pop_front();
			chain_result->result->Copy(var_temp);
		}
		else {
			// print error
			Err->Print(new ScannerState(scanner),ErrorPrinter::SUBSCRIPT_ON_NON_ARRAY);
			return 0;
		}
	}
	else
	{
		Err->Print(new ScannerState(scanner), ErrorPrinter::SYNTAX);
		return 0;
	}
	return chain_result;
}

ChainExpression * InstructionParser::PerformPrimaryExp()
{
	ISA * isa = ISA::CreateISAObject(ProgramArgumentsParser::GetInstance()->GetPlatform());
	ChainExpression * r_chain = new ChainExpression();
	//Var * temp_right_op = new Var();
	PropertyVar * temp_right_op1 = new PropertyVar();
	ModuleVar * temp_right_op2 = new ModuleVar();
	//label_type temp_label;
	Var * cnst_var;
	wchar_t name[80], buffer[80];
	char char_value;
	//int d_result, f_result;
	int prop_result,var_result,label_result;
	int op_code_type = isa->GetOpCodeGroupIndex(scanner->token);
	switch (scanner->token_type) {
	case SundCompiler::IDENTIFIER:
		wcscpy(name, scanner->token);
		prop_result = module->GetPropertyVarData(name, temp_right_op1);
		var_result = module->GetModuleVarData(name, temp_right_op2);
		label_result = module->IsExistLabel(name);
		//op_code_type = isa->GetOpCodeGroupIndex(name);
		if (prop_result == 0 && var_result == 0 && label_result == 0 && op_code_type == 0)
		{
			// print error
			Err->Print(new ScannerState(scanner),ErrorPrinter::MODULE_UNDEFINED_VAR, name);
			return 0;
		}
		r_chain->result = new Var();
		if(prop_result) {
			r_chain->result->Copy(temp_right_op1);
		}
		else
		if(var_result)
		{
			r_chain->result->Copy(temp_right_op2);
		}
		else
		if(op_code_type)
		{
			Var * temp = new Var();
			temp->SetName(name);
			r_chain->result->Copy(temp);
		}

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
	case SundCompiler::KEYWORD:
		//r_chain->result = new Var();
		Err->Print(new ScannerState(scanner), ErrorPrinter::MODULE_VAR_IS_KEYWORD,scanner->token);
		return 0;
	case SundCompiler::DELIMITER:
		// process parenthezis expression eg. (expr)
		if ((*(scanner->token) == '('))
		{
			ChainExpression * chain = 0;
			scanner->GetToken();
			chain = PerformBitwiseIOR();//PerformAddSub();//PerformAsgn();
			if (*(scanner->token) != ')') {
				Err->Print(new ScannerState(scanner), ErrorPrinter::PAREN_EXPECTED);
			}
			scanner->GetToken();
			return chain;
		}
		Err->Print(new ScannerState(scanner), ErrorPrinter::UNSUP_EXPR);
		return 0;
	default:
		if (*(scanner->token) == ')') {
			return r_chain;
		}
		else {
			Err->Print(new ScannerState(scanner), ErrorPrinter::SYNTAX);
			return r_chain;
		}
	}
}


InstructionParser::~InstructionParser()
{
}
