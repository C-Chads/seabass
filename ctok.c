#include "stringutil.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
/*for realpath*/

#ifdef WINDOWS
#error "Operating system not supported- need to have function `realpath` implemented!!!"
#endif
#include <limits.h>

#include "targspecifics.h"
#include "libmin.h"
#include "data.h"



/*This is a convenient example.*/
/*//this is another example*/
//#include
//#include "ast.h"

/*
	System include directory.
*/

const char* sys_include_dir = "/usr/include/cbas/";

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

#define MAX_FILES 4096
char* filenames[MAX_FILES];
char* guards[MAX_FILES];
unsigned long nfilenames = 0;
unsigned long nguards = 0;
/*see parser.c for this one...*/
void compile_unit(strll* _unit);


static char* filepath_to_directory(char* infilename){
	char* retval;
	unsigned long len;
	
	{
    	retval = realpath(infilename,NULL);
    	if(!retval) return NULL;
    	len = strlen(retval);
    	len--;
    	while(retval[len] != '/' &&
    	retval[len] != '\\'){
    		retval[len] = '\0';
    		len--;	
    	}
    	return retval;
	}
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

static void strll_show(strll* current, long lvl){
	long i;
	{
		for(;current != NULL; current = current->right){
			{
				if(current->filename){
					printf(
						"File: %s| Line Number: %lu | Column Number: %lu |", 
						current->filename, 
						current->linenum, 
						current->colnum
					);
				}
				if(current->data == (void*)1)
					printf("<newline/>\n");
				else if(current->data == (void*)0){
					printf("<space/>\n");
				}else if(current->data == (void*)2)
					printf("<string len=%u>%s</string>\n", (unsigned)strlen(current->text),current->text);
				else if(current->data == (void*)3)
					printf("<char len=%u>%s</char>\n", (unsigned)strlen(current->text),current->text);
				else if(current->data == (void*)4)
					printf("<comment len=%u>%s</commment>\n", (unsigned)strlen(current->text),current->text);
				else if(current->data == (void*)5)
					printf("<macro len=%u>%s</macro>\n", (unsigned)strlen(current->text),current->text);
				else if(current->data == (void*)6)
					printf("<int len=%u>%s</int>\n", (unsigned)strlen(current->text),current->text);
				else if(current->data == (void*)7)
					printf("<float len=%u>%s</float>\n", (unsigned)strlen(current->text),current->text);
				else if(current->data == (void*)8)
					printf("<ident len=%u>%s</ident>\n", (unsigned)strlen(current->text),current->text);
				else if(current->data == (void*)18)
					printf("<keyw len=%u>%s</keyw>\n", (unsigned)strlen(current->text),current->text);
				else if(current->data == (void*)9)
					printf("<op len=%u>%s</op>\n", (unsigned)strlen(current->text),current->text);
				else if(current->data == (void*)10)
					printf("</ocb>\n");
				else if(current->data == (void*)11)
					printf("</ccb>\n");
				else if(current->data == (void*)12)
					printf("</opar>\n");
				else if(current->data == (void*)13)
					printf("</cpar>\n");
				else if(current->data == (void*)14)
					printf("</obrac>\n");
				else if(current->data == (void*)15)
					printf("</cbrac>\n");
				else if(current->data == (void*)16)
					printf("</semic>\n");
				else if(current->data == (void*)19)
					printf("</ign>\n");	
				else if(current->data == (void*)20)
					printf("</,>\n");
				else if(current->data == (void*)21)
					printf("<mop len=%u>%s</mop>\n",(unsigned)strlen(current->text),current->text);
				else if(current->data == (void*)22)
					printf("<incsys len=%u>%s</incsys>\n",(unsigned)strlen(current->text),current->text);
				else if(current->data == (void*)23)
					printf("</include>\n");
				else if(current->data == (void*)24)
					printf("</define>\n");
				else if(current->data == (void*)25)
					printf("</undef>\n");
				else
					printf("<error len=%u>%s</TK>\n", (unsigned)strlen(current->text),current->text);
				
			}
			/*
			if(current->left)
			{	for(i = 0; i < lvl; i++) printf("\t");
				printf("<LC>\n");
				strll_show(current->left, lvl + 1);
				for(i = 0; i < lvl; i++) printf("\t");
				printf("</LC>\n");
			}
			if(current->child)
			{	for(i = 0; i < lvl; i++) printf("\t");
				printf("<C>\n");
				strll_show(current->child, lvl + 1);
				for(i = 0; i < lvl; i++) printf("\t");
				printf("</C>\n");
			}
			*/
		}
	}
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

#define MAX_INCLUDE_LEVEL 20

static void tokenizer(
	strll* work, 
	const char* filename,
	long include_level
){
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
	char* text_store = work->text;
	long mode; 
	long i;
	strll* current_meta;
	strll* father;
	long line_num = 1;
	long col_num = 1;
	char* c;
	char is_parsing_include = 0;
	unsigned long entire_input_file_len = 0;
	int guard_exists = 0;
	strll* guard_statement;
	strll* guard_ident;
	char* file_to_include = NULL;
	FILE* f;
	strll* right_of_include_statement;
	strll tokenz = {0};

	char* temp_include_manip;
	
	original_passin = work;
	
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
				is_parsing_include = 0; /*We are definitely not parsing an include statement anymore!*/
				work->data = (void*)1;
				work = consume__bytes_with_bigstore(work, 2);
				continue;
			}
			if(strprefix("\n",work->text)){
				i = -1;
				is_parsing_include = 0; /*We are definitely not parsing an include statement anymore!*/
				work->data = (void*)1;
				work = consume__bytes_with_bigstore(work, 1);
				continue;
			}
			if(is_parsing_include)
			if(strprefix(INCLUDE_OPEN_SYS, work->text)){
				mode = 12;
				i+=strlen(INCLUDE_OPEN_SYS);
				goto done_selecting_mode;
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

			if(!is_parsing_include){
				if(strprefix("#include",work->text)){
					work->data = (void*)23;
					work = consume__bytes_with_bigstore(work, 8);
					i = -1;
					is_parsing_include = 1;
					continue;
				}
				if(strprefix("#define", work->text)){
					work->data = (void*)24;
					work = consume__bytes_with_bigstore(work, 7);
					i = -1;
					is_parsing_include = 0;
					continue;
				}
				if(strprefix("#undef", work->text)){
					work->data = (void*)25;
					work = consume__bytes_with_bigstore(work, 6);
					i = -1;
					is_parsing_include = 0;
					continue;
				}
				if(strprefix("#guard", work->text)){
					work->data = (void*)26;
					work = consume__bytes_with_bigstore(work, 6);
					i = -1;
					is_parsing_include = 0;
					continue;
				}
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
				work = consume__bytes_with_bigstore(work, 2);
				i = -1;
				continue;
			}
			if(
				work->text[i] == '\\' &&
				work->text[i+1] == '\r'&&
				work->text[i+2] == '\n'
			){
				work->data = (void*)19;
				work = consume__bytes_with_bigstore(work, 3);
				i = -1;
				continue;
			}
			if(
				work->text[i] == '\\' &&
				work->text[i+1] == '\n'&&
				work->text[i+2] == '\r'
			){
				work->data = (void*)19;
				work = consume__bytes_with_bigstore(work, 3);
				i = -1;
				continue;
			}

			/*-> operator*/
			if(
				work->text[i] == '-' &&
				work->text[i+1] == '>'
			){
				work->data = (void*)9;
				work = consume__bytes_with_bigstore(work, 2);
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
				work = consume__bytes_with_bigstore(work, 3);
				i = -1;
				continue;
			}

			/*-- operator*/
			if(
				work->text[i] == '-' &&
				work->text[i+1] == '-'
			){
				work->data = (void*)9;
				work = consume__bytes_with_bigstore(work, 2);
				i = -1;
				continue;
			}
			/*:= operator*/
			if(
				work->text[i] == ':' &&
				work->text[i+1] == '='
			){
				work->data = (void*)9;
				work = consume__bytes_with_bigstore(work, 2);
				i = -1;
				continue;
			}
			/*.& operator*/
			if(
				work->text[i] == '.' &&
				work->text[i+1] == '&'
			){
				work->data = (void*)9;
				work = consume__bytes_with_bigstore(work, 2);
				i = -1;
				continue;
			}

			/*++ operator*/
			if(
				work->text[i] == '+' &&
				work->text[i+1] == '+'
			){
				work->data = (void*)9;
				work = consume__bytes_with_bigstore(work, 2);
				i = -1;
				continue;
			}

			/*&& operator*/
			if(
				work->text[i] == '&' &&
				work->text[i+1] == '&'
			){
				work->data = (void*)9;
				work = consume__bytes_with_bigstore(work, 2);
				i = -1;
				continue;
			}

			/*|| operator*/
			if(
				work->text[i] == '|' &&
				work->text[i+1] == '|'
			){
				work->data = (void*)9;
				work = consume__bytes_with_bigstore(work, 2);
				i = -1;
				continue;
			}


			/*>> operator*/
			if(
				work->text[i] == '>' &&
				work->text[i+1] == '>'
			){
				work->data = (void*)9;
				work = consume__bytes_with_bigstore(work, 2);
				i = -1;
				continue;
			}

			/*<< operator*/
			if(
				work->text[i] == '<' &&
				work->text[i+1] == '<'
			){
				work->data = (void*)9;
				work = consume__bytes_with_bigstore(work, 2);
				i = -1;
				continue;
			}

			/*<= operator*/
			if(
				work->text[i] == '<' &&
				work->text[i+1] == '='
			){
				work->data = (void*)9;
				work = consume__bytes_with_bigstore(work, 2);
				i = -1;
				continue;
			}

			/*>= operator*/
			if(
				work->text[i] == '>' &&
				work->text[i+1] == '='
			){
				work->data = (void*)9;
				work = consume__bytes_with_bigstore(work, 2);
				i = -1;\
				continue;
			}

			/*== operator*/
			if(
				work->text[i] == '=' &&
				work->text[i+1] == '='
			){
				work->data = (void*)9;
				work = consume__bytes_with_bigstore(work, 2);
				i = -1;
				continue;
			}

			/*!= operator*/
			if(
				work->text[i] == '!' &&
				work->text[i+1] == '='
			){
				work->data = (void*)9;
				work = consume__bytes_with_bigstore(work, 2);
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
				if(work->text[i] == ';')work->data = (void*)16; //SEMICOLON- 16
				if(work->text[i] == '{')work->data = (void*)10;
				if(work->text[i] == '}')work->data = (void*)11;
				if(work->text[i] == '(')work->data = (void*)12;
				if(work->text[i] == ')')work->data = (void*)13;
				if(work->text[i] == '[')work->data = (void*)14;
				if(work->text[i] == ']')work->data = (void*)15;
				if(work->text[i] == ',')work->data = (void*)20;
				if(work->text[i] == '#')work->data = (void*)21;
				if(work->text[i] == '\n'){work->data = (void*)1;is_parsing_include = 0;}
				if(work->text[i] == '\r'){work->data = (void*)1;is_parsing_include = 0;}
				work = consume__bytes_with_bigstore(work, 1);
				i = -1;
				continue;
			}
			mode = 1; 
		}
		if(mode == 0){ /*reading whitespace.*/
			work->data = 0;
			if(isspace(work->text[i]) && work->text[i]!='\n') continue;
			work = consume__bytes_with_bigstore(work, i); i=-1; mode = -1; continue;
		}
		if(mode == 1){ /*valid identifier*/
			work->data = (void*)8;
			if(
				isUnusual(work->text[i])
			)
			{
				work = consume__bytes_with_bigstore(work, i); 
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
				
				work = consume__bytes_with_bigstore(work, i); 
				i = -1; mode = -1; continue;
			}
		}
		if(mode == 8){ /*Hex literal*/
			work->data = (void*)6;
			if(
				!my_ishex(work->text[i])
			)
			{
				work = consume__bytes_with_bigstore(work, i); 
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
				work = consume__bytes_with_bigstore(work, i); 
				i = -1; mode = -1; continue;
			}
		}
		if(mode == 10){ /*after E*/
			work->data = (void*)7;
			if(!my_isdigit(work->text[i])){
				work->data = (void*)7;
				work = consume__bytes_with_bigstore(work, i); 
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
				work = consume__bytes_with_bigstore(work, i); 
				i = -1; mode = -1; continue;
			}
		}
		if(mode == 12){
			work->data = (void*)22;
			if(work->text[i] == '\\' && work->text[i+1] != '\0') {i++; continue;}
			if(strfind(work->text + i, INCLUDE_CLOSE_SYS) == 0){
				i+=strlen(INCLUDE_CLOSE_SYS);
				work = consume__bytes_with_bigstore(work, i); i = -1; mode = -1; continue;
			}
		}
		if(mode == 2){ /*string.*/
			work->data = (void*)2;
			if(work->text[i] == '\\' && work->text[i+1] != '\0') {i++; continue;}
			if(strfind(work->text + i, STRING_END) == 0){
				i+=strlen(STRING_END);
				work = consume__bytes_with_bigstore(work, i); i = -1; mode = -1; continue;
			}
		}
		if(mode == 3){ /*comment.*/
			work->data = (void*)4;
			/*if(work->text[i] == '\\' && work->text[i+1] != '\0') {i++; continue;}*/
			if(strfind(work->text + i, COMMENT_END) == 0){
				i+=strlen(COMMENT_END);

				work = consume__bytes_with_bigstore(work, i); i = -1; mode = -1; continue;
			}
		}
		if(mode == 4){ /*char literal.*/
			work->data = (void*)3;
			if(work->text[i] == '\\' && work->text[i+1] != '\0') {i++; continue;}
			if(strfind(work->text + i, CHAR_END) == 0){
				i+=strlen(CHAR_END);
				work = consume__bytes_with_bigstore(work, i); i = -1; mode = -1; continue;
			}
		}
		if(mode == 6){ /*line comment*/
			work->data = (void*)4;
			if(strfind(work->text + i, LINECOMMENT_END) == 0){
				/*i+=strlen(LINECOMMENT_END);*/

				work = consume__bytes_with_bigstore(work, i); 
				i = -1; 
				mode = -1; continue;
			}
		}
	} /*eof main tokenizer*/
	//The very last token is currently pointing at the middle of a buffer...
	{
	    char*tmp;
	    tmp = work->text;
	    work->text = strdup(tmp);
	    //now, free the text store!
	    free(text_store);
	}

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
			current_meta->filename = (char*)filename; /*So it knows what file it's part of.*/
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
					streq(current_meta->text, "fn")|| //CHECK
					streq(current_meta->text, "function")|| //CHECK
					streq(current_meta->text, "func")|| //CHECK
					streq(current_meta->text, "procedure")|| //CHECK
					streq(current_meta->text, "proc")|| //CHECK
					streq(current_meta->text, "cast")|| //CHECK
					streq(current_meta->text, "u8")|| //CHECK
					streq(current_meta->text, "i8")|| //CHECK
					streq(current_meta->text, "u16")|| //CHECK
					streq(current_meta->text, "i16")|| //CHECK
					streq(current_meta->text, "u32")|| //CHECK
					streq(current_meta->text, "i32")|| //CHECK
					streq(current_meta->text, "u64")|| //CHECK
					streq(current_meta->text, "i64")|| //CHECK
					streq(current_meta->text, "f32")|| //CHECK
					streq(current_meta->text, "f64")|| //CHECK
					streq(current_meta->text, "char")|| //CHECK
					streq(current_meta->text, "uchar")|| //CHECK
					streq(current_meta->text, "schar")|| //CHECK
					streq(current_meta->text, "byte")|| //CHECK
					streq(current_meta->text, "ubyte")|| //CHECK
					streq(current_meta->text, "sbyte")|| //CHECK
					streq(current_meta->text, "uint")|| //CHECK
					streq(current_meta->text, "int")|| //CHECK
					streq(current_meta->text, "sint")|| //CHECK
					streq(current_meta->text, "long")|| //CHECK
					streq(current_meta->text, "slong")|| //CHECK
					streq(current_meta->text, "ulong")|| //CHECK
					streq(current_meta->text, "llong")|| //CHECK
					streq(current_meta->text, "sllong")|| //CHECK
					streq(current_meta->text, "ullong")|| //CHECK
					streq(current_meta->text, "qword")|| //CHECK
					streq(current_meta->text, "uqword")|| //CHECK
					streq(current_meta->text, "uptr")|| //CHECK
					streq(current_meta->text, "sqword")|| //CHECK
					streq(current_meta->text, "noexport")|| //CHECK
					streq(current_meta->text, "float")|| //CHECK
					streq(current_meta->text, "double")|| //CHECK
					streq(current_meta->text, "short")|| //CHECK
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
					streq(current_meta->text, "union")||
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
					current_meta->data == (void*)16 || //yes, semicolons are deleted.
					current_meta->data == (void*)20


					/* IDEA FOR METAPROGRAMMING SAVINGS
						@ifmultitest [
							current_meta
							==
							[
								0,1,10,11
								12,13,14,15,16,20
							]
						]
					*/
				){
					free(current_meta->text);
					current_meta->text = NULL;
				}
				/*make all operators unified.*/


			} /*eof if(current_meta->text)*/


		} /*eof for*/
	} /*Eof transform characters*/

	/*Handle guards.*/
	{
		current_meta = original_passin;
		for(;
			current_meta != NULL;
			current_meta = current_meta->right
		){
			guard_exists = 0;
			if(current_meta->data == (void*)26){
				if(!current_meta->right){
					fail_guard_needs_ident:;
					puts("<TOKENIZER ERROR>");
					puts("#guard requires an IDENTIFIER to its right.");
					exit(1);
				}
				if(current_meta->right->data != (void*)8)
					goto fail_guard_needs_ident;
				guard_statement = current_meta;
				guard_ident = current_meta->right;	/*Check if the guard's identifier has already been declared.*/
				for(i = 0;i < (long)nguards;i++){
					if(streq(guard_ident->text, guards[i])){
						guard_exists = 1;
						break;
					}
				}
				if(guard_exists){
					strll_free(guard_statement->right,1);
					guard_statement->right = NULL;
					free(guard_statement->text);
					/*Turn ourselves into a newline.*/
					guard_statement->text = NULL;
					guard_statement->data = (void*)1;
				}
				if(!guard_exists){
					if(nguards >= MAX_FILES)
					{
						puts("<TOKENIZER ERROR>");
						puts("Too many guards.");
						exit(1);
					}
					guards[nguards] = strdup(guard_ident->text);
					nguards++;
					/*Assume it succeeded...*/
					guard_statement->data = (void*)1;
					guard_ident->data = (void*)1;
					free(guard_statement->text);
					free(guard_ident->text);
					guard_statement->text = NULL;
					guard_ident->text = NULL;
				}
			}
		}
	}
	/*Handle includes.*/
	{
		father = NULL;
		
		current_meta = original_passin;
			for(;current_meta != NULL; 
				/*Increment. We use the comma operator to do multiple assignments in a single expression.*/
				(father = current_meta),
				(current_meta = current_meta->right)
	
			){
				strll* include_statement;
				strll* end_of_include;
				strll* include_text;
				top:;
				/*Lex*/
				if(current_meta->data == (void*)23){
					if(!current_meta->right){
						printf("\r\n<ERROR> include without a destination... end of file? line %ld", current_meta->linenum);
						goto error;
					}
					if(current_meta->right->data != (void*)2 &&
						current_meta->right->data != (void*)22
					){
						printf("\r\n<ERROR> include without a destination. line %ld", current_meta->linenum);
						goto error;
					}
					
				}

				if(father)
				if(father->data == (void*)23){
					end_of_include = current_meta; /*current_meta is either the string literal or the incsys*/
					if(current_meta->right){
						if(current_meta->right->data != (void*)1){
							printf("\r\n<SYNTAX ERROR> include statement is not immediately followed by newline? line %ld", current_meta->linenum);
							goto error;
						}
						end_of_include = current_meta->right;
					}
					include_statement = father;
					free(father->text); father->text = NULL;
					include_text = current_meta;
					goto handle_include;
				}

				continue;
				handle_include:;
				{/*handle the include, then shove in tokenized where the beginning of the include statement was.*/
					file_to_include = NULL;
					memset(&tokenz,0,sizeof(strll));
					
					right_of_include_statement = end_of_include->right;
					end_of_include->right = NULL;
					
					if(include_level > MAX_INCLUDE_LEVEL)
						{
							printf("\r\n<SYNTAX ERROR> include level maximum exceeded in file %s. line %ld",filename, current_meta->linenum);
							goto error;
						}
					if(include_text->data ==(void*)22){
						char* temp;
						include_text->text[strlen(include_text->text)-1] = '\0';
						temp = strcata(sys_include_dir,include_text->text+1);
						file_to_include = realpath(temp, NULL);
						include_text->text[strlen(include_text->text)] = '>'; /*Must be longer*/
						free(temp);
					} else if(include_text->data ==(void*)2){
						include_text->text[strlen(include_text->text)-1] = '\0';
						//printf("Filename is %s\n", filename);
						temp_include_manip = filepath_to_directory((char*)filename);
						temp_include_manip = strcataf1(temp_include_manip,include_text->text + 1);
						file_to_include = realpath(temp_include_manip, NULL);
						include_text->text[strlen(include_text->text)] = '"';
						free(temp_include_manip);
					} else {
						printf("\r\n<ERROR> include destination is not string or <> string. line %ld", current_meta->linenum);
						goto error;
					}

					if(!file_to_include){
						printf("<ERROR> cannot find file %s, or malloc failed\r\n",include_text->text );
						goto error;
					}
					if(strlen(file_to_include) == 0){
						printf("\r\n<ERROR> include destination is zero length string. line %ld", current_meta->linenum);
						goto error;
					}
					
					/*TODO*/
//					printf("</DEBUG including %s>\r\n", file_to_include);
					f = fopen(file_to_include,"r");
					
					if(!f){
						printf("<ERROR> cannot open file %s\r\n",file_to_include);
						goto error;
					}
					{
						tokenz.text = read_file_into_alloced_buffer(f, &entire_input_file_len);
						if(!tokenz.text){
							printf("<ERROR> file had nothing in it? %s\r\n",file_to_include);
							goto error;
						}
						fclose(f);
					}
					if(nfilenames >= MAX_FILES){
						puts("Cannot include file, too many have been included.");
						exit(1);
					}
					char* from_files_to_include = 0;
					unsigned int i;
					for(i = 0; i < nfilenames; i++){
					    if(streq(filenames[i], file_to_include)){
					        from_files_to_include = filenames[i];
					        break;
					    }
					}
					if(from_files_to_include == 0){
					    filenames[nfilenames++]= strdup(file_to_include);
					    from_files_to_include = filenames[nfilenames-1];
					}
					tokenizer(&tokenz,  from_files_to_include, include_level+1);
					
					free(file_to_include);
					strll_append(&tokenz, right_of_include_statement);	
					strll_replace_node_with_list(include_statement, &tokenz, 0);
					current_meta = include_statement;
					goto top; /*Don't go right, we need to start handling this include as well.*/
				}
			}
	}


	return;
	error:;
	printf("\r\nParsing of %s aborted.\r\n",filename);
	exit(1);
}
strll* linear_strll_dupe(strll* i){
	strll* f;
	strll* retval = 0;
	strll* w;

	while(i){
		f = malloc(sizeof(strll));
		if(!f){
			puts("<ERROR> failed malloc");
			exit(1);
		}
		memcpy(f,i,sizeof(strll));
		if(i->text) f->text = strdup(i->text);
		
		f->right = NULL;
		f->left = NULL;
		f->child = NULL;
		
		if(retval){
			w = retval;
			while(w->right) w = w->right;
			w->right = f;
		} else{
			retval = f;
		}
		i = i->right;
	}
	return retval;
}

strll* strll_replace_using_replacement_list(
	strll* target_father, 
	strll* replacement_list
){
	/**/
	strll* insertion_procedure;
	strll* old_right;
	
	if(target_father->right == NULL){
		return 0; /*error!*/
	}
	insertion_procedure = linear_strll_dupe(replacement_list);
	old_right = target_father->right->right; /*immediately right of the node we want to replace.*/
	target_father->right->right = NULL; /*set the node we want to replace's right pointer to null*/
	strll_free(target_father->right,1); /*free the node we want to replace.*/
	/*append the old right pointer to */
	strll_append(insertion_procedure, old_right);
	target_father->right = insertion_procedure;
	return old_right;
}





void strll_handle_defines(strll* original_passin){
	/*Generate define replacement lists.*/
	strll* current_meta;
	strll* father = NULL;
	strll* replacement_list; /**/
	
	
	
	for(current_meta = original_passin;current_meta != NULL; 
		/*Increment. We use the comma operator to do multiple assignments in a single expression.*/
		(father = current_meta),
		(current_meta = current_meta->right)

	){
		
		if(current_meta->data ==TOK_DEFINE)
		{
			strll* walker;
			strll* walker_father;
			strll* stop_replacing_point;
			if(!father){
				puts("<ERROR> a #define is at beginning of the compilation unit. This is not compatible. Put a newline in!");
				goto error;
			}
			walker_father = current_meta;
			walker = current_meta->right;
			for(;walker != NULL; (walker_father = walker),(walker = walker->right)  ){
				if(walker->data == (void*)1)
					break;
			}
			
			if(walker == NULL)
			{
				puts("<ERROR> #define without newline ending the replacement list.");
				goto error;
			}
			/*There is at least one thing to the right of us... what is it?*/
			if(current_meta->right->data != (void*)8){
				puts("<ERROR> #define with non-identifier trailing.");
				goto error;
			}
			/*get the replacement list. We will use father to continue iteration.*/
			/*replacement list points to the #define*/
			/*Pattern of the replacement list is replacement_list->ident->(list->)newline*/
			/*We actually don't want to move the newline, and walker is the newline...*/
			replacement_list = current_meta;
			/*Walker currently points to the newline after the replacement list.*/
			/*We want to iterate into the */
			father->right = walker_father->right;
			walker_father->right = NULL;

			/*Get rid of that newline!*/
			//strll_free(walker,1);
			//walker_father->right = NULL;
			/*search for where this thing is undef'd*/
			stop_replacing_point = NULL;
			for(walker = father->right;walker != NULL; walker = walker->right){
				if(walker->data == TOK_UNDEF ||
				walker->data == TOK_DEFINE)
				if(walker->right){
					if(walker->right->data == TOK_IDENT){ /*found identifier*/
						if(streq(walker->right->text, replacement_list->right->text)){ /*matches replacement list's identifier*/
							stop_replacing_point = walker;break; /**/
						}
					} else {
						puts("<TOKENIZER ERROR> #undef or #define without identifier. It might be a reserved keyword or operator...");
						goto error;
					} /*identifier*/
				}
			}
			/*Walk the entire file and find the tokens that need to be replaced.*/
			for(walker = father->right;
				walker != stop_replacing_point; 
				walker = walker->right
			){
				replace_stuff_top:;
				if(walker->right){
					if(walker->right == stop_replacing_point) break; /*Do not proceed past the undef point.*/
					
					if(walker->right->data == (void*)8)
					if(streq(walker->right->text, replacement_list->right->text)){ /*do replacement list...*/
						walker = strll_replace_using_replacement_list(
							walker, 
							replacement_list->right->right
						);
						goto replace_stuff_top; /*we have effectively done one iteration.*/
					}
				}
			}



			/*After we are done...*/
			strll_free(current_meta,1); //TODO: find out what this does
			current_meta = father; /*we set father->right earlier so that iteration will work as normal if we do things this way.*/
			continue;
		} /*define handled*/
	}

	/*At this point, all defines are handled. Remove all undefs.*/
	for(current_meta = original_passin;current_meta != NULL; 
			/*Increment. We use the comma operator to do multiple assignments in a single expression.*/
			(father = current_meta),
			(current_meta = current_meta->right)
	
	){
		if(current_meta->data == (void*)25)
		{
			strll* old_right;
			if(!father){
				puts("<ERROR> an #undef is at beginning of the compilation unit.");
				goto error;
			}
			/*to our right is the identifier that is being undef'd....*/
			if(current_meta->right)
			{
				
				old_right = current_meta->right->right;
				
				current_meta->right->right=NULL;
				strll_free(current_meta->right,1);
				current_meta->right = old_right;
			}
			/*Now, remove the undef itself.*/
			old_right = current_meta->right;
			current_meta->right = NULL;
			strll_free(current_meta,1);
			father->right = old_right;
			current_meta = father; /*iteration will proceed as normal.*/
		}
	}
	return;
	error:;
	printf("\r\nDefine evaluation aborted.\r\n");
	exit(1);
}

static void strll_delete_newlines(strll* in){
	strll* current;
	strll* saved_delete;

	current = NULL;
	for(current = in; current != NULL;
		(current = current->right)
	){
		/**/
		top:;
		if(current == NULL) return;
		if(current->data == (void*)1){
			saved_delete = current->right;
			if(current->right){
				memcpy(current, current->right, sizeof(strll));
				saved_delete->right = NULL;
				saved_delete->text = NULL;
				free(saved_delete);
				goto top;
			}
			/*The file is just a newline. turn ourselves into a semicolon...*/
			current->data = (void*)16;
			return;
		}
		if(current->right)
		if(current->right->data == (void*)1){
			saved_delete = current->right->right;
			if(current->right->right){
				memcpy(current->right, current->right->right, sizeof(strll));
				saved_delete->right = NULL;
				saved_delete->text = NULL;
				free(saved_delete);
				goto top;
			} 
			{
				/*we must remove current->right, and this is the end, so, return.*/
				strll_free(current->right,1);
				current->right = NULL;
				return;
			}
			
		}
	}
}

static void strll_process_charliterals(strll* in){
	strll* current;
	unsigned long value = 0;
	unsigned long i;
	char buf[128];

	current = NULL;
	for(current = in; current != NULL;
			(current = current->right)
	){
		if(current->data == (void*)3){
			mstrcpy(current->text, current->text+1);
			current->text[strlen(current->text)-1] = '\0';

			for(i = 0; i < strlen(current->text);i++){
				if(current->text[i] == '\\'){
					i++;
					if(current->text[i] == 'a') {value = 0x7;continue;}
					if(current->text[i] == 'b') {value = 0x8;continue;}
					if(current->text[i] == 'e') {value = 0x1B;continue;}
					if(current->text[i] == 'f') {value = 0xC;continue;}
					if(current->text[i] == 'n') {value = 0xa;continue;}
					if(current->text[i] == 'r') {value = 0xD;continue;}
					if(current->text[i] == 't') {value = 0x9;continue;}
					if(current->text[i] == 'v') {value = 0xB;continue;}
					if(current->text[i] == '\\') {value = 0x5C;continue;}
					if(current->text[i] == '\'') {value = 0x27;continue;}
					if(current->text[i] == '\"') {value = 0x22;continue;}
					if(current->text[i] == '\?') {value = 0x3F;continue;}
					if(current->text[i] == '0') {value = 0;continue;}
					if(current->text[i] == 'x'){
						value = atou_hex(current->text+1);
						i++;
						while(mishex(current->text[i])) i++;
						if(current->text[i] == '\0') break;
						continue;
					}
				}
				value = ((unsigned char*)current->text)[i];
			}
			/*generate a string.*/
			mutoa(buf, value);
			free(current->text);
			current->text = strdup(buf);
			if(!current->text){
				puts("Failed Malloc");
				exit(1);
			}
			current->data = (void*)6; /*integer constant.*/
			
		}
	}
}


static void strll_process_stringliterals(strll* in){
	strll* current;
	unsigned long i;

	current = NULL;
	for(current = in; current != NULL;
			(current = current->right)
	){
		if(current->data == (void*)2){
			mstrcpy(current->text, current->text+1);
			current->text[strlen(current->text)-1] = '\0';
			for(i = 0; i < strlen(current->text);i++){
				if(current->text[i] == '\\'){
					if(current->text[i+1] == 'a'){
						current->text[i] = '\a';
						mstrcpy(current->text+i+1, 
								current->text+i+2
						);
						continue;
					}
					if(current->text[i+1] == 'b'){
						current->text[i] = '\b';
						mstrcpy(current->text+i+1, 
								current->text+i+2
						);
						continue;
					}
					if(current->text[i+1] == 'e'){
						current->text[i] = '\e';
						mstrcpy(current->text+i+1, 
								current->text+i+2
						);
						continue;
					}
					if(current->text[i+1] == 'f'){
						current->text[i] = '\f';
						mstrcpy(current->text+i+1, 
								current->text+i+2
						);
						continue;
					}
					if(current->text[i+1] == 'n'){
						current->text[i] = '\n';
						mstrcpy(current->text+i+1, 
								current->text+i+2
						);
						continue;
					}
					if(current->text[i+1] == 'r'){
						current->text[i] = '\r';
						mstrcpy(current->text+i+1, 
								current->text+i+2
						);
						continue;
					}
					if(current->text[i+1] == 't'){
						current->text[i] = '\t';
						mstrcpy(current->text+i+1, 
								current->text+i+2
						);
						continue;
					}
					if(current->text[i+1] == 'v'){
						current->text[i] = '\v';
						mstrcpy(current->text+i+1, 
								current->text+i+2
						);
						continue;
					}
					if(current->text[i+1] == '\0'){
						puts("<INTERNAL ERROR> Somehow escaping the end of string?");
						exit(1);
					}
					/*if it is not a recognized sequence... get rid of the backslash.*/
					mstrcpy(current->text+i, current->text+i+1);
					continue;
				}
			}
			if(!current->text){puts("Failed Malloc");exit(1);}
		}
	}
}



static void strll_concat_stringliterals(strll* in){
	strll* current;
	strll* father;
	strll* saved_current_right;

	current = NULL;
	father = NULL;
	for(current = in; 
			current != NULL;
			(father = current),
			(current = current->right)
			
	){
		top:;
		if(father)
		if(father->data == (void*)2)
		if(current->data == (void*)2){
			/*we have 2 string literals next to each other to concatenate!*/
			father->text = strcatafb(father->text, current->text);
			current->text = NULL;
			saved_current_right = current->right;
			current->right = NULL;
			current->left = NULL;
			current->child = NULL;
			strll_free(current,1);
			father->right = saved_current_right;
			current = saved_current_right;
			goto top;
		}
	}
}

static char hex_expansion[1024];
//this is never done. see below.
static void strll_hexify_int_constants(strll* in){
	for(;in != NULL;in = in->right)
		if(in->data == (void*)6){
			unsigned long long s = strtoull(in->text,0,0);
			mutoa_hex(hex_expansion, s);
			free(in->text);
			in->text = strdup(hex_expansion);
			if(!in->text){
				puts("Malloc failed!");
				exit(1);
			}
			
		}
}

//neither is this done
static void strll_decify_int_constants(strll* in){
	for(;in != NULL;in = in->right)
		if(in->data == (void*)6){
			unsigned long long s = strtoull(in->text,0,0);
			mutoa(hex_expansion, s);
			free(in->text);
			in->text = strdup(hex_expansion);
			if(!in->text){
				puts("Malloc failed!");
				exit(1);
			}
			
		}
}




strll tokenized = {0};


int saved_argc;
char** saved_argv;


static char* read_stdin_until_we_get_eof(FILE* f, unsigned long* len){
	char* buf = NULL;
	size_t lent = 0;
	size_t cap = 10000;
	buf = malloc(10000);
	while(1){
	    int c = fgetc(f);
	    if(feof(f)) break;
	    buf[lent++] = c;
	    if(lent == cap){
	        cap *= 2;
	        buf = realloc(buf, cap);
	    }
	}
	*len = lent;
	return buf;
}


void vm_allocate_needed_memory(size_t amt);
//void alloc_symbol_table();

int main(int argc, char** argv){
	saved_argc = argc;
	saved_argv = argv;
	strll* f;
	char* infilename = NULL;
	FILE* ifile = NULL;
	char* entire_input_file;
	unsigned long entire_input_file_len = 0;
	char* t;
	nfilenames = 0;
	if(argc > 1) {
	    infilename = strdup(argv[1]);
    }
    if(infilename)
        if(
            streq(infilename, "--manpage") ||
            streq(infilename, "-m") ||
            streq(infilename, "--manual") ||
            streq(infilename, "--man") ||
            streq(infilename, "-man")
        ){
            if(!(argc >2)){
                puts("You need to say what manpage you want!");
                exit(1);
            }
            print_manpage(argv[2]);
            exit(0);
        }
	if(
	    !infilename ||
	    streq(infilename, "-v") ||
	    streq(infilename, "-h") ||
	    streq(infilename, "--help") ||
	    streq(infilename, "--version")
	){
	    const char* bar = "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~";
	    puts("The CBAS Metaprogramming Tool");
	    puts(bar);
	    puts("  For the maximum utilization of Man's");
	    puts("            God-Given Talents");
	    puts(bar);
	    puts("Follow Peace with all men, and holiness,");
	    puts("without which no man shall see the Lord:");
	    puts("-Hebrews 12:14 KJV");
	    puts(bar);
	    puts("  Written in C99, under the CC0 License.");
	    puts(bar);
	    puts("~~~~~~~~~~~1 Corinthians 16:14~~~~~~~~~~~");
	    puts(bar);
	    puts("Let all your things be done with charity.");
	    puts(bar);
	    puts("Please ask the Lord Jesus Christ to bless");
	    puts("this project, and all fruit that it bears");
	    puts(bar);
	    puts(" May your adventures be as grand as your");
	    puts(" imagination can entail.");
	    puts(bar);
	    puts("May this software increase and multiply");
	    puts("your joy in the labor of your hands.");
	    puts(bar);
	    puts("                                         ");
	    puts("         ***               ***           ");
	    puts("       ** | **           **   **         ");
	    puts("      *   |   **       **       *        ");
	    puts("     * ---+---  **   **          *       ");
	    puts("     *    |       * *            *       ");
	    puts("     *    |        *             *       ");
	    puts("      *   |                     *        ");
	    puts("       *                       *         ");
	    puts("         *                   *           ");
	    puts("           *               *             ");
	    puts("             *           *               ");
	    puts("               *       *                 ");
	    puts("                 *   *                   ");
	    puts("                   *                     ");
	    puts(bar);
	    puts("Blessed be the Lord Jesus Christ forever.");
	    puts(bar);
	    puts("If you would like to learn how to use");
	    puts("seabass, try invoking...");
	    printf("%s -m help\n",argv[0]);
	    exit(0);
	}
	ifile = fopen(infilename, "rb");
	if(!ifile){
		puts("Cannot open the input file");
		exit(1);
	}
	entire_input_file = read_file_into_alloced_buffer(ifile, &entire_input_file_len);
	fclose(ifile); 
	ifile = NULL;
	tokenized.text = entire_input_file;
	vm_allocate_needed_memory(0x2000000);
	/*
	    Needed for secure symbol table...
	*/
	
	{
	    {
		    t = realpath(infilename, NULL);
		} 
		if(!t) {
			puts("\r\nCannot find file realpath!\n");
			exit(1);
		}
		filenames[nfilenames++] = strdup(t);
		free(t);
		t = NULL;
		if(!filenames[nfilenames-1]){
			puts("Strdup failed!");
			exit(1);
		}
		tokenizer(&tokenized, filenames[nfilenames-1], 0);
		{
			f = malloc(sizeof(strll));
			if(!f){
				puts("Malloc failed!");
				exit(1);
			}
			memcpy(f, &tokenized, sizeof(strll));
			tokenized.right = f;
			tokenized.text = NULL;
			tokenized.data = (void*)1; /*newline*/
			tokenized.linenum = 0; /*line does not exist*/
			tokenized.left = NULL; /*no left child*/
			tokenized.child = NULL; /*no true heirs to the throne*/
		}
		strll_handle_defines(&tokenized);
		/*strll_hexify_int_constants(&tokenized);*/
		strll_process_charliterals(&tokenized);
		strll_process_stringliterals(&tokenized);
		//strll_concat_stringliterals(&tokenized); //TODO: Maybe remove this?
		strll_delete_newlines(&tokenized);
		//strll_remove_owning_operator_pointers(&tokenized);
		{
			strll* m = malloc(sizeof(strll));
			memcpy(m, &tokenized, sizeof(strll));
			compile_unit(m);
		}
		//free(t);
	}

	

	/*Call your Compiler here!*/
	strll_show(&tokenized, 0);
	strll_free(&tokenized, 0);
	free(infilename);
	return 0;
}
/*End of file comment.*/
