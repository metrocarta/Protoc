#include "../Include/SundCompiler.h"
#include "../Include/ScannerState.h"
#include "../Include/SundParser.h"
#include "../Include/ChainExpression.h"
#include "../Include/ErrorPrinter.h"
#include "../Include/IfControl.h"
#include "../Include/SwitchControl.h"
#include "../Include/ForControl.h"
#include "../Include/WhileControl.h"
#include "../Include/DoControl.h"
#include "../Include/SundData.h"
#include "../Include/SundTypeBuilder.h"
#include "../Include/SundInitializer.h"
#include "../Module/Include/Module.h"

SundCompiler * SundCompiler::instance = NULL;

SundCompiler::SundCompiler(void)
{
	Init();

	// create a Scanner object
	scanner = new Scanner();
	//parser = new Parser(scanner);
	sundParser = new SundParser(scanner);
	//preprocessor = new Preprocessor();

	instance = this;
}
SundCompiler::SundCompiler(Scanner * scanner,SundParser * sundParser)
{
	Init();

	// create a Scanner object
	this->scanner = scanner;
	//this->parser = parser;
	this->sundParser = sundParser;
	//preprocessor = new Preprocessor();

	instance = this;
}
void SundCompiler::Init()
{
	struct commands tmp_table[]={L"if",IF,L"else",ELSE,L"for",FOR,L"do",DO,L"while",WHILE,L"switch",SWITCH,
								L"case", CASE, L"default", DEFAULT, L"org",ORG,
								L"try",TRY,L"throw",THROW,L"catch",CATCH,
								L"char", CHAR, L"byte", BYTE, L"bit", BIT, L"int", INT, L"word", WORD, L"pointer", POINTER,L"p_function",POINTER_TO_FUNCTION, L"array", ARRAY, L"function",FUNCTION,
								L"goto",GOTO,L"break",BREAK,L"continue",CONTINUE,L"return",RETURN,
								L"void", VOID, L"label", LABEL, L"inline", INLINE, L"static", STATIC, L"auto", AUTO, L"register", REGISTER,
								L"const", CONST, L"volatile",VOLATILE,
								L"module", MODULE,
								L"property", PROPERTY, L"extends",EXTENDS,L"device",DEVICE,L"code", CODE, L"endc", ENDC, L"var", VAR, L"macro", MACRO, L"endm",ENDM,
								L"block", CODE_BLOCK, L"endb", ENDB,L"lib",LIB,L"endlib",ENDLIB, L"sizeof", SIZEOF, L"import", IMPORT, L"end", END };

	int length1 = sizeof(tmp_table)/sizeof(tmp_table[0]);
	table = new struct commands[length1];
	for(int i=0;i<length1;i++)
	{
		wcscpy((table + i)->command, tmp_table[i].command);
		(table+i)->tok = tmp_table[i].tok;
	}
	
	stmt_id = 0;
	global_statement = 0;
	main_program = 0;
	constant_folding_enabled =  TRUE;
    propagation_enabled = FALSE;
}
SundCompiler::~SundCompiler(void)
{
}
SundCompiler * SundCompiler::GetInstance()
{
	return instance;
}
wchar_t * SundCompiler::GetCode()
{
	return scanner->prog;
}
wchar_t * SundCompiler::GetProgramBuffer()
{
	return scanner->p_buf;
}
void SundCompiler::Run(wchar_t  *fname)
{
	wchar_t * ppc_fname = L"out.pre";
	FILE *outp_file = _wfopen(ppc_fname, L"w");
	WriteBOM(outp_file);
	preprocessor = new Preprocessor(fname, ppc_fname, outp_file);//fname
	int result = preprocessor->Execute(fname, ppc_fname);
	if (result == 0)
	{
		Err->Print(Err->CANNOT_OPEN_FILE,fname);
		return;
	}
	fclose(outp_file);
	delete preprocessor;
    
	FILE * inp_file = _wfopen(ppc_fname, L"r");//L"out.pre"
	if (inp_file == NULL)
		return ;
	// obtain file size:
	fseek(inp_file, 0, SEEK_END);
	int f_size = ftell(inp_file);
	rewind(inp_file);

	wchar_t *p_buf = new wchar_t[f_size];
	wchar_t wc;
	int i = 0;
	do {
		wc = fgetwc(inp_file);
		p_buf[i] = wc;
		++i;
	} while (wc != WEOF);
	p_buf[i-1] = '\0';
	// close inp_file
	fclose(inp_file);

	int bom_offset = 0;
	if (Preprocessor::HasBOM(p_buf))
	{
		// the input file consists BOM signature (0xEF 0xBB 0xBF)
		bom_offset = 3;
	}

	wchar_t *prog = p_buf;
	scanner->prog = prog+bom_offset;
	scanner->p_buf=p_buf+bom_offset;

	//Tokenize();
	Compile();

	delete p_buf;
}
void SundCompiler::Tokenize()
{
	wchar_t * p;
	p=scanner->prog;
	do{
		scanner->GetToken();
		/*if(scanner->token_type==CAST){
			int s = 9;
		}*/
		scanner->Print();
	}while(scanner->tok!=FINISHED);
}
int SundCompiler::IsStorageClass(int type)
{
	int storage_types[] = { STATIC,AUTO,REGISTER };
	for (int i = 0; i<sizeof(storage_types) / sizeof(storage_types[0]); i++)
	{
		if (type == storage_types[i]){
			return 1;
		}
	}
	return 0;
}
int SundCompiler::IsTypeQualifier(int qualifier)
{
	int types[] = { CONST,VOLATILE };
	for (int i = 0; i<sizeof(types) / sizeof(types[0]); i++)
	{
		if (qualifier == types[i]){
			return 1;
		}
	}
	return 0;
}
int SundCompiler::IsRegularType(int type)
{
	int regular_types[] = {CHAR,INT,BYTE,WORD,BIT}; 
	for(int i=0;i<sizeof(regular_types)/sizeof(regular_types[0]);i++)
	{
		if(type==regular_types[i]){
			return 1;
		}
	}
	return 0;
}
int SundCompiler::IsQualifier(int qualifier)
{
	if(qualifier==INLINE || qualifier==STATIC){
	   return 1;
	}	
	return 0;
}
void SundCompiler::DeclLabel(wchar_t * name, wchar_t * func_name)
{
	// label definition
	struct label_type label_node;
	wchar_t * temp = scanner->prog;
	wcscpy(label_node.label_name, name);
	wcscpy(label_node.func_name, func_name);
	label_node.start = ++temp;
	if(DataSet->IsLabel(name)) {
		// print error
		Err->Print(ErrorPrinter::REDEFINED_LABEL,name);
		return;
	}
	// add label to list
	DataSet->labels.push_back(label_node);
}
void SundCompiler::DeclFunction(int type, int qualifier, int storage_class, int type_qualifier)
{	
	ScannerState  *s0 = new ScannerState(scanner);
	Var * func_var = new Var();
	SundTypeBuilder * builder = new SundTypeBuilder(SundTypeBuilder::GLOBAL, type);
	builder->AssignVar(func_var);
	builder->sund_type = builder->Build(scanner);
	
	Var * v = DataSet->GetGlobalVar(func_var->var_name);
	if (v == 0 || (v != 0 && v->is_defined==0))
	{
		int brace = 1;
		wchar_t temp[80];
		// add func_var to vars 
		func_var->basic_type = type;
		func_var->sund_type = builder->sund_type;
		func_var->value = 0;
		func_var->size = Var::GetSize(scanner->tok);
		func_var->level = Var::GLOBAL;
		func_var->SetStorageClass(storage_class);
		func_var->SetQualifier(type_qualifier);
		func_var->is_defined = 0;// 1;
		if (v == 0){
		   DataSet->vars.push_front(func_var);
		}
		else{
			DataSet->SetFuncDefined(func_var->var_name);
		}
		scanner->Revert(*s0);
		scanner->GetToken();
		while (*scanner->token == '*'){
			scanner->GetToken();
		}
		
		scanner->GetToken();
		PushParams(func_var->var_name);
		ScannerState  *s1 = new ScannerState(scanner);
		sundParser->SetFuncName(func_var->var_name);
		scanner->GetToken();
		// bypass code inside func. body
		while (brace)
		{
			wcscpy(temp, scanner->token);
			scanner->GetToken();
			// deadlock prevention
			if (scanner->tok == FINISHED){
				// print error msg
				Err->Print(ErrorPrinter::SYNTAX);
			}
			if (*(scanner->token) == ':'){
				scanner->Putback();
				DeclLabel(temp, func_var->var_name);
				scanner->GetToken();
			}

			if (*(scanner->token) == '{'){
				brace++;
			}
			if (*(scanner->token) == '}'){
				brace--;
			}
		}
		ScannerState  *s2 = new ScannerState(scanner);
		scanner->Revert(*s1);
		// get next token ( '{' )
		scanner->GetToken();
		if (*(scanner->token) == '{'){
			ProgramControlBase * program_control = new ProgramControlBase(ProgramControlBase::FUNC, func_var->var_name);
			DoStatement(program_control, 1, func_var->var_name);
			this->main_program->Add(program_control);
		}
		scanner->Revert(*s2);
	}
	else 
	{
		if (v->is_defined)
		{
		    Err->Print(s0, ErrorPrinter::FUNC_REDEFINITION, func_var->var_name);
		    return;
		}
	}
}
int SundCompiler::IsClassOrStructOrUnion(wchar_t *t)
{
	if (!wcscmp(t, L"class"))
		return 1;
	if (!wcscmp(t, L"struct"))
		return 1;
	if (!wcscmp(t, L"union"))
		return 1;

	return 0;
}
void SundCompiler::Compile(void)
{
	wchar_t * p;
	p=scanner->prog;
	int qualifier = 0;
	do{
		scanner->GetToken();
		if (scanner->tok == MODULE){
		    // handles 'module' statement: machine dependent code
			Module * module = new Module(scanner);
			ProgramControlBase * module_control = module->DoStatement();
			if (module_control != 0)
			{
				this->global_statement->Add(module_control);
			}
			continue;
		}

		if(IsQualifier(scanner->tok)){
			qualifier = scanner->tok;
			scanner->GetToken();
		}
		//is storage class: STATIC
		int storage_class = -1;
		if (scanner->tok == STATIC){
			storage_class = scanner->tok;
			scanner->GetToken();
		}
		// is type qualifier: CONST or VOLATILE
		int type_qualifier = -1;
		if (IsTypeQualifier(scanner->tok)){
			type_qualifier = scanner->tok;
			scanner->GetToken();
		}
		if(IsRegularType(scanner->tok) || scanner->tok==VOID){
			ScannerState * temp = new ScannerState(scanner);
			if (IsFuncDef()){
				scanner->Revert(*temp);
				DeclFunction(scanner->tok, qualifier, storage_class, type_qualifier);
				continue;
			}
			else{
				scanner->Revert(*temp);
				scanner->Putback();
				// decl. vars,function,arrays...
				DeclGlobal(scanner->tok, qualifier, storage_class, type_qualifier);
			}
		}
		else
		if(scanner->tok!=FINISHED)
		{
			// wrong type...print error msg.
			Err->Print(ErrorPrinter::TYPE_OR_QUALIFIER_EXPECTED,scanner->token);
			return;
		}
	}while(scanner->tok!=FINISHED);

	// print unresolved extern functions!
	DataSet->PrintUndefinedFunc();

	scanner->prog=p;
}
int SundCompiler::IsFuncDef()
{
	scanner->GetToken();
	while (*scanner->token == '*'){
		scanner->GetToken();
	}
	if (scanner->token_type != IDENTIFIER)
		return 0;
	scanner->GetToken();
	if (*scanner->token != '(')
		return 0;
	// bypass args
	BypassFuncArgs();
	scanner->GetToken();
	if (*scanner->token == '{')	
		return 1;

	return 0;
}

void SundCompiler::DeclGlobal(int type, int qualifier, int storage_class,int type_qualifier)
{
	scanner->GetToken();
	do{
		Var * global_var = new Var();
		//sund_type_node * type_node = new sund_type_node(type, -1);
		SundTypeBuilder * builder = new SundTypeBuilder(SundTypeBuilder::GLOBAL,type);
		builder->AssignVar(global_var);
		builder->sund_type = builder->Build(scanner);
		//builder->sund_type->Add(type_node);
		builder->sund_type->basic_type = type;
		global_var->basic_type = type;
		global_var->sund_type = builder->sund_type;
		global_var->value = 0;
		global_var->size = Var::GetSize(scanner->tok);
		global_var->level = Var::GLOBAL;
		global_var->SetStorageClass(storage_class);
		global_var->SetQualifier(type_qualifier);

		if (!builder->sund_type->HasSubscript()){
			// print error
			Err->Print(ErrorPrinter::MISSING_SUBSCRIPT, global_var->var_name);
			return;
		}
		if (*scanner->token == '='){
			SundInitializer * initializer = new SundInitializer(scanner, sundParser, builder->sund_type);
			initializer->Build();
			ProgramControlBase * init_list_control = new ProgramControlBase(ProgramControlBase::INITIALIZER, builder,initializer->GetInitListSize(),initializer->GetInitList());
			this->global_statement->Add(init_list_control);
		}
		else{
			ProgramControlBase * decl_control = new ProgramControlBase(ProgramControlBase::DECL, builder);
			this->global_statement->Add(decl_control);
		}
		// verify global_var
		if (DataSet->IsGlobalVar(global_var->var_name)){
			Err->AddError(ErrorPrinter::GLOBAL_VAR_REDEFINITION, global_var->var_name);
		}
		else{
			DataSet->vars.push_front(global_var);
		}
		
	}while(*(scanner->token)==',');

	if(*(scanner->token)!=';'){
		Err->Print(ErrorPrinter::SEMI_EXPECTED);
	}
}
void SundCompiler::PushParams(wchar_t * func_name)
{
	do{
		Var * var = new Var();
		SundTypeBuilder * builder = new SundTypeBuilder(SundTypeBuilder::PARAMS);
		scanner->GetToken();
		sund_type_node * type_node = new sund_type_node(scanner->tok, -1);
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
		builder->sund_type->basic_type = type_node->type_code;
		var->basic_type = scanner->tok;
		var->level = Var::ARG;
		var->sund_type = builder->sund_type;
		wcscpy(var->func_name, func_name);
		DataSet->vars.push_front(var);
	}while(*(scanner->token)==',');

	if(*(scanner->token)!=')'){
		Err->Print(ErrorPrinter::PAREN_EXPECTED);
	}
}
void SundCompiler::DeclLocal(ProgramControlBase *program_control, int storage_class, int type_qualifier, wchar_t * func_name)
{
	//int is_pointer = 0;
	//wchar_t var_name[80], temp[80];
	ScannerState *start_state = new ScannerState(scanner);
	scanner->GetToken();
	int type = scanner->tok;
	do{
		Var * i = new Var();
		//sund_type_node * type_node = new sund_type_node(type, -1);
		SundTypeBuilder * builder = new SundTypeBuilder(SundTypeBuilder::LOCAL, type);
		builder->AssignVar(i);
		builder->sund_type = builder->Build(scanner);
		//builder->sund_type->Add(type_node);
		builder->sund_type->basic_type = type;
		i->basic_type = type;
		if (func_name != 0){
			wcscpy(i->func_name, func_name);
		}
		i->sund_type = builder->sund_type;
		i->value = 0;
		i->size = Var::GetSize(scanner->tok);
		i->program_control = program_control;
		//is_pointer = 0;
		i->SetStorageClass(storage_class);
		i->SetQualifier(type_qualifier);

		if (!builder->sund_type->HasSubscript()){
			// print error
			Err->Print(ErrorPrinter::MISSING_SUBSCRIPT, i->var_name);
			return;
		}
		if (*scanner->token == '='){
			SundInitializer * initializer = new SundInitializer(scanner, sundParser, builder->sund_type);
			initializer->Build();
			ProgramControlBase * init_list_control = new ProgramControlBase(ProgramControlBase::INITIALIZER, builder, initializer->GetInitListSize(), initializer->GetInitList());
			program_control->Add(init_list_control);
		}
		else{
			ProgramControlBase * decl_control = new ProgramControlBase(ProgramControlBase::DECL, builder);
			program_control->Add(decl_control);
		}
		
		// verify local var
		if (DataSet->IsLocalVar(i->var_name, i->func_name)){
			Err->AddError(ErrorPrinter::LOCAL_VAR_REDEFINITION, i->var_name);
		}
		else DataSet->vars.push_front(i);

	}while(*(scanner->token)==',');
	

	if(*(scanner->token)!=';'){
		Err->Print(start_state,ErrorPrinter::SEMI_EXPECTED);
		return ;
	}
}
ProgramControlBase * SundCompiler::PerformWhile(ProgramControlBase *program_control)
{
	WhileControl * while_control = new WhileControl(ProgramControlBase::WHILE);
	while_control->parent = program_control;
	while_control->SetId(ProgramControlBase::GenerateId(ProgramControlBase::WHILE));
	int brace = 0;
	ScannerState * scannerState = new ScannerState(scanner);
	ChainExpression * chain = sundParser->PerformExp(scannerState);
	if(chain == 0){
		Err->Print(scannerState,ErrorPrinter::ILLEGAL_EXPR);
		delete scannerState;
		return 0;
	}
	else{
		while_control->SetWhileCondition(chain);
		//chain->Print();
	}
	scanner->GetToken();
	if(*(scanner->token)=='{'){
		++brace;
	}
	else{
		scanner->Putback();
	}
	 
	ChainExpression * end_stmp_chain = DoStatement(while_control,brace);
	/*wchar_t * label_name = end_stmp_chain->GetResult()->var_name;
	ProgramControlBase * label_control = new ProgramControlBase(ProgramControlBase::LABEL);
	label_control->SetName(label_name);
	while_control->Add(label_control);
	// don't print chain!
	//end_stmp_chain->Print();*/
	delete scannerState;
	return while_control;
}
ProgramControlBase * SundCompiler::PerformDo(ProgramControlBase *program_control)
{
	DoControl * do_control = new DoControl(ProgramControlBase::DO); 
	do_control->parent = program_control;
	do_control->SetId(ProgramControlBase::GenerateId(ProgramControlBase::DO));
	int brace = 0;
	ScannerState * scannerState = new ScannerState(scanner);
	scanner->GetToken();
	if(*(scanner->token)=='{'){
		++brace;
	}
	else{
		scanner->Putback();
	}
	
	ChainExpression * end_stmp_chain = DoStatement(do_control,brace);
	// is there while 
	scanner->GetToken();
	if(scanner->tok == WHILE){
	   //printf("%s cond: ",while_temp);
	   ChainExpression * while_exp = sundParser->PerformExp(scannerState);
	   if(while_exp==0){
		   Err->Print(scannerState,ErrorPrinter::SYNTAX);
		   delete scannerState;
		   return 0;
	   }
	   else{
		   do_control->SetWhileCondition(while_exp);
		   //while_exp->Print();
	   }
	   scanner->GetToken();
	   if(*(scanner->token)!=';'){
		   Err->Print(scannerState,ErrorPrinter::SEMI_EXPECTED);
		   delete scannerState;
		   return 0;
	   }
	}
	else{
		Err->Print(scannerState,ErrorPrinter::WHILE_EXPECTED);
	}
	delete scannerState;
	return do_control; 
}
ProgramControlBase * SundCompiler::PerformIf(ProgramControlBase *program_control)
{
	IfControl * if_control = new IfControl(ProgramControlBase::IF);
	if_control->parent = program_control;
	if_control->SetId(ProgramControlBase::GenerateId(ProgramControlBase::IF));
	ScannerState * state = new ScannerState(scanner);
	int brace = 0;
	ChainExpression * chain = sundParser->PerformExp(state);
	if(chain == 0){
		Err->Print(state,ErrorPrinter::ILLEGAL_EXPR);
		delete state;
		return 0;
	}
	else{
		if_control->SetIfCondition(chain);
		//chain->Print();
	}
	scanner->GetToken();
	if(*(scanner->token)=='{'){
		++brace;
	}
	else{
		scanner->Putback();
	}
	
	ChainExpression * end_stmp_chain = DoStatement(if_control,brace);
	
	// is there else branch 
	scanner->GetToken();
	if(scanner->tok == ELSE){
		// handles else branch
		ProgramControlBase * else_program_control = new ProgramControlBase(ProgramControlBase::ELSE);
		else_program_control->parent = program_control;
		int else_brace = 0;
		scanner->GetToken();
		if(*(scanner->token)=='{'){
			++else_brace;
		}
		else{
			scanner->Putback();
		}
		//printf("%s: \n",else_temp);
		ChainExpression * else_end_stmp_chain = DoStatement(else_program_control,else_brace);
		if_control->SetElseControl(else_program_control);
	}
	else{
		scanner->Putback();
	}
	delete state;

	return if_control;
}
ProgramControlBase * SundCompiler::PerformFor(ProgramControlBase *program_control)
{
	ForControl * for_control = new ForControl(ProgramControlBase::FOR);
	for_control->parent = program_control;
	for_control->SetId(ProgramControlBase::GenerateId(ProgramControlBase::FOR));
	ScannerState * state = new ScannerState(scanner);
	ChainExpression * chain1 = 0, * chain2 = 0, * chain3 = 0;
	
	int brace = 0;
	// Get next token "("
	scanner->GetToken();
	if(*(scanner->token)!='('){
		Err->Print(state,ErrorPrinter::PAREN_EXPECTED,L"for");
		delete state;
		return 0;
	}
	chain1 = sundParser->PerformExp(state,DO_SEPARATE_EXPR);
	if(chain1 == 0){
		Err->Print(state,ErrorPrinter::SYNTAX,L"for");
		delete state;
		return 0;
	}
	else{
		for_control->SetExpr1(chain1);
		//chain1->Print();
	}
	// Get next token ";"
	scanner->GetToken();
	if(*(scanner->token)!=';'){
		Err->Print(state,ErrorPrinter::SEMI_EXPECTED,L"for");
		delete state;
		return 0;
	}
	
	chain2 = sundParser->PerformExp(state);
	if(chain2 == 0){
		Err->Print(state,ErrorPrinter::SYNTAX,L"for");
		delete state;
		return 0;
	}
	else{
		for_control->SetExpr2(chain2);
		//chain2->Print();
	}
	// Get next token ";"
	scanner->GetToken();
	if(*(scanner->token)!=';'){
		Err->Print(state,ErrorPrinter::SEMI_EXPECTED,L"for");
		delete state;
		return 0;
	}
	
	chain3 = sundParser->PerformExp(state,DO_SEPARATE_EXPR);
	if(chain3 == 0){
		Err->Print(state,ErrorPrinter::SYNTAX,L"for");
		delete state;
		return 0;
	}
	else{
		for_control->SetExpr3(chain3);
		//chain3->Print();
	}
	
	// Get next token ")"
	scanner->GetToken();
	if(*(scanner->token)!=')'){
		Err->Print(state,ErrorPrinter::PAREN_EXPECTED,L"for");
		delete state;
		return 0;
	}
	scanner->GetToken();
	if(*(scanner->token)=='{'){
		++brace;
	}
	else{
		scanner->Putback();
	}

	ChainExpression * end_stmp_chain = DoStatement(for_control,brace);

	delete state;
	return for_control;
}
ProgramControlBase * SundCompiler::PerformSwitch(ProgramControlBase *program_control)
{
	SwitchControl * switch_control = new SwitchControl(ProgramControlBase::SWITCH);
	switch_control->parent = program_control;
	switch_control->SetId(ProgramControlBase::GenerateId(ProgramControlBase::SWITCH));
	ScannerState * state = new ScannerState(scanner);
	int brace = 0;
	ChainExpression * chain = sundParser->PerformExp(state);
	if(chain == 0){
		Err->Print(state,ErrorPrinter::ILLEGAL_EXPR);
		delete state;
		return 0;
	}
	else{
		switch_control->SetSwitchExpr(chain);
		//chain->Print();
	}
	scanner->GetToken();
	if(*(scanner->token)=='{'){
		++brace;
	}
	else{
		scanner->Putback();
	}
	ChainExpression * end_stmp_chain = DoStatement(switch_control,brace);

	delete state;
	return switch_control;
}
ProgramControlBase * SundCompiler::PerformCase(ProgramControlBase *program_control)
{
	ProgramControlBase * case_control = 0;
	ScannerState * state = new ScannerState(scanner);
	if(program_control->GetControlType() != ProgramControlBase::SWITCH){
		Err->Print(state,ErrorPrinter::SYNTAX,L"case");
		delete state;
	    return 0;
	}
	int brace = 0;
	wchar_t temp[80];
	ChainExpression * chain = sundParser->PerformExp(state);
	if(chain == 0){
		Err->Print(state,ErrorPrinter::ILLEGAL_EXPR);
		delete state;
		return 0;
	}
	else{
		case_control = new ProgramControlBase(ProgramControlBase::CASE);
		case_control->parent = program_control;
		//chain->Print();
	}
	// get ':'
	scanner->GetToken();
	if(*(scanner->token)!=':'){
		Err->Print(state,ErrorPrinter::SYNTAX);
		delete state;
		return 0;
	}
	scanner->GetToken();
	if(*(scanner->token)=='{'){
		++brace;
	}
	else{
		scanner->Putback();
	}
	
	scanner->ReadNextToken(temp);
	if (!wcscmp(temp, L"case")){
		//fallthru
		delete state;
		case_control->SetExpr(chain);
		return case_control;
	}
	
	case_control->SetExpr(chain);
	ChainExpression * end_stmp_chain = DoStatement(case_control,brace);

	delete state;
	return case_control;
}
ProgramControlBase * SundCompiler::PerformDefault(ProgramControlBase *program_control)
{
	ProgramControlBase * default_control = new ProgramControlBase(ProgramControlBase::DEFAULT);
	default_control->parent = program_control;
	ScannerState * state = new ScannerState(scanner);
	if(program_control->GetControlType() != ProgramControlBase::SWITCH){
		Err->Print(state,ErrorPrinter::SYNTAX,L"default");
		delete state;
	    return 0;
	}
	int brace = 0;
	// get ':'
	scanner->GetToken();
	if(*(scanner->token)!=':'){
		Err->Print(state,ErrorPrinter::SYNTAX);
		delete state;
		return 0;
	}
	scanner->GetToken();
	if(*(scanner->token)=='{'){
		++brace;
	}
	else{
		scanner->Putback();
	}
	ChainExpression * end_stmp_chain = DoStatement(default_control,brace);

	delete state;
	return default_control;
}
ProgramControlBase * SundCompiler::PerformGoto(ProgramControlBase *program_control)
{
	wchar_t temp[80];
	ScannerState * state = new ScannerState(scanner);
	ProgramControlBase * goto_control = new ProgramControlBase(ProgramControlBase::GOTO);
	goto_control->parent = program_control;
	// get label's name
	scanner->GetToken();
	wcscpy(temp, scanner->token);
	goto_control->SetName(temp);
	if(!DataSet->IsLabel(temp)){
		Err->Print(ErrorPrinter::UNDEFINED_LABEL,temp);
		return 0;
	}
	scanner->GetToken();
	if(*(scanner->token)!=';'){
		Err->Print(ErrorPrinter::SEMI_EXPECTED);
		return 0;
	}
	delete state;
	return goto_control;
}
ProgramControlBase * SundCompiler::PerformBreak(ProgramControlBase *program_control)
{
	wchar_t temp[80];
	ProgramControlBase * p ;
	ProgramControlBase * break_control = new ProgramControlBase(ProgramControlBase::BREAK);
	break_control->parent = program_control;
	ScannerState * state = new ScannerState(scanner);
	p = program_control;
	int found = 0;
	do{
		if(p->GetControlType() == ProgramControlBase::WHILE 
			|| p->GetControlType() == ProgramControlBase::DO 
			|| p->GetControlType() == ProgramControlBase::SWITCH
			|| p->GetControlType() == ProgramControlBase::FOR)
		{
			found = 1;
			break;
		}
		p = p->parent;
	}while(p != 0);

	if(found == 0){
		Err->Print(state,ErrorPrinter::SYNTAX,L"break");
		delete state;
		return 0;
	}
	swprintf(temp,80, L"%s_%d", p->GetControlTypeName(), p->GetId());
	break_control->SetName((wchar_t *)temp);
	// get ':'
	scanner->GetToken();
	if(*(scanner->token)!=';'){
		Err->Print(state,ErrorPrinter::SEMI_EXPECTED);
		delete state;
		return 0;
	}

	delete state;
	return break_control;
}
ProgramControlBase * SundCompiler::PerformContinue(ProgramControlBase *program_control)
{
	wchar_t temp[80];
	ProgramControlBase * p ;
	ProgramControlBase * continue_control = new ProgramControlBase(ProgramControlBase::CONTINUE);
	continue_control->parent = program_control;
	ScannerState * state = new ScannerState(scanner);
	p = program_control;
	int found = 0;
	do{
		if(p->GetControlType() == ProgramControlBase::WHILE 
			|| p->GetControlType() == ProgramControlBase::DO 
			|| p->GetControlType() == ProgramControlBase::FOR)
		{
			found = 1;
			break;
		}
		p = p->parent;
	}while(p != 0);

	if(found == 0){
		Err->Print(state,ErrorPrinter::SYNTAX,L"continue");
		delete state;
		return 0;
	}
	
	swprintf(temp,80, L"%s_%d", p->GetControlTypeName(), p->GetId());
	continue_control->SetName((wchar_t *)temp);
	//continue_control->SetName((char *)p->GetControlTypeName());
	// get ':'
	scanner->GetToken();
	if(*(scanner->token)!=';'){
		Err->Print(state,ErrorPrinter::SEMI_EXPECTED);
		delete state;
		return 0;
	}

	delete state;
	return continue_control;
}
ProgramControlBase * SundCompiler::PerformReturn(ProgramControlBase *program_control)
{
	ProgramControlBase * return_control =  0;
	Var * func_var =  DataSet->GetFuncVar(this->sundParser->GetFuncName());
	// remove FUNCTION node
	func_var->GetSundType()->sund_type_list.pop_front();
	ScannerState * scannerState = new ScannerState(scanner);
	ChainExpression * chain = sundParser->PerformExp(scannerState);
	return_control = new ProgramControlBase(ProgramControlBase::RETURN,chain);
	return_control->parent = program_control;
	
	if(chain == 0)
	{
		//Err->Print(scannerState,ErrorPrinter::FUNC_UNDEF);
		//delete scannerState;
	    //return 0;

		// expression 'return ;'
		if (func_var->sund_type->basic_type != SundCompiler::VOID){
			Err->AddError(scannerState, ErrorPrinter::ILLEGAL_RETURN_EXPR);
		}
		if (*(scanner->token) != ';'){
			Err->Print(ErrorPrinter::SEMI_EXPECTED);
		}
		delete scannerState;
		return return_control;
	}
	else{
		//chain->Print();

		if (!func_var->sund_type->IsEqual(chain->GetSundType()) || (func_var->sund_type->basic_type == SundCompiler::VOID && chain->GetSundType()->basic_type!=SundCompiler::VOID)){
			Err->AddError(scannerState, ErrorPrinter::ILLEGAL_RETURN_EXPR);
		}
	}
	
	scanner->GetToken();
	if(*(scanner->token)!=';'){
		Err->Print(ErrorPrinter::SEMI_EXPECTED);
	}
	delete scannerState;
	return return_control;
}
ChainExpression * SundCompiler::DoStatement(ProgramControlBase *program_control, int brace,wchar_t * func_name)
{
	sundParser->program_control = program_control;
	ChainExpression * chain_result = 0;
	do{
		scanner->token_type=scanner->GetToken();
		if(*(scanner->token)=='{'){
			// new statement: do recursion
			ProgramControlBase * new_program_control = new ProgramControlBase();
			new_program_control->parent = program_control;
			new_program_control->SetId(ProgramControlBase::GenerateId(ProgramControlBase::BLOCK));

			ChainExpression * end_stmp_chain = DoStatement(new_program_control, 1);
			program_control->Add(new_program_control);

			continue;
	    }
		else
		if(*(scanner->token)=='}'){
			brace = 0;
		}
		if(brace == 0 && *(scanner->token)=='}'){
			break;
		}
		//is storage class: STATIC,AUTO,REGISTER
		int storage_class = AUTO;
		if (IsStorageClass(scanner->tok)){
			storage_class = scanner->tok;
			scanner->GetToken();
		}
		// is type qualifier: CONST or VOLATILE
		int type_qualifier = -1;
		if (IsTypeQualifier(scanner->tok)){
			type_qualifier = scanner->tok;
			scanner->GetToken();
		}
		if(IsRegularType(scanner->tok))
		{
			scanner->Putback();
			DeclLocal(program_control, storage_class, type_qualifier,func_name);
			continue;
		} 
		switch(scanner->tok)
		{
			case RETURN:
			program_control->Add(PerformReturn(program_control));
			break;
			case IF:
			program_control->Add(PerformIf(program_control));
			break;
			case ELSE:
			Err->Print(ErrorPrinter::UNRECOGNIZED_STMT_ELSE);
			break;
			case WHILE:
			program_control->Add(PerformWhile(program_control));
			break;
			case DO:
			program_control->Add(PerformDo(program_control));
			break;
			case FOR:
			program_control->Add(PerformFor(program_control));
			break;
			case GOTO:
			program_control->Add(PerformGoto(program_control));
			break;
			case BREAK:
			program_control->Add(PerformBreak(program_control));
			break;
			case CONTINUE:
			program_control->Add(PerformContinue(program_control));
			break;
			case SWITCH:
			program_control->Add(PerformSwitch(program_control));
			break;
			case CASE:
			program_control->Add(PerformCase(program_control));
			break;
			case DEFAULT:
			program_control->Add(PerformDefault(program_control));
			break;
			
			default:
			//  identifier is label ?
			if(DataSet->IsLabel(scanner->token)){
				ScannerState * state = new ScannerState(scanner);
				ProgramControlBase * label_control = new ProgramControlBase(ProgramControlBase::LABEL);
				label_control->SetName(scanner->token);
				// get next token ':'
				scanner->GetToken();
				if(*(scanner->token)!=':'){
					Err->Print(state,ErrorPrinter::SYNTAX,L"label");
					delete state;
				}
				program_control->Add(label_control);
				continue;
			}
			scanner->Putback();
			
			// perform expression
			ChainExpression * root;
			ScannerState * state = new ScannerState(scanner); 
			root = sundParser->PerformExp(state,DO_SEPARATE_EXPR);
			if(*(scanner->token)!=';'){
				Err->Print(state,ErrorPrinter::SEMI_EXPECTED);
				delete state;
			}
			else{
				// get next token ';'
				scanner->GetToken();
			}
			ProgramControlBase * expr_program_control = new ProgramControlBase(ProgramControlBase::EXPR,root); 
			expr_program_control->parent = program_control; 
			program_control->Add( expr_program_control );
			break;
		}
    }while(scanner->tok!=FINISHED && brace);

	/*if(stmt_name == 0){
	   return 0;
	}*/

	chain_result = ChainExpression::MakeLabelChain(program_control->GetControlTypeName(),program_control->GetId());
	//revert old program_control
	sundParser->program_control = program_control->parent;

	return chain_result;
}
void SundCompiler::AssignMainProgram(ProgramControlBase * main_program)
{
	this->main_program = main_program;
}
void SundCompiler::AssignGlobalStatement(ProgramControlBase * global_statement)
{
	this->global_statement = global_statement;
}
void SundCompiler::WriteBOM(FILE * file)
{
	// always puts BOM signature (0xEF 0xBB 0xBF)
	fputwc(0xEF, file);
	fputwc(0xBB, file);
	fputwc(0xBF, file);
}
void SundCompiler::BypassFuncArgs()
{
	// bypass code inside function's (arg1,arg2...)
	int c_left = 1,c_right = 0;
	/*while (*(scanner->prog) != ')' && *(scanner->prog)){
		(scanner->prog)++;
	}*/
	while (*(scanner->prog)){
		if (*(scanner->prog) == ')')
			c_right++;
		if (*(scanner->prog) == '(')
			c_left++;
		
		(scanner->prog)++;
		if (c_left == c_right)
			break;
	}
	// deadlock prevention
	if (*(scanner->prog) == '\0'){
		Err->Print(ErrorPrinter::SYNTAX);
		return;
	}
	// move pointer after  ')'
	//(scanner->prog)++;
}
/*ProgramControlBase * SundCompiler::DeclArray(Var * var)
{
wchar_t temp[80];
ChainExpression * chain = 0;
ProgramControlBase * result = 0;
ScannerState * scannerState = new ScannerState(scanner);
scanner->ReadNextToken(temp);
if(*temp!=']')
{
chain = sundParser->PerformExp(scannerState);
if(chain == 0)
{
Err->Print(scannerState,ErrorPrinter::SYNTAX);
delete scannerState;
return 0;
}
if(!chain->IsEmpty() || !chain->GetResult()->IsConst())
{
Err->Print(scannerState,ErrorPrinter::NO_CONST_EXPR);
delete scannerState;
return 0;
}

scanner->GetToken();
if(*(scanner->token)!=']')
{
Err->Print(scannerState,ErrorPrinter::SYNTAX);
delete scannerState;
return 0;
}
// set array specifier
int array_size = chain->GetResult()->value;
var->SetSpecifier(Var::ARRAY);
var->size = var->size*array_size;
//push var
DataSet->vars.push_front(var);
}
else{
// print error message
scanner->GetToken();
}


scanner->GetToken();
if(*(scanner->token) == '='){
//array init. = {a1,a2,...an};
scanner->GetToken();
if(*(scanner->token)!='{'){
if(scanner->token_type == STRING && var->type == CHAR){
// char arr[] = "string";
ProgramControlBase * p_c = DoCharArrayInit(var);
return p_c;
}
else{
Err->Print(scannerState,ErrorPrinter::SYNTAX);
delete scannerState;
return 0;
}

}
result = DoArrayInit(var);
}
else{
scanner->Putback();
}

delete scannerState;
return result;
}
ProgramControlBase * SundCompiler::DoArrayInit(Var * var)
{
ProgramControlBase * result = 0;
ChainExpression * chain_result = new ChainExpression(), * curr_chain = 0;
// create array_init node
Var * array_init_var = new Var();
array_init_var->SetName(var->var_name);
//array_init_var->SetSpecifier(Var::FUNCTION);
IntermediateNode * array_init_node = new IntermediateNode(ChainExpression::ARRAY_INIT,array_init_var,0,1);
chain_result->AddNode(array_init_node);
int i = 0;
ScannerState * scannerState = new ScannerState(scanner);
curr_chain = chain_result;
do{
ChainExpression * chain = sundParser->PerformExp(scannerState);
curr_chain->comma_chain = chain;
curr_chain = chain;
scanner->GetToken();
++i;
}while(*(scanner->token)==',');

if(*(scanner->token)!='}'){
Err->Print(scannerState,ErrorPrinter::SYNTAX);
delete scannerState;
}

// set array specifier
int array_size = i;
var->SetSpecifier(Var::ARRAY);
var->size = var->size*array_size;
//push var
DataSet->vars.push_front(var);

result = new ProgramControlBase(ProgramControlBase::ARRAY_INIT,chain_result);

return result;
}
// eg. char arr[] = "string";
ProgramControlBase * SundCompiler::DoCharArrayInit(Var * var)
{
wchar_t buffer[80];
ChainExpression * chain_result = new ChainExpression(), * curr_chain = 0;
int length = (int)wcslen(scanner->token);
// create array_init node
Var * array_init_var = new Var();
array_init_var->SetName(var->var_name);
IntermediateNode * array_init_node = new IntermediateNode(ChainExpression::ARRAY_INIT,array_init_var,0,1);
chain_result->AddNode(array_init_node);
ProgramControlBase * result = new ProgramControlBase(ProgramControlBase::ARRAY_INIT,chain_result);
curr_chain = chain_result;
for(int i=0;i<=length;++i){
wchar_t ch;
ChainExpression * chain = new ChainExpression();
// create result var in chain
Var * cnst_var = new Var((wchar_t *)"",-1,Var::CONST);

if(i != length){
ch = *(scanner->token+i);
cnst_var->SetValue(ch);
swprintf(buffer, 80,L"CONST_'%c'", ch);
}
else{
cnst_var->SetValue(0);
swprintf(buffer,80, L"CONST_%c%c0%c", 0x27, 0x5c, 0x27);
}
cnst_var->SetName(buffer);
chain->result = new Var();
chain->result->Copy(cnst_var);

curr_chain->comma_chain = chain;
curr_chain = chain;
}
// set array specifier
var->SetSpecifier(Var::ARRAY);
var->size = var->size*length;
//push var
DataSet->vars.push_front(var);
return result;
}*/
/*void SundCompiler::DoFunction(int type, int qualifier, wchar_t * func_name, int storage_class)
{
int brace = 0;// , func_index;
//wchar_t temp_token[80];
int fi = DataSet->GetFuncTableSize();
int index = DataSet->GetFunc(func_name);
// da li je funkcija 'func_name' vec deklarisana?
if(index == -1){
func_index = fi;
}
else{
// yes, func. def.
func_index = index;
}
sundParser->SetFuncIndex(func_index);

sundParser->SetFuncName(func_name);
ScannerState  *s1 = new ScannerState(scanner);

// first: bypass code inside function's (arg1,arg2...)
BypassFuncArgs();

ScannerState  *s3 = new ScannerState(scanner);
// get next token
scanner->GetToken();

//FuncType * func_type = new FuncType(func_name,type, qualifier,storage_class);
//if (*(scanner->token) == ';'){
if (*(scanner->token) != '{'){
// function declaration
//if (DataSet->IsFunc(func_name) == -1){
if (DataSet->IsVarFunc(func_name) == 0){
ScannerState * temp = new ScannerState(scanner);
scanner->Revert(*s1);
PushParams(func_name);
//DataSet->func_table.push_back(*func_type);//front
scanner->Revert(*temp);
}
else{
Err->Print(s1, ErrorPrinter::FUNC_REDEFINITION, func_name);
return;
}
return;
}
else
if(*(scanner->token)=='{'){
if (DataSet->IsFunc(func_name) == -1){
//if (DataSet->IsVarFunc(func_name) == 0){
ScannerState * temp = new ScannerState(scanner);
//func_type->defined = 1;
scanner->Revert(*s1);
PushParams( func_name);
//DataSet->func_table.push_back(*func_type);//front
scanner->Revert(*temp);
}
else{
//func. already declared
if (DataSet->SetFuncDefined(func_name)){
// print error
Err->Print(s3,ErrorPrinter::FUNC_ALREADY_HAS_BODY, func_name);
return;
}
}
brace++;
}
else{
Err->Print(ErrorPrinter::SYNTAX, scanner->token);
return;
}
// bypass code inside func. body
while(brace)
{
wcscpy(temp_token, scanner->token);
scanner->GetToken();
// deadlock prevention
if(scanner->tok==FINISHED){
// print error msg
Err->Print(ErrorPrinter::SYNTAX);
}
if(*(scanner->token)==':'){
scanner->Putback();
DeclLabel(temp_token, func_name);
scanner->GetToken();
}

if(*(scanner->token)=='{'){
brace++;
}
if(*(scanner->token)=='}'){
brace--;
}
}
ScannerState  *s2 = new ScannerState(scanner);
scanner->Revert(*s3);

// get next token ( '{' )
scanner->GetToken();
if(*(scanner->token)=='{'){
ProgramControlBase * program_control = new ProgramControlBase(ProgramControlBase::FUNC,func_name);
DoStatement(program_control, 1, func_name);
this->main_program->Add(program_control);
}
else{
// print error message
}
scanner->Revert(*s2);
}*/
