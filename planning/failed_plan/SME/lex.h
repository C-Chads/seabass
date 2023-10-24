

/*
    Lex the input.
*/

#define TOK_SPACE ((void*)0)
#define TOK_NEWLINE ((void*)1)
#define TOK_STRING ((void*)2)
#define TOK_CHARLIT ((void*)3)
#define TOK_COMMENT ((void*)4)
#define TOK_MACRO ((void*)5)
#define TOK_INT_CONST ((void*)6)
#define TOK_FLOAT_CONST ((void*)7)
#define TOK_IDENT ((void*)8)
#define TOK_OPERATOR ((void*)9)
#define TOK_OCB ((void*)10)
#define TOK_CCB ((void*)11)
#define TOK_OPAREN ((void*)12)
#define TOK_CPAREN ((void*)13)
#define TOK_OBRACK ((void*)14)
#define TOK_CBRACK ((void*)15)
#define TOK_SEMIC ((void*)16)
#define TOK_UNKNOWN ((void*)17)
#define TOK_KEYWORD ((void*)18)
#define TOK_ESC_NEWLINE ((void*)19)
#define TOK_COMMA ((void*)20)
#define TOK_MACRO_OP ((void*)21)
#define TOK_INCSYS ((void*)22)
#define TOK_INCLUDE ((void*)23)
#define TOK_DEFINE ((void*)24)
#define TOK_UNDEF ((void*)25)
#define TOK_GUARD ((void*)26)

LIBMIN_UINT atou_hex(char* s){
LIBMIN_UINT retval = 0;
while(*s && mishex(*s)){
		retval *= 16;
		switch(*s){
			
			default: return retval;
			case 'a':case 'A': retval += 10; break;
			case 'b':case 'B': retval += 11; break;
			case 'c':case 'C': retval += 12; break;
			case 'd':case 'D': retval += 13; break;
			case 'e':case 'E': retval += 14; break;
			case 'f':case 'F': retval += 15; break;
			case '0': break;
			case '1': retval+=1; break;
			case '2': retval+=2; break;
			case '3': retval+=3; break;
			case '4': retval+=4; break;
			case '5': retval+=5; break;
			case '7': retval+=7; break;
			case '8': retval+=8; break;
			case '9': retval+=9; break;
		}
		s++;
	}
	return retval;
}

static long strll_len(strll* head){
	long len = 1;
	if(!head) return 0;
	while(head->right) {head = head->right; len++;}
	return len;
}

static void strll_append(strll* list, strll* list2){
	if(!list) return;
	while(list->right) list = list->right;
	list->right = list2;
	return;
}



static void strll_linearize(strll* node){
	strll* work;
	strll* place; /*What we place right*/

	top:;
	{
		/*Step 0: Do we return?*/
		if(!node) return;
		
		work = node;
		place = NULL;
		/*STEP 1: do we have a child or left pointer?*/
		if(node->child) {
			place = node->child;
			node->child = NULL;
		} else if(node->left){
			place = node->left;
			node->left = NULL;
		}
		/*STEP 2: Do we have a node to place? Place it at the end and start over.*/
		if(place){
			while(work->right) work = work->right;
			work->right = place;
			goto top;
		}
		/*STEP 3: Go right, repeat this.*/
		node = node->right;
		goto top;
	}
}

static void strll_free(strll* root, char free_root){
	strll* work;
	strll* next;

	if(root == NULL) return;
	/*Step 1: Linearize the tree.
	Since I never use left or child, this is un-needed.
	*/
	/*strll_linearize(root);*/
	work = root->right;
	next = NULL;
	/*Step 2: Walk down the tree, freeing pointers as we go.*/
	while(work){
		if(work->text){free(work->text);}
		next = work->right;
		free(work);
		work = next;
	}

	if(root->text) {
		free(root->text);
		root->text = NULL;
	}
	if(free_root) free(root);
	return;
	
}

/*moves a list node over-top of a node. 
DOES NOT copy the list*/
static void strll_replace_node_with_list(
	strll* replaceme, strll* list,
	char free_list
){
	strll* r;

	r = replaceme->right;
	*replaceme = *list;
	list->right = NULL;
	strll_free(r,1); 
	/*Free the stuff to the right of ol' replacement.*/
	if(free_list)strll_free(list,1);
}

/*Valid identifier character?*/
static char isUnusual(char x){
	if(isalnum(x)) return 0;
	if(x == '_') return 0;
		return 1;
}
static char isPartOfPair(char x){
	if(
		x == '(' || x == ')' ||
		x == '{' || x == '}' ||
		x == '[' || x == ']'
	)
		return 1;
	return 0;
}

static char my_isoct(char c){
	if(c == '0') return 1;
	if(c == '1') return 1;
	if(c == '2') return 1;
	if(c == '3') return 1;
	if(c == '4') return 1;
	if(c == '5') return 1;
	if(c == '6') return 1;
	if(c == '7') return 1;
	return 0;
}

static char my_isdigit(char c){
	if(c == '0') return 1;
	if(c == '1') return 1;
	if(c == '2') return 1;
	if(c == '3') return 1;
	if(c == '4') return 1;
	if(c == '5') return 1;
	if(c == '6') return 1;
	if(c == '7') return 1;
	if(c == '8') return 1;
	if(c == '9') return 1;
	return 0;
}

static char my_ishex(char c){
	if(c == '0') return 1;
	if(c == '1') return 1;
	if(c == '2') return 1;
	if(c == '3') return 1;
	if(c == '4') return 1;
	if(c == '5') return 1;
	if(c == '6') return 1;
	if(c == '7') return 1;
	if(c == '8') return 1;
	if(c == '9') return 1;
	if(c == 'A') return 1;
	if(c == 'a') return 1;
	if(c == 'B') return 1;
	if(c == 'b') return 1;
	if(c == 'C') return 1;
	if(c == 'c') return 1;
	if(c == 'D') return 1;
	if(c == 'd') return 1;
	if(c == 'E') return 1;
	if(c == 'e') return 1;
	if(c == 'F') return 1;
	if(c == 'f') return 1;
	return 0;
}


//We don't use includes or macros, so this should be very easy!

strll lex_input(char* text){
    strll retval;
    memset(&retval, 0, sizeof(strll));
    retval.text = text;
    
    strll* original_passin;
	const char* STRING_BEGIN = 		"\"";
	const char* STRING_END = 		"\"";
	const char* CHAR_BEGIN = 		"\'";
	const char* CHAR_END = 			"\'";
	/*C-style comments.*/
	const char* COMMENT_BEGIN = 	"/*";
	const char* COMMENT_END = 		"*/";
	const char* LINECOMMENT_BEGIN = 	"//";
	const char* LINECOMMENT_END = 	"\n";
	const char* INCLUDE_OPEN_SYS = "<";
	const char* INCLUDE_CLOSE_SYS = ">";
	long mode; 
	long i;
	strll* current_meta;
	strll* father;
	long line_num = 1;
	long col_num = 1;
	char* c;
	unsigned long entire_input_file_len = 0;
	strll tokenz = {0};

	char* temp_include_manip;
    	
	original_passin = &retval;
	strll* work = original_passin;
    //TODO: lex the input. Keep track of state. It's basically just a clone of
    //ctok, but without any of the include or define stuff... and I'm also
    //not sure about the multi-line string literals
	/*
		FOR THE DATA POINTER OF THE STRLL ELEMENT, WE INDICATE THE KIND OF
		THING IT IS
		0 = space
		1 = newline or carriage return.
		2 = string
		3 = character literal
		4 = comment, any kind
		5 = macro
		6 = integer constant
		7 = floating point constant
		8 = identifier
		9 = operator (including question mark and colon)
		10 = {
		11 = }
		12 = (
		13 = )
		14 = [
		15 = ]
		16 = ;
		17 = unknown/invalid
		18 = keyword
		19 = escaped newline
		20 = comma
		21 = macro operator, # or ##
		22 = include <>
		23 = #include
		24 = #define
		25 = #undef
		26 = #guard myGuardName
	*/
	
	/*We use comma operator here to do multiple assignments in a single statement*/
	for( (i=0) , (mode=-1) ;work->text[i] != '\0'; i++){
		done_selecting_mode:;
		if(work->text[i] == '\0') break;
		if(((unsigned char)work->text[i]) == 255){
			work->text[i] = '\0';
			break;
		}
		/*Allow line numbers to be printed.*/
//#include
		if(mode == -1){/*Determine what our next mode should be.*/
			if(i != 0){
				puts("Something quite unusual has happened... mode is negative one, but i is not zero!");
				exit(1);
			}
			if(
				strprefix("\r\n",work->text)||
				strprefix("\n\r",work->text)
			){
				i = -1;
				work->data = (void*)1;
				work = consume_bytes(work, 2);
				continue;
			}
			if(strprefix("\n",work->text)){
				i = -1;
				work->data = (void*)1;
				work = consume_bytes(work, 1);
				continue;
			}
			if(strfind(work->text, STRING_BEGIN) == 0){
				mode = 2;
				i+= strlen(STRING_BEGIN);
				goto done_selecting_mode;
			}
			if(strfind(work->text, COMMENT_BEGIN) == 0){
				mode = 3;
				i+= strlen(COMMENT_BEGIN);
				goto done_selecting_mode;
			}
			if(strfind(work->text, CHAR_BEGIN) == 0){
				mode = 4;
				i+= strlen(CHAR_BEGIN);
				goto done_selecting_mode;
			}
			if(strprefix(LINECOMMENT_BEGIN, work->text)){
				mode = 6;
				i+=strlen(LINECOMMENT_BEGIN); 
				goto done_selecting_mode;
			}


			/*
			if(strprefix("#", work->text)){
				mode = 6;
				i++;
				goto done_selecting_mode;
			}
			*/
			if(my_isdigit(work->text[0])){
				if(
					strprefix("0x",work->text) || 
					strprefix("0X",work->text)
				)
				{
					i+=2;
					mode = 8; /*hex literal*/
					goto done_selecting_mode;
				}
				if(work->text[0] == '0'){
					i++;
					mode = 9; /*octal literal*/
					goto done_selecting_mode;
				}
				i++; mode = 7; goto done_selecting_mode; /*decimal literal*/
			}
			if(isspace(work->text[i]) && work->text[i] != '\n'){
				mode = 0;
				i++;
				goto done_selecting_mode;
			}

			/*Three cases of escaped newlines.*/
			if(work->text[i] == '\\' && work->text[i+1] == '\n'){
				work->data = (void*)19;
				work = consume_bytes(work, 2);
				i = -1;
				continue;
			}
			if(
				work->text[i] == '\\' &&
				work->text[i+1] == '\r'&&
				work->text[i+2] == '\n'
			){
				work->data = (void*)19;
				work = consume_bytes(work, 3);
				i = -1;
				continue;
			}
			if(
				work->text[i] == '\\' &&
				work->text[i+1] == '\n'&&
				work->text[i+2] == '\r'
			){
				work->data = (void*)19;
				work = consume_bytes(work, 3);
				i = -1;
				continue;
			}

			/*-> operator*/
			if(
				work->text[i] == '-' &&
				work->text[i+1] == '>'
			){
				work->data = (void*)9;
				work = consume_bytes(work, 2);
				i = -1;
				continue;
			}

			/*java equals operator*/
			if(
				work->text[i] == '=' &&
				work->text[i+1] == '=' &&
				work->text[i+2] == '='
			){
				work->data = (void*)9;
				work = consume_bytes(work, 3);
				i = -1;
				continue;
			}

			/*-- operator*/
			if(
				work->text[i] == '-' &&
				work->text[i+1] == '-'
			){
				work->data = (void*)9;
				work = consume_bytes(work, 2);
				i = -1;
				continue;
			}
			/*:= operator*/
			if(
				work->text[i] == ':' &&
				work->text[i+1] == '='
			){
				work->data = (void*)9;
				work = consume_bytes(work, 2);
				i = -1;
				continue;
			}
			/*.& operator*/
			if(
				work->text[i] == '.' &&
				work->text[i+1] == '&'
			){
				work->data = (void*)9;
				work = consume_bytes(work, 2);
				i = -1;
				continue;
			}

			/*++ operator*/
			if(
				work->text[i] == '+' &&
				work->text[i+1] == '+'
			){
				work->data = (void*)9;
				work = consume_bytes(work, 2);
				i = -1;
				continue;
			}

			/*&& operator*/
			if(
				work->text[i] == '&' &&
				work->text[i+1] == '&'
			){
				work->data = (void*)9;
				work = consume_bytes(work, 2);
				i = -1;
				continue;
			}

			/*|| operator*/
			if(
				work->text[i] == '|' &&
				work->text[i+1] == '|'
			){
				work->data = (void*)9;
				work = consume_bytes(work, 2);
				i = -1;
				continue;
			}


			/*>> operator*/
			if(
				work->text[i] == '>' &&
				work->text[i+1] == '>'
			){
				work->data = (void*)9;
				work = consume_bytes(work, 2);
				i = -1;
				continue;
			}

			/*<< operator*/
			if(
				work->text[i] == '<' &&
				work->text[i+1] == '<'
			){
				work->data = (void*)9;
				work = consume_bytes(work, 2);
				i = -1;
				continue;
			}

			/*<= operator*/
			if(
				work->text[i] == '<' &&
				work->text[i+1] == '='
			){
				work->data = (void*)9;
				work = consume_bytes(work, 2);
				i = -1;
				continue;
			}

			/*>= operator*/
			if(
				work->text[i] == '>' &&
				work->text[i+1] == '='
			){
				work->data = (void*)9;
				work = consume_bytes(work, 2);
				i = -1;\
				continue;
			}

			/*== operator*/
			if(
				work->text[i] == '=' &&
				work->text[i+1] == '='
			){
				work->data = (void*)9;
				work = consume_bytes(work, 2);
				i = -1;
				continue;
			}

			/*!= operator*/
			if(
				work->text[i] == '!' &&
				work->text[i+1] == '='
			){
				work->data = (void*)9;
				work = consume_bytes(work, 2);
				i = -1;
				continue;
			}

			/*Single unusual byte*/
			if(isUnusual(work->text[i])){ /*Merits its own thing.*/
				work->data = (void*)17;
				if(work->text[i] == '~')work->data = (void*)9;
				if(work->text[i] == ':')work->data = (void*)9;
				if(work->text[i] == '!')work->data = (void*)9;
				if(work->text[i] == '.')work->data = (void*)9;
				if(work->text[i] == '&')work->data = (void*)9;
				if(work->text[i] == '*')work->data = (void*)9;
				if(work->text[i] == '+')work->data = (void*)9;
				if(work->text[i] == '-')work->data = (void*)9;
				if(work->text[i] == '/')work->data = (void*)9;
				if(work->text[i] == '%')work->data = (void*)9;
				if(work->text[i] == '<')work->data = (void*)9;
				if(work->text[i] == '>')work->data = (void*)9;
				if(work->text[i] == '^')work->data = (void*)9;
				if(work->text[i] == '|')work->data = (void*)9;
				if(work->text[i] == '?')work->data = (void*)9;
				if(work->text[i] == '=')work->data = (void*)9;
				if(work->text[i] == '@')work->data = (void*)9; /*parser hook.*/
				if(work->text[i] == ';')work->data = (void*)16;
				if(work->text[i] == '{')work->data = (void*)10;
				if(work->text[i] == '}')work->data = (void*)11;
				if(work->text[i] == '(')work->data = (void*)12;
				if(work->text[i] == ')')work->data = (void*)13;
				if(work->text[i] == '[')work->data = (void*)14;
				if(work->text[i] == ']')work->data = (void*)15;
				if(work->text[i] == ',')work->data = (void*)20;
				if(work->text[i] == '#')work->data = (void*)21;
				if(work->text[i] == '\n'){work->data = (void*)1;}
				if(work->text[i] == '\r'){work->data = (void*)1;}
				work = consume_bytes(work, 1);
				i = -1;
				continue;
			}
			mode = 1; 
		}
		
		
		if(mode == 0){ /*reading whitespace.*/
			work->data = 0;
			if(isspace(work->text[i]) && work->text[i]!='\n') continue;
			work = consume_bytes(work, i); i=-1; mode = -1; continue;
		}
		if(mode == 1){ /*valid identifier*/
			work->data = (void*)8;
			if(
				isUnusual(work->text[i])
			)
			{
				work = consume_bytes(work, i); 
				i = -1; mode = -1; continue;
			}
		}
		if(mode == 7){ /*Decimal literal, might turn into float*/
			work->data = (void*)6;
			if(work->text[i] == '.')
			{
				mode = 11; /*Begin parsing decimal fractional portion*/
				work->data = (void*)7;
				continue;
			}
			if(work->text[i] == 'E' || work->text[i] == 'e'){
				if(work->text[i+1] == '-' || work->text[i+1] == '+') 
					i++;
				mode = 10; /*Begin parsing the E portion.*/
				work->data = (void*)7;
				i++;
				goto done_selecting_mode;
			}
			if(
				!my_isdigit(work->text[i])
			)
			{
				
				work = consume_bytes(work, i); 
				i = -1; mode = -1; continue;
			}
		}
		if(mode == 8){ /*Hex literal*/
			work->data = (void*)6;
			if(
				!my_ishex(work->text[i])
			)
			{
				work = consume_bytes(work, i); 
				i = -1; mode = -1; continue;
			}
		}
		if(mode == 9){ /*Octal Literal*/
			work->data = (void*)6;
			if(
				!my_isoct(work->text[i])
			)
			{
				if(my_isdigit(work->text[i]))
				{
					mode = 7; /*This is a decimal literal, actually*/
					continue;
				}
				/*allow 0e-3*/
				if(work->text[i] == 'E' || work->text[i] == 'e'){
					if(work->text[i+1] == '-' || work->text[i+1] == '+') 
						i++;
					mode = 10; /*Begin parsing the E portion.*/
					work->data = (void*)7;
					i++;
					goto done_selecting_mode;
				}
				if(work->text[i] == '.')
				{
					mode = 11; /*Begin parsing decimal fractional portion*/
					work->data = (void*)7;
					continue;
				}
				work = consume_bytes(work, i); 
				i = -1; mode = -1; continue;
			}
		}
		if(mode == 10){ /*after E*/
			work->data = (void*)7;
			if(!my_isdigit(work->text[i])){
				work->data = (void*)7;
				work = consume_bytes(work, i); 
				i = -1; mode = -1; continue;
			}
		}
		if(mode == 11){ /*After the radix of a float*/
			work->data = (void*)7;
			if(work->text[i] == 'E' || work->text[i] == 'e'){
				if(work->text[i+1] == '-' || work->text[i+1] == '+')
					i++;
				mode = 10; /*Begin parsing the E portion.*/
				i++;
				goto done_selecting_mode;
			}
			if(!my_isdigit(work->text[i])){
				work = consume_bytes(work, i); 
				i = -1; mode = -1; continue;
			}
		}
		if(mode == 2){ /*string.*/
			work->data = (void*)2;
			if(work->text[i] == '\\' && work->text[i+1] != '\0') {i++; continue;}
			if(strfind(work->text + i, STRING_END) == 0){
				i+=strlen(STRING_END);
				work = consume_bytes(work, i); i = -1; mode = -1; continue;
			}
		}
		if(mode == 3){ /*comment.*/
			work->data = (void*)4;
			/*if(work->text[i] == '\\' && work->text[i+1] != '\0') {i++; continue;}*/
			if(strfind(work->text + i, COMMENT_END) == 0){
				i+=strlen(COMMENT_END);

				work = consume_bytes(work, i); i = -1; mode = -1; continue;
			}
		}
		if(mode == 4){ /*char literal.*/
			work->data = (void*)3;
			if(work->text[i] == '\\' && work->text[i+1] != '\0') {i++; continue;}
			if(strfind(work->text + i, CHAR_END) == 0){
				i+=strlen(CHAR_END);
				work = consume_bytes(work, i); i = -1; mode = -1; continue;
			}
		}
		if(mode == 6){ /*line comment*/
			work->data = (void*)4;
			if(strfind(work->text + i, LINECOMMENT_END) == 0){
				/*i+=strlen(LINECOMMENT_END);*/

				work = consume_bytes(work, i); 
				i = -1; 
				mode = -1; continue;
			}
		}
	} /*eof main tokenizer*/

	/*Do line numbers*/
	/*
	printf("<DEBUG> len = %ld\r\n",strll_len(original_passin));
	strll_show(original_passin,0);
	*/
	{
		line_num = 1;
		col_num = 1;
		for(
			current_meta = original_passin;
			current_meta != NULL ;
			(current_meta = current_meta->right)
		){
			current_meta->linenum = line_num;
			current_meta->colnum = col_num;
			current_meta->filename = 0; /*So it knows what file it's part of.*/
			if(current_meta->text != NULL){
				c = current_meta->text;
				for(;*c;c++){
					if(*c == '\n') {line_num++;col_num = 1;continue;}
					if(*c != '\n') {col_num++;}
				}
			}
		}
	}

	/*Transform characters.*/
	{
		
		father = NULL;
		current_meta = original_passin;
		for(;
		current_meta != NULL 
		&& current_meta->text != NULL; 
			/*Increment. We use the comma operator to do multiple assignments in a single expression.*/
			(father = current_meta),
			(current_meta = current_meta->right)

		){
			if(current_meta->data != TOK_STRING)
			if(current_meta->text){
				/*Recognize keywords*/
				if(
					streq(current_meta->text, "fn")|| /*Extension*/
					streq(current_meta->text, "cast")|| /*Extension*/
					streq(current_meta->text, "r8")|| /*Extension*/
					streq(current_meta->text, "r16")|| /*Extension*/
					streq(current_meta->text, "r32")|| /*Extension*/
					streq(current_meta->text, "r64")|| /*Extension*/
					streq(current_meta->text, "r128")|| /*Extension*/
					streq(current_meta->text, "r256")|| /*Extension*/
					streq(current_meta->text, "u8")|| /*Extension*/
					streq(current_meta->text, "i8")|| /*Extension*/
					streq(current_meta->text, "u16")|| /*Extension*/
					streq(current_meta->text, "i16")|| /*Extension*/
					streq(current_meta->text, "u32")|| /*Extension*/
					streq(current_meta->text, "i32")|| /*Extension*/
					streq(current_meta->text, "u64")|| /*Extension*/
					streq(current_meta->text, "i64")|| /*Extension*/
					streq(current_meta->text, "f32")|| /*Extension*/
					streq(current_meta->text, "f64")|| /*Extension*/
					streq(current_meta->text, "char")|| /*Extension*/
					streq(current_meta->text, "uchar")|| /*Extension*/
					streq(current_meta->text, "schar")|| /*Extension*/
					streq(current_meta->text, "byte")|| /*Extension*/
					streq(current_meta->text, "ubyte")|| /*Extension*/
					streq(current_meta->text, "sbyte")|| /*Extension*/
					streq(current_meta->text, "uint")|| /*Extension*/
					streq(current_meta->text, "int")|| /*Extension*/
					streq(current_meta->text, "sint")|| /*Extension*/
					streq(current_meta->text, "long")|| /*Extension*/
					streq(current_meta->text, "slong")|| /*Extension*/
					streq(current_meta->text, "ulong")|| /*Extension*/
					streq(current_meta->text, "llong")|| /*Extension*/
					streq(current_meta->text, "ullong")|| /*Extension*/
					streq(current_meta->text, "qword")|| /*Extension*/
					streq(current_meta->text, "uqword")|| /*Extension*/
					streq(current_meta->text, "sqword")|| /*Extension*/
					streq(current_meta->text, "float")|| /*Extension*/
					streq(current_meta->text, "double")|| /*Extension*/
					streq(current_meta->text, "short")|| /*Extension*/
					streq(current_meta->text, "ushort")|| /*Extension*/
					streq(current_meta->text, "sshort")|| /*Extension*/
					streq(current_meta->text, "break")||
					streq(current_meta->text, "data")||
					streq(current_meta->text, "string")|| /*used for the data statement.*/
					streq(current_meta->text, "end")|| /*We're doing lua-style syntax!*/
					streq(current_meta->text, "continue")||
					streq(current_meta->text, "if")||
					streq(current_meta->text, "elif")||
					streq(current_meta->text, "elseif")||
					streq(current_meta->text, "else")||
					streq(current_meta->text, "while") ||
					streq(current_meta->text, "for") ||
					streq(current_meta->text, "goto")||
					streq(current_meta->text, "jump")||
					streq(current_meta->text, "switch")|| /*very different from C, optimized jump table*/
					streq(current_meta->text, "return")|| /*same as C*/
					streq(current_meta->text, "tail")|| /*tail call*/
					streq(current_meta->text, "sizeof")|| /*get size of expression*/
					streq(current_meta->text, "static")|| /*Static storage. Not the C usage.*/
					streq(current_meta->text, "pub")|| /*used to declare as exporting.*/
					streq(current_meta->text, "public")|| /*used to declare as exporting.*/
					streq(current_meta->text, "predecl")|| /*used to predeclare*/
					streq(current_meta->text, "struct")||
					streq(current_meta->text, "class")|| 
					streq(current_meta->text, "method")||
					streq(current_meta->text, "codegen")||
					streq(current_meta->text, "constexpri")|| 
					streq(current_meta->text, "constexprf")|| 
					streq(current_meta->text, "pure")|| /*enforce purity.*/
					streq(current_meta->text, "inline")|| /*inline*/
					streq(current_meta->text, "atomic")|| /*qualifier*/
					streq(current_meta->text, "volatile")|| /*qualifier*/
					streq(current_meta->text, "getfnptr")|| /*gets a function pointer.*/
					streq(current_meta->text, "callfnptr")|| /*calls a function pointer.*/
					streq(current_meta->text, "getglobalptr")|| /*gets pointer to global*/
					streq(current_meta->text, "asm") /*extension*/
					/*Builtins for metaprogramming.*/
				){
					current_meta->data = (void*)18;
				}

				if(streq(current_meta->text, "streq"))current_meta->data = TOK_OPERATOR;
				if(streq(current_meta->text, "strneq"))current_meta->data = TOK_OPERATOR;

				if(streq(current_meta->text, "eq"))current_meta->data = TOK_OPERATOR;
				if(streq(current_meta->text, "neq"))current_meta->data = TOK_OPERATOR;
				/*Delete escaped newlines and spaces.*/
				if(
					current_meta->data == (void*)19 || /*MAGIC escaped newline*/
					current_meta->data == (void*)0x4 || /*MAGIC comment*/
					current_meta->data == (void*)0   /*MAGIC space*/
				){
					free(current_meta->text);
					current_meta->text = NULL;
					/*Take our right pointer and assign it to our father, then, free ourselves.*/
					if(father){
						father->right = current_meta->right;
						current_meta->right = NULL;
						free(current_meta);
						current_meta = father; /*wink*/
					} else {
						/*We are the first element in the list. This is actually an interesting situation.
						if we are also the last in the list, then this means that the user has an empty text
						file with only ignore-able chracters in it.

						In that circumstance, we do nothing.

						However, under normal circumstances, there is something to the right of us,
						so we should turn us into that, and proceed.
						*/
						if(current_meta->right){
							strll* r = current_meta->right;
							memcpy(current_meta, current_meta->right, sizeof(strll));
							r->right = NULL;
							
							free(r);
						} else current_meta->data = (void*)0xff; /*MAGIC empty_file*/
					}
				}
				/*Free un-needed text..*/
				if(current_meta->data == (void*)0 ||
					current_meta->data == (void*)1 ||
					current_meta->data == (void*)10 ||
					current_meta->data == (void*)11 ||
					current_meta->data == (void*)12 ||
					current_meta->data == (void*)13 ||
					current_meta->data == (void*)14 ||
					current_meta->data == (void*)15 ||
					current_meta->data == (void*)16 ||
					current_meta->data == (void*)20
				){
					free(current_meta->text);
					current_meta->text = NULL;
				}
				/*make all operators unified.*/

			} /*eof if(current_meta->text)*/


		} /*eof for*/
	} /*Eof transform characters*/


    return retval;
}

