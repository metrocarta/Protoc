#include "../Include/Scanner.h"
#include "../Include/ErrorPrinter.h"
#include "../Include/SundCompiler.h"
#include "../Include/ScannerState.h"

Scanner::Scanner()
{
}
Scanner::Scanner(wchar_t const * file_name)
{
	this->file_name = file_name;
}

Scanner::Scanner(wchar_t * prog, wchar_t * p_buf)
{
	this->prog = prog;
	this->p_buf = p_buf;
}
Scanner::~Scanner(void)
{
}


int Scanner::Iswhite(wchar_t c)
{
	if(c==' '||c=='\t') return 1;
	else return 0;
}
int Scanner::Isdelim(wchar_t c) //vraca 1 ako je c delimiter
{
	if (wcschr(L" !;,+-<>'/*%^=(){}:&|^[]$#@~.?\\", c) || c == 9 || c == '\r' || c == '\n' || c == 0) return 1;
	else return 0;  // {} sam dodao zbog else{
}
int Scanner::GetToken(void)
{
	wchar_t *temp;
	
Entry:
	
	token_type=0;
	tok=0;
	temp=token;

	*temp='\0';
    // skip over white space
	while(Iswhite(*prog)&& *prog) ++prog;

	if(*prog=='\r'||*prog=='\n')
	{
		if (*prog == '\r')	
			++prog;
		if (*prog == '\n')
			++prog;

			//++prog;
			//++prog;
		// again, skip over white space
		while(Iswhite(*prog)&&*prog) ++prog;

		goto Entry;
    }
    if(*prog=='\0') //end of file
	{
         *token='\0';
		 tok = SundCompiler::FINISHED;
		 return (token_type = SundCompiler::DELIMITER);
    }

	if (wcschr(L"{}", *prog)) // block delimiters
	{
       *temp=*prog;
       temp++;
       *temp='\0';
       prog++;
	   return (token_type = SundCompiler::BLOCK);
    }
	// look for c preprocessor directives
	if(*prog=='#')
	{
		*temp++='#';
		*temp='\0';
		prog++;
		return (token_type = SundCompiler::C_PPC_DIR);
	}
	// look for module directives (proto_c extension) @directive
	if (*prog == '@')
	{
		*temp++ = '@';
		*temp = '\0';
		prog++;
		return (token_type = SundCompiler::MODULE_DIR);
	}
	if(*prog==':')
	{
		prog++;
		*temp++=':';
		*temp='\0';
		return (token_type = SundCompiler::LABEL_DELIMITER);
	}
	// look for comment
    if(*prog=='/')
	{
        if(*(prog+1)=='*') // if a comment
		{
		   //char * s_ptr = prog;
           prog+=2;
           do{ 
			  // find end of comment
              while(*prog!='*' && *prog) prog++;
			  if (*prog == '\0')  return (token_type = SundCompiler::FINISHED);
              prog++;
		   } while (*prog != '/'  && *prog);
		   if (*prog == '\0')  return (token_type = SundCompiler::FINISHED);
           prog++;
	       goto Entry;
        }
		if(*(prog+1)=='/')
		{
			// C++ comment (dg)
			//char * s_ptr = prog;
			prog+=2;
			do{
				prog++;
			}while(*prog!='\n');

			prog++;
			//return (token_type=COMMENT);
			goto Entry;
		}
	}
	if (wcschr(L"!<>=", *prog)) // might be relation operator
	{
       switch(*prog){
             case '=':
                  if(*(prog+1)=='=')
				  {
                      prog++;prog++;
                      *temp=EQ;
                      temp++;
                      *temp=EQ;
                      temp++;
                      *temp='\0';
                  }
                  break;
             case '!':
                  if(*(prog+1)=='=')
				  {
                     prog++;
                     prog++;
                     *temp=NE;
                     temp++;
                     *temp=NE;
                     temp++;
                     *temp='\0';
                   }
                   break;
              case '<':
				   if(*(prog+1)=='<' && *(prog+2)!='<') 
				   {
					   // bitshift left
					   prog++;
                       prog++;
					   *temp = SundCompiler::BITSHIFT_LEFT;
                       temp++;
					   *temp = SundCompiler::BITSHIFT_LEFT;
				   }
				   else
                   if(*(prog+1)=='=')
				   {
                      prog++;
                      prog++;
                      *temp=LE;
                      temp++;
                      *temp=LE;
                   }
                   else 
				   {
                      prog++;
                      *temp=LT;
                   }
                   temp++;
                   *temp='\0';
                   break;
              case '>':
			       if(*(prog+1)=='>' && *(prog+2)!='>')
				   {
					   // bitshift right
					   prog++;
                       prog++;
					   *temp = SundCompiler::BITSHIFT_RIGHT;
                       temp++;
					   *temp = SundCompiler::BITSHIFT_RIGHT;
				   }
				   else
                   if(*(prog+1)=='=')
				   {
                      prog++;prog++;
                      *temp=GE;
                      temp++;
                      *temp=GE;
                   }
                   else 
				   {
                      prog++;
                      *temp=GT;
                   }
                   temp++;
                   *temp='\0';
                   break;
       }//end of switch
	   if (*token) return(token_type = SundCompiler::DELIMITER);
    }
	if (wcschr(L"+-&|*^/%=;(),'[]$#@~!.?\\", *prog)) //was strchr,delimiter
	{
		if(*prog == '('){
			wchar_t * save_prog = prog++;
			int result = IsCastOperator(temp); 
		    if(result == 0){
			   prog = save_prog;
		    }
			else{
			   // this is cast operator
			   tok = result;
			   return (token_type = SundCompiler::CAST);
			}
		}
				
        if(*prog=='+' && *(prog+1)=='+' && *(prog+2)!='+'){
			*temp++ = SundCompiler::INCREMENT;
			*temp++ = SundCompiler::INCREMENT;
			*temp='\0';
			prog++;prog++;
			return(token_type = SundCompiler::DELIMITER);
		}
		else
		if(*prog=='-' && *(prog+1)=='-' && *(prog+2)!='-'){
			*temp++ = SundCompiler::DECREMENT;
			*temp++ = SundCompiler::DECREMENT;
			*temp='\0';
			prog++;prog++;
			return(token_type = SundCompiler::DELIMITER);
		}
		else
		if(*prog=='+' && *(prog+1)=='='){
			*temp++ = SundCompiler::COMP_ADD;
			*temp++ = SundCompiler::COMP_ADD;
			*temp='\0';
			prog++;prog++;
			return(token_type = SundCompiler::DELIMITER);
		}
		else
		if(*prog=='-' && *(prog+1)=='='){
			*temp++ = SundCompiler::COMP_SUB;
			*temp++ = SundCompiler::COMP_SUB;
			*temp='\0';
			prog++;prog++;
			return(token_type = SundCompiler::DELIMITER);
		}
		else
		if(*prog=='*' && *(prog+1)=='='){
			*temp++ = SundCompiler::COMP_MULT;
			*temp++ = SundCompiler::COMP_MULT;
			*temp='\0';
			prog++;prog++;
			return(token_type = SundCompiler::DELIMITER);
		}
		else
		if(*prog=='/' && *(prog+1)=='='){
			*temp++ = SundCompiler::COMP_DIV;
			*temp++ = SundCompiler::COMP_DIV;
			*temp='\0';
			prog++;prog++;
			return(token_type = SundCompiler::DELIMITER);
		}
		else
		if(*prog=='&' && *(prog+1)=='='){
			*temp++ = SundCompiler::COMP_AND;
			*temp++ = SundCompiler::COMP_AND;
			*temp='\0';
			prog++;prog++;
			return(token_type = SundCompiler::DELIMITER);
		}
		else
		if(*prog=='|' && *(prog+1)=='='){
			*temp++ = SundCompiler::COMP_OR;
			*temp++ = SundCompiler::COMP_OR;
			*temp='\0';
			prog++;prog++;
			return(token_type = SundCompiler::DELIMITER);
		}
		else
		if(*prog=='&' && *(prog+1)=='&'){
			prog++;prog++;
			*temp = SundCompiler::AND_AND;
            temp++;
			*temp = SundCompiler::AND_AND;
			temp++;
			*temp='\0';
			return(token_type = SundCompiler::DELIMITER);
		}
		else
		if(*prog=='|' && *(prog+1)=='|'){
			prog++;prog++;
			*temp = SundCompiler::OR_OR;
            temp++;
			*temp = SundCompiler::OR_OR;
			temp++;
			*temp='\0';
			return(token_type = SundCompiler::DELIMITER);
		}
		else
		if(*prog == 0x27 && *(prog+2) == 0x27){
			//  this is maybe (') apostroph character
			if (iswprint(*(prog + 1))){
				*temp++ = *prog;
				*temp++ = *(prog+1);
				*temp++ = *(prog+2);
				*temp='\0';
				prog++;prog++;prog++;
				return (token_type = SundCompiler::CHARACTER);
			}
		}
		else
		if (*prog == '\''){
			wchar_t * s = prog ;
			int i = 0;
			    do{
					*temp++ = *prog++;
					i++;
				} while (*prog != '\'' && i<(TOKEN_MAX_SIZE-1) && *prog);

				if (*prog == '\0' || i>=(TOKEN_MAX_SIZE - 1))
				{
					prog = s+1;
					temp = token + 1;
					*temp = '\0';
					return (token_type = SundCompiler::DELIMITER);
				}
				else{
					*temp++ = *prog++;
					*temp = '\0';
					return (token_type = SundCompiler::CHARACTER);
				}		
		}
        *temp=*prog;
        prog++;
        temp++;
        *temp='\0';
		return (token_type = SundCompiler::DELIMITER);
     }

     if(*prog=='"') // quoted string
	 {
		 int i = 0;
         prog++;
		 wchar_t *s = prog;
		 while (*prog != '"' && *prog != '\0' && i<(TOKEN_MAX_SIZE - 1)){
			 *temp++=*prog++;
			 ++i;
		 }
		 if (*prog == '\0' || i >= (TOKEN_MAX_SIZE - 1))
		 {
			 //*prog == '"' || 
			 prog = s + 1;
			 temp = token + 1;
			 *temp = '\0';
			 return (token_type = SundCompiler::DELIMITER);
		 }
         /*if(*prog=='\r'){
			 int line_count = (1+GetLineCount());
			 Err->PrintFatal(line_count,ErrorPrinter::SYNTAX);
			 return 0;
		 }*/
         prog++;
         *temp='\0';
		 return (token_type = SundCompiler::STRING);
     }
	 if (iswdigit(*prog))// was isdigit, is number
	 {
	    if(*prog == '0')
		{
			prog++;
			if(*prog == 'b' || *prog == 'B')
			{
				// binary constant
				prog++;
				while(*prog == '1' || *prog == '0'){
					 *temp++=*prog++;
				}
				*temp = '\0';
				return (token_type = SundCompiler::B_NUMBER);
			}
			if(*prog == 'x' || *prog == 'X')
			{
				// hexadecimal constant
				prog++;
				while (iswxdigit(*prog)){//isxdigit
					*temp++=*prog++;
				}
				*temp = '\0';
				return (token_type = SundCompiler::HEX_NUMBER);
			}
			prog--;
		}

        while(!Isdelim(*prog)){
			  *temp++=*prog++;
		}
        *temp='\0';
	
		return (token_type = SundCompiler::NUMBER);
     }
	 if (iswalpha(*prog)) //was isalpha, var or command
	 {    
		 // isalpha
		 while(!Isdelim(*prog)){
		    	*temp++=*prog++;
		 }
		 token_type = SundCompiler::TEMP;
     }
	 /*if (!isascii(*prog)){
		 Err->Print(ErrorPrinter::NON_ASCII_CHAR);
		 //Err->PrintFatal(0, ErrorPrinter::NON_ASCII_CHAR, *prog);
	 }*/
	
     *temp='\0';
	 // see if a string is a command or a variable
	 if (token_type == SundCompiler::TEMP)
	 {
        tok=LookUp(token);
		if (tok) token_type = SundCompiler::KEYWORD;
		else token_type = SundCompiler::IDENTIFIER;
     }
	
     return token_type;
}
/*void Scanner::GetTokenAsString(wchar_t * buff)
{
	if (token_type != DELIMITER){
		wcscpy(buff, token);
		return;
	}

	switch (token[0]){
	case LT:
		wcscpy(buff, L"<");
		return;
	case GT:
		wcscpy(buff, L">");
		return;
	default:
		wcscpy(buff, token);
		return;
	}
}*/
int Scanner::IsCastOperator(wchar_t * p_token)
{
	int result = 0;
	int c = 0;
	// basic types
	wchar_t * table[] = { L"char", L"int", L"byte", L"word", L"bit" };
	while(isalpha(*prog)){
	      *p_token++=*prog;
		  prog++;
		  ++c;
	}
	*p_token='\0';
	int found = 0;
	for(int i=0;i<sizeof(table)/sizeof(table[0]);i++)
    {
		if (!wcscmp(table[i], token)){
			found = 1;
			result = (i+1);
			break;
		}
	}
	if(found == 0){
	   return 0;
	}
	
	int c_stars = 0,c_errors = 0;
	do{
		if(*prog == ')'){
			break;
		}
		if(*prog == '*'){
			c_stars++;
		}
		else
		if(*prog != ' '){
			c_errors++;
			break;
		}
		prog++;
	}while(*prog != ')');
	++prog;

	if(c_stars>1 || c_errors>0){
		return 0;
	}
	if(c_stars == 1){
		result = -result;
	}
	return result;
}
void Scanner::ReadNextToken(wchar_t * buff)
{
	ScannerState * state = new ScannerState(this);
	GetToken();
	wcscpy(buff, token);
	Revert(*state);
	delete state;
}
void Scanner::Print()
{
	wprintf(L"\t %s %s %s\n", token, GetTokenType(), GetTok());
}
wchar_t const * Scanner::GetTokenType()
{
	static wchar_t const * s[] = { L"DELIMITER", L"ILLEGAL_CHAR", L"IDENTIFIER", L"NUMBER", L"HEX_NUMBER", L"B_NUMBER", L"KEYWORD",
							 L"TEMP",L"STRING",L"BLOCK",L"C_PPC_DIR",L"PROTO_C_POST_DIR",L"LABEL_DELIMITER",L"INCREMENT",L"DECREMENT",L"COMP_ADD",
							 L"COMP_SUB",L"COMP_MULT",L"COMP_DIV",L"COMP_AND",L"COMP_OR",L"AND_AND",L"OR_OR", L"NOT",L"BITWISE_NOT",
							 L"CHARACTER",L"BITSHIFT_LEFT",L"BITSHIFT_RIGHT",L"CAST",L"SIZEOF",L"NEGATION"};

	if (token_type<SundCompiler::DELIMITER || token_type>SundCompiler::NEGATION)
	   return L"Unknown token type";

	return s[token_type];
}
wchar_t const * Scanner::GetTok()
{
	static wchar_t const * s[] = { L"CHAR", L"INT", L"BYTE", L"WORD", L"BIT", L"IF", L"ELSE", L"FOR", L"DO", L"WHILE", L"SWITCH", L"RETURN", L"EOL",
							L"GOTO",L"BREAK",L"CONTINUE",L"FINISHED",L"VOID",L"CASE",L"DEFAULT",L"TRY",L"THROW",L"CATCH",L"ORG",
							L"DB",L"DW",L"DT",L"LABEL",L"INLINE",L"STATIC",L"END"};

	if (tok - 1<SundCompiler::CHAR || tok - 1>SundCompiler::END)
	   return L"Unknown tok";

	return s[tok-1];
}
int Scanner::LookUp(wchar_t *s)
{
	struct commands * table = Compiler->GetTable();
	int i;
	wchar_t * p;
	p=s;
	
	for(i=0;*table[i].command;i++)
    {
		if (!wcscmp(table[i].command, s)){
			return table[i].tok;
		}
	}
	return 0;
}
/*int Scanner::LookUpType(char *s)
{
	// basic types
	char * table[] = {"char","int","byte","word","bit"};
	
	for(int i=0;i<sizeof(table)/sizeof(table[0]);i++)
    {
		if(!strcmp(table[i],s)){
			return i;
		}
	}
	return -1;
}*/
void Scanner::Revert(ScannerState state)
{
	this->prog = state.GetCode();
	wcscpy(this->token, state.GetToken());
	this->token_type = state.GetTokenType();
	this->tok = state.GetTok();
}
void Scanner::Putback(void)
{
	wchar_t * t;
	t=token;
	for(;*t;t++) prog--;
}
void Scanner::GetCodeDifference(wchar_t * p1, wchar_t * p2, wchar_t * buffer)
{
	int i=0;
	wchar_t * p = p1;
	for(p=p1;p!=p2;p++){
		buffer[i++] = *p;
	}
	buffer[i] = '\0';
}
int Scanner::IsEndOfLine()
{
	if(*prog=='\r'||*prog=='\n')
	{
		return 1;
	}
	return 0;
}
int Scanner::CountEndLine()
{ 
	int line_count = 0;
	wchar_t * p = p_buf;
	while (p != prog){
		if (*p == L'\n'){
			line_count++;
		}
		p++;
	}
	return line_count;
}
int Scanner::GetErrorPosition()
{
	// first, goto char. '#'
	int line_count = 0,i = 0;
	wchar_t temp_buf[80], temp_buf_number[80];
	wchar_t * p = prog;
	Loop:
	while (*p != L'#'){
		if (*p == L'\n'){
			line_count++;
		}
		if (p == p_buf){
			//error
			return -1;
		}
		p--;
	}
	wchar_t * save_p = p;
	// now,read #line n "filename"
	while (iswprint(*p)){
		temp_buf[i++] = *p;
		if (*p++ == ' ') 
			break;
	}
	temp_buf[i-1] = '\0';
	--p;
	if (wcscmp(temp_buf, L"#line")){
		//error
		//return -1;
		p = save_p - 1;
		i = 0;
		goto Loop;
	}
	// again, skip over white space
	while (Iswhite(*p) && *p) ++p;

	// now, read 'n'
	int j = 0;
	while (iswdigit(*p))
	{
		temp_buf_number[j++] = *p++;
	}
	temp_buf_number[j] = '\0';
	// again, skip over white space
	while (Iswhite(*p) && *p) ++p;

	wchar_t ch = '"';
	if (*p++ != ch){
		return -1;
	}
	int k = 0;
	while (*p != ch && *p){
		curr_file_name[k++] = *p++;
	}
	curr_file_name[k] = '\0';
	if (*p == '\0'){
		return -1;
	}

	return (line_count + _wtoi(temp_buf_number)-1);
}
void  Scanner::GetTokenPrefix(wchar_t * buff, wchar_t * p1, wchar_t * p2)
{
	wchar_t * p = p2;
	wchar_t * t;
	t = token;
	for (; *t; t++) p--;
	wchar_t *ptr = p1;
	int i = 0;
	for (i = 0; ptr != p; ptr++)
		buff[i++] = *ptr;

	// put null
	buff[i] = '\0';
}
int Scanner::SearchForbiddenToken(wchar_t * buff, wchar_t * token)
{
	wchar_t temp_buf[200];
	int i = 0;
	wchar_t * p = buff;
	int line_count = 0;
	Loop:
	while (*p != L'#' && *p){
		if (*p == L'\n'){
			line_count++;
		}
		p++;
	}
	if (*p == '\0'){
		return -1;
	}
	wchar_t * save_p = p;
	//checks token
	while (iswprint(*p)){
		temp_buf[i++] = *p;
		if (*p++ == ' ')
			break;
	}
	temp_buf[i-1] = '\0';
	if (wcscmp(temp_buf, token)){
		//error
		p = save_p + 1;
		i = 0;
		goto Loop;
	}
	else{
		// found token
		return 1+line_count;
	}
	return -1;
}

