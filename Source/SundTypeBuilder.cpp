#include "../Include/SundTypeBuilder.h"
#include "../Include/Scanner.h"
#include "../Include/ScannerState.h"
#include "../Include/SundCompiler.h"
#include "../Include/ErrorPrinter.h"


SundType::SundType()
{
	basic_type = -1;
	sund_type_list.clear();
}
SundType::SundType(int basic_type)
{
	this->basic_type = basic_type;
	sund_type_list.clear();
}
void SundType::Add(sund_type_node * type_node)
{ 
	sund_type_list.push_back(type_node); 
}
void SundType::AddToFront(sund_type_node * type_node)
{
	sund_type_list.push_front(type_node);
}
list<sund_type_node *> *  SundType::PushBack(list<sund_type_node *> * lst1, list<sund_type_node *> * lst2)
{
	list<sund_type_node *>::iterator it;
	for (it = lst2->begin(); it != lst2->end(); it++){
		lst1->push_back(*it);
	}
	return lst1;
}
SundType * SundType::Merge(SundType * sund_type1, SundType * sund_type2)
{
	if (sund_type1 == 0) 
		return sund_type2;
	if (sund_type2 == 0) 
		return sund_type1;

	PushBack(&(sund_type1->sund_type_list), &(sund_type2->sund_type_list));
	sund_type2->sund_type_list.clear();

	return sund_type1;
}
wchar_t * SundType::GetBasicType(int type)
{
	int result = -1;
	int basic_types[] = { SundCompiler::CHAR, SundCompiler::INT, SundCompiler::BYTE, SundCompiler::WORD, SundCompiler::BIT,SundCompiler::VOID };
	wchar_t * s_types[] = {L"char",L"int",L"byte",L"word",L"bit",L"void"};
	for (int i = 0; i < sizeof(basic_types) / sizeof(basic_types[0]); i++){
		if (type == basic_types[i]){
			result = i;
			return s_types[result];
		}
	}
	return 0;
}
wchar_t * SundType::GetTypeCode(int type_code, int array_size)
{
	static wchar_t temp[80];
	switch (type_code){
	case SundCompiler::POINTER:
		return L"pointer to ";
	case SundCompiler::ARRAY:
		if (array_size != -1){
			swprintf(temp, 80, L"array of %d", array_size);
			return temp;
		}
		return L"array ";
	case SundCompiler::FUNCTION:
		return L"function return ";
	default:
		swprintf(temp, 80, L"%ls", GetBasicType(type_code));
		return temp;
		//return 0;
	}
}
SundType * SundType::Copy(SundType * type)
{
	if (type == 0)
		return this;

	list<sund_type_node *>::iterator it;
	for (it = type->sund_type_list.begin(); it != type->sund_type_list.end(); it++)
	{
		sund_type_node * node = new sund_type_node();
		node->type_code = (*it)->type_code;
		node->array_size = (*it)->array_size;
		list<SundType *> * fs = (*it)->func_signature;
		if (fs != 0)
		{
			node->func_signature = new list<SundType *>();
			list<SundType *>::iterator fs_it;
			for (fs_it = fs->begin(); fs_it != fs->end();fs_it++)
			{
				SundType * new_st = new SundType();
				// recursion
				new_st->Copy(*fs_it);
				node->func_signature->push_back(new_st);
			}
		}

		this->Add(node);
	}
	this->basic_type = type->basic_type;
	return this;
}
int SundType::HasSubscript()
{
	int i = 0;
	list<sund_type_node *>::iterator it;
	for (it = this->sund_type_list.begin(); it != this->sund_type_list.end(); it++,i++){
		if (i && (*it)->array_size == -1 && (*it)->type_code == SundCompiler::ARRAY){
			return 0;
		}
	}
	return 1;
}
list<int> * SundType::GetArrayList()
{
	list<int> * result = new list<int>();
	list<sund_type_node *>::iterator it;
	for (it = this->sund_type_list.begin(); it != this->sund_type_list.end(); it++){
		if ((*it)->type_code == SundCompiler::ARRAY){
			result->push_back((*it)->array_size);
		}
	}
	return result;
}
int SundType::GetTopTypeCode()
{
	if (this->sund_type_list.size() == 0)
		return -1;

	list<sund_type_node *>::iterator it = this->sund_type_list.begin();
	return (*it)->type_code;
}
int SundType::GetBottomTypeCode()
{
	if (this->sund_type_list.size() == 0)
		return -1;

	list<sund_type_node *>::iterator it = this->sund_type_list.end();
	it--;
	return (*it)->type_code;
	//return ret;
}
void SundType::SetTopArraySize(int new_array_size)
{
	list<sund_type_node *>::iterator it = this->sund_type_list.begin();
	// now, change array_size
	(*it)->array_size = new_array_size;
}
int SundType::IsFunc()
{
	if (this->sund_type_list.size() == 0)
		return 0;

	list<sund_type_node *>::iterator it = this->sund_type_list.begin();
	if ((*it)->type_code == SundCompiler::FUNCTION)
		return 1;
	else return 0;
}
int SundType::IsPtrToFunc()
{
	if (this->sund_type_list.size() <= 1)
		return 0;

	int c = 0;
	list<sund_type_node *>::iterator it = this->sund_type_list.begin();
	if ((*it)->type_code == SundCompiler::POINTER){
		c++;
		it++;
	}
	if ((*it)->type_code == SundCompiler::FUNCTION)
		c++;
	if (c == 2)
		return 1;
	else return 0;
}
int SundType::IsEqual(SundType * type)
{
	list<sund_type_node *>::iterator it1;
	list<sund_type_node *>::iterator it2;
	if (this->sund_type_list.size() != type->sund_type_list.size())
		return SundType::NOT_EQUAL;
	for (it1 = this->sund_type_list.begin(), it2 = type->sund_type_list.begin(); it1 != this->sund_type_list.end();it1++,it2++){
		if ((*it1)->type_code != (*it2)->type_code)
			return 0;
		if ((*it1)->func_signature != 0){
			// func
			if ((*it2)->func_signature == 0)
				return SundType::NOT_EQUAL;
			if ((*it1)->func_signature->size() != (*it2)->func_signature->size())
				return SundType::NOT_EQUAL;
			list<SundType *>::iterator it_func1;
			list<SundType *>::iterator it_func2;
			for (it_func1 = (*it1)->func_signature->begin(),it_func2 = (*it2)->func_signature->begin(); it_func1 != (*it1)->func_signature->end(); it_func1++, it_func2++){
				// recursion
				if (!(*it_func1)->IsEqual(*it_func2))
					return SundType::NOT_EQUAL;
			}
		}
	}
	if ((this->basic_type == SundCompiler::BIT && type->basic_type != SundCompiler::BIT) || (this->basic_type != SundCompiler::BIT && type->basic_type == SundCompiler::BIT))
		return SundType::NOT_EQUAL;
	if (this->basic_type != type->basic_type)
		return SundType::CAST;

	return SundType::EQUAL;
}
void SundType::Print(FILE * file)
{
	for (list<sund_type_node *>::iterator it = this->sund_type_list.begin(); it != this->sund_type_list.end(); it++){
		sund_type_node * node = (sund_type_node *)(*it);
		fprintf(file, "%ls ", SundType::GetTypeCode(node->type_code, node->array_size));
		if (node->func_signature != 0){
			int i = 0;
			// print args
			fprintf(file, "\narguments: \n");
			for (list<SundType *>::iterator it_func = node->func_signature->begin(); it_func != node->func_signature->end(); it_func++,i++){
				 fprintf(file, "arg%d: ",i);
				 (*it_func)->Print(file);
				 fprintf(file, "\n");
			}
		}
	}
}
//class SundTypeBuilder def.
SundTypeBuilder::SundTypeBuilder(int level)
{
	this->sund_type = new SundType();
	this->level = level;
	this->var = 0;
}
SundTypeBuilder::SundTypeBuilder(int level, int type)
{
	this->sund_type = new SundType();
	this->level = level;
	this->type = type;
}
void SundTypeBuilder::AssignVar(Var * v)
{
	this->var = v;
}
// implements c declarator: entry point
SundType *  SundTypeBuilder::Build(Scanner * scanner)
{
	return PerformDecl(scanner);
}
SundType * SundTypeBuilder::PerformDecl(Scanner * scanner)
{
	SundType * sund_type_decl,* sund_type_suffix_decl;
	SundType * sund_type_pointer = new SundType();
	// perform pointer
	scanner->GetToken();
	while (*(scanner->token) == '*')
	{
		sund_type_node * type_node = new sund_type_node(SundCompiler::POINTER,-1);
		sund_type_pointer->Add(type_node);
		scanner->GetToken();
	}
	
	// direct-declarator 
	switch (scanner->token_type){
	case SundCompiler::IDENTIFIER:
		wcscpy(this->var->var_name, scanner->token);
		// perform suffix-declarator
		sund_type_suffix_decl = PerformSuffixDecl(scanner);
		return SundType::Merge(sund_type_suffix_decl, sund_type_pointer);
	case SundCompiler::DELIMITER:
		if (*scanner->token == '(')
		{
			//recursion
			sund_type_decl = PerformDecl(scanner);
			//scanner->GetToken();
			if (*scanner->token != ')'){
				// print error
				Err->Print(ErrorPrinter::PAREN_EXPECTED,scanner->token);
				return 0;
			}
			// perform suffix-declarator
			sund_type_suffix_decl = PerformSuffixDecl(scanner);
			return SundType::Merge(SundType::Merge(sund_type_decl,sund_type_suffix_decl), sund_type_pointer);
		}
		else{
			// print error
			Err->Print(ErrorPrinter::SYNTAX, scanner->token);
			return 0;
		}
		break;
	default:
		// print error
		Err->Print(ErrorPrinter::SYNTAX);
		return 0;
	}
}
SundType *  SundTypeBuilder::PerformSuffixDecl(Scanner * scanner)
{
	scanner->GetToken();
	if (*scanner->token == '(' || *scanner->token == '['){
		if (*scanner->token == '('){
			// function
			return PerformFunction(scanner);
		}
		else
		if (*scanner->token == '['){
			// array
			return PerformArray(scanner);
		}
	}
	return 0;
}
SundType * SundTypeBuilder::PerformFunction(Scanner * scanner)
{
	SundType * func_sund_type = new SundType();
	sund_type_node * func_node = new sund_type_node(SundCompiler::FUNCTION,-1);
	func_node->func_signature = new list<SundType *>();
	do{
		Var * var = new Var();
		SundTypeBuilder * builder = new SundTypeBuilder(SundTypeBuilder::PARAMS);
		// get basic type
		scanner->GetToken();
		int basic_type = scanner->tok;
		//sund_type_node * type_node = new sund_type_node(scanner->tok,-1);
		if (*scanner->token == ')'){
			break;
		}
		if (scanner->tok == SundCompiler::VOID){
			scanner->GetToken();
			if (*scanner->token == ')'){
				break;
			}
			else scanner->Putback();
		}
		builder->AssignVar(var);
		builder->sund_type = builder->Build(scanner);
		//builder->sund_type->Add(type_node);
		builder->sund_type->basic_type = basic_type;
		func_node->func_signature->push_back(builder->sund_type);
	} while (*(scanner->token) == ',');

	if (*(scanner->token) != ')'){
		Err->Print(ErrorPrinter::PAREN_EXPECTED);
	}
	// get ')'
	scanner->GetToken();
	func_sund_type->Add(func_node);
	return func_sund_type;
}
SundType * SundTypeBuilder::PerformArray(Scanner * scanner)
{
	SundType * array_sund_type = new SundType();
	sund_type_node * node;
	do{
		scanner->GetToken();
		switch (scanner->token_type){
		case SundCompiler::NUMBER:
			node = new sund_type_node(SundCompiler::ARRAY, _wtoi(scanner->token));
			array_sund_type->Add(node);
			scanner->GetToken();
			if (*scanner->token != ']'){
				// print error
				Err->Print(ErrorPrinter::SYNTAX, scanner->token);
				return 0;
			}
			scanner->GetToken();
			break;
		case SundCompiler::HEX_NUMBER:
			node = new sund_type_node(SundCompiler::ARRAY, wcstol(scanner->token, NULL, 16));
			array_sund_type->Add(node);
			scanner->GetToken();
			if (*scanner->token != ']'){
				// print error
				Err->Print(ErrorPrinter::SYNTAX, scanner->token);
				return 0;
			}
			scanner->GetToken();
			break;
		default:
			if (*scanner->token == ']'){
				node = new sund_type_node(SundCompiler::ARRAY, -1);
				array_sund_type->Add(node);
				scanner->GetToken();
			}
			break;
		}
	} while (*scanner->token == '[');
	
	return array_sund_type;
}

void SundTypeBuilder::Print(FILE * file)
{
	fprintf(file, "%ls: ", this->var->var_name);
	this->sund_type->Print(file);
	fprintf(file, "the basic type: %ls\n", SundType::GetBasicType(this->sund_type->basic_type));
}
SundTypeBuilder::~SundTypeBuilder()
{
}
