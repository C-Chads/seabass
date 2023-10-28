
#include <stdio.h>
#include "data.h"

typedecl* type_table = NULL;
typedecl_oop_metadata* oop_metadata = NULL;
symdecl* symbol_table = NULL;
uint64_t* parsehook_table = NULL;
scope** scopestack = NULL;
stmt** loopstack;
uint64_t active_function = 0;
uint64_t ntypedecls = 0;
uint64_t nsymbols = 0;
uint64_t nscopes = 0;
uint64_t nloops = 0;
uint64_t nparsehooks = 0;
static uint64_t noop_metadata = 0; //used internally only...


/*
    TODO: author stuff to create metadata...
*/

void* re_allocX(void* p, uint64_t sz); //a function declared in parser.c that I think will be useful here.


uint64_t push_empty_metadata(){
    oop_metadata = re_allocX(oop_metadata, (noop_metadata + 1) * sizeof(typedecl_oop_metadata));
    memset(oop_metadata+noop_metadata, 0, sizeof(typedecl_oop_metadata)); //zero-initialize, for safety!
    noop_metadata++;
    return noop_metadata-1;
}

//This function should only be called when the ctor or dtor are needed. It creates the
//relationship.
void update_metadata(uint64_t declid){
    if(oop_metadata[declid].have_checked) return;
    
    
    oop_metadata[declid].have_checked = 1; // WE ARE OFFICIALLY CHECKING!
    
    //initialize these to negative one...
    oop_metadata[declid].ctor_id = -1;
    oop_metadata[declid].dtor_id = -1;
    //get the name
    char* name = type_table[declid].name;
    //mutate for the ctor...
    char* ctor_name = strcata("__method_", name);
    ctor_name = strcataf1(ctor_name, "_____");
    ctor_name = strcataf1(ctor_name,"ctor");
    //and for the dtor...
    char* dtor_name = strcata("__method_", name);
    dtor_name = strcataf1(dtor_name, "_____");
    dtor_name = strcataf1(dtor_name,"dtor");
    
    //search the list of symdecls for the existence of a ctor and dtor matching the prototype...
    int64_t i;
    int found_ctor = 0;
    int found_dtor = 0;
    for(i = 0; i < (int64_t)nsymbols; i++){
        if(symbol_table[i].t.is_function == 0) continue;
        int is_ctor = streq(symbol_table[i].name, ctor_name);
        int is_dtor = streq(symbol_table[i].name, dtor_name);
        if(!is_ctor && !is_dtor) continue;
        //it has the correct name, make sure it has the right number of arguments!
        if(
            symbol_table[i].nargs != 1              || //Recall that a method takes itself as the first argument...
            symbol_table[i].t.basetype != BASE_VOID ||
            symbol_table[i].t.pointerlevel != 0     ||
            symbol_table[i].t.arraylen != 0
        ){
            puts("Object Oriented Programming Error!!!!");
            if(is_ctor)
                puts("You attempted to define the ctor");
            else
                puts("You attempted to define the dtor");
            puts("of the type");
            puts(type_table[declid].name);
            puts("but with an improper prototype.");
            puts("Ctor and dtor always take zero arguments, and return nothing.");
            if(symbol_table[i].nargs == 0){
                puts("I can tell you have been using undefined behavior... Naughty!");
                puts("User is invoking undefined behavior!");
                exit(1);
            }
            puts("ctor and dtor methods _must_ always be declared with no arguments, returning nothing!");
            exit(1);
        }
        
        if(is_ctor){
            oop_metadata[declid].ctor_id = i;
            found_ctor = 1;
            if(found_ctor && found_dtor) break;
        }else if(is_dtor){
            oop_metadata[declid].dtor_id = i;
            found_dtor = 1;
            if(found_ctor && found_dtor) break;
        }
    }
    free(ctor_name);
    free(dtor_name);
}




int scope_has_label(scope* s, char* name){
	uint64_t i;
	stmt* stmt_list = s->stmts;
	
	for(i = 0; i < s->nstmts; i++){
		if(stmt_list[i].kind == STMT_LABEL){
			if(streq(stmt_list[i].referenced_label_name,name))
				return 1;
		}
		if(stmt_list [i].myscope)
			if(scope_has_label(stmt_list [i].myscope, name))
				return 1;		
		/*if(stmt_list [i].myscope2)
			if(scope_has_label(stmt_list [i].myscope2, name))
				return 1;*/
	}
	return 0;
}


int unit_has_incomplete_symbols(){
	uint64_t i;
	for(i = 0; i < nsymbols; i++)
		if(symbol_table[i].is_incomplete) return 1;
	
	return 0;
}


void unit_throw_if_had_incomplete(){
	uint64_t i;
	for(i = 0; i < nsymbols; i++)
		if(symbol_table[i].is_incomplete)
		if(symbol_table[i].is_codegen){
			puts("The global symbol:");
			puts(symbol_table[i].name);
			puts("is codegen, and INCOMPLETE.");
			puts("Please provide a compatible definition for it.");
			parse_error("Error: Failure to define all symbols for compilation");
		}
	return;
}

static int is_parsing_codegen_code_code = 0;
void set_is_codegen(int a){
    is_parsing_codegen_code_code = a;
}
int get_is_codegen(){
    return is_parsing_codegen_code_code;
}
uint64_t get_target_word();
uint64_t get_target_max_float_type();
/*determine base type.*/
uint64_t peek_basetype(){
	uint64_t keyw_id = ID_KEYW(peek());
	if(keyw_id){
		if(keyw_id == 2) return BASE_U8;
		if(keyw_id == 3) return BASE_I8;
		if(keyw_id == 4) return BASE_U16;
		if(keyw_id == 5) return BASE_I16;
		if(keyw_id == 6) return BASE_U32;
		if(keyw_id == 7) return BASE_I32;
		if(keyw_id == 8) return BASE_U64;
		if(keyw_id == 9) return BASE_I64;
		if(keyw_id == 10) return BASE_F32;
		if(keyw_id == 11) return BASE_F64;
		if(keyw_id == 80){
		    //we need to do special logic to determine if we are in codegen code...
		    /*
		    if(get_is_codegen()){
		        return BASE_U64;
		    }else{
		        //check what the system's default is...
		        return get_target_word();
		    }*/
		    return BASE_U64; //Standard update: uptr is _always_ U64...
		}
		parse_error("Unknown peek_basetype keyword!");
		return 0;
	}
	if(ntypedecls > 0)
	{
		uint64_t i;
		for(i = 0; i < ntypedecls; i++)
			if(streq(type_table[i].name, peek()->text)){
				return BASE_STRUCT;
			}
	}
	parse_error("Unknown peek_basetype identifier.");
	return 0;
	/*unreachable...*/
	/*Search the functions. why not?*/
	if(nsymbols > 0){
		uint64_t i;
		for(i = 0; i < nsymbols; i++)
			if(symbol_table[i].t.is_function)
				if(streq(symbol_table[i].name, peek()->text)){
					return BASE_FUNCTION;
				}
	}
}

uint64_t consume_basetype(char* msg){
	uint64_t retval;
	require(peek_is_typename(),msg);
	retval = peek_basetype();
	consume();
	return retval;
}



int string_is_typename(char* text){
    if(streq(text, "u8")) return 1;
    if(streq(text, "i8")) return 1;
    if(streq(text, "u16")) return 1;
    if(streq(text, "i16")) return 1;
    if(streq(text, "u32")) return 1;
    if(streq(text, "i32")) return 1;
    if(streq(text, "u64")) return 1;
    if(streq(text, "i64")) return 1;
    if(streq(text, "f32")) return 1;
    if(streq(text, "f64")) return 1;
    if(streq(text, "float")) return 1;
    if(streq(text, "double")) return 1;
    if(streq(text, "int")) return 1;
    if(streq(text, "sint")) return 1;
    if(streq(text, "uint")) return 1;
    if(streq(text, "byte")) return 1;
    if(streq(text, "ubyte")) return 1;
    if(streq(text, "sbyte")) return 1;
    if(streq(text, "char")) return 1;
    if(streq(text, "uchar")) return 1;
    if(streq(text, "schar")) return 1;
    if(streq(text, "short")) return 1;
    if(streq(text, "ushort")) return 1;
    if(streq(text, "sshort")) return 1;
    if(streq(text, "long")) return 1;
    if(streq(text, "ulong")) return 1;
    if(streq(text, "slong")) return 1;
    if(streq(text, "llong")) return 1;
    if(streq(text, "ullong")) return 1;
    if(streq(text, "sllong")) return 1;
    if(streq(text, "qword")) return 1;
    if(streq(text, "uqword")) return 1;
    if(streq(text, "uptr")) return 1;
    if(streq(text, "sqword")) return 1;
	if(ntypedecls > 0)
	{
		uint64_t i;
		for(i = 0; i < ntypedecls; i++)
			if(streq(type_table[i].name, text)){
				return 1;
			}
	}
	return 0;
}

int peek_is_typename(){
	if(peek() == NULL) return 0;
	if(
	    peek()->data != TOK_KEYWORD &&
	    peek()->data != TOK_IDENT
	) return 0;
	return string_is_typename(peek()->text);
	//unreachable...
	if(peek()->data == TOK_KEYWORD){
		if(streq(peek()->text, "u8")) return 1;
		if(streq(peek()->text, "i8")) return 1;
		if(streq(peek()->text, "u16")) return 1;
		if(streq(peek()->text, "i16")) return 1;
		if(streq(peek()->text, "u32")) return 1;
		if(streq(peek()->text, "i32")) return 1;
		if(streq(peek()->text, "u64")) return 1;
		if(streq(peek()->text, "i64")) return 1;
		if(streq(peek()->text, "f32")) return 1;
		if(streq(peek()->text, "f64")) return 1;
		if(streq(peek()->text, "float")) return 1;
		if(streq(peek()->text, "double")) return 1;
		if(streq(peek()->text, "int")) return 1;
		if(streq(peek()->text, "sint")) return 1;
		if(streq(peek()->text, "uint")) return 1;
		if(streq(peek()->text, "byte")) return 1;
		if(streq(peek()->text, "ubyte")) return 1;
		if(streq(peek()->text, "sbyte")) return 1;
		if(streq(peek()->text, "char")) return 1;
		if(streq(peek()->text, "uchar")) return 1;
		if(streq(peek()->text, "schar")) return 1;
		if(streq(peek()->text, "short")) return 1;
		if(streq(peek()->text, "ushort")) return 1;
		if(streq(peek()->text, "sshort")) return 1;
		if(streq(peek()->text, "long")) return 1;
		if(streq(peek()->text, "ulong")) return 1;
		if(streq(peek()->text, "slong")) return 1;
		if(streq(peek()->text, "llong")) return 1;
		if(streq(peek()->text, "ullong")) return 1;
		if(streq(peek()->text, "sllong")) return 1;
		if(streq(peek()->text, "qword")) return 1;
		if(streq(peek()->text, "uqword")) return 1;
		if(streq(peek()->text, "uptr")) return 1;
		if(streq(peek()->text, "sqword")) return 1;
	}
	if(peek()->data == TOK_IDENT)
	if(ntypedecls > 0)
	{
		uint64_t i;
		for(i = 0; i < ntypedecls; i++)
			if(streq(type_table[i].name, peek()->text)){
				return 1;
			}
	}
	return 0;
}

/*MANPAGES*/

extern const char* reflection_library_contents;
extern const char* license_text;
extern const char* oop_syntax_example;
extern const char* builder_library_contents;
extern const char* builder_example_contents;

void print_manpage(char* subject){
    #define p puts
    #define pp(x) {unsigned long qq; for(qq = 0; qq < sizeof(x) / sizeof(char*); qq++) puts(x[qq]);exit(0);}
    #define r(x) {if(streq(subject, #x)) pp(x);}
    #define l(x) x ,
    #define ll(x) "    " x ,
    #define lll(x) "        " x ,
    #define nl l("")
    #define o(x) l( "    " #x)
    #define b(x) l(bar)l("    Manpage: " #x)l(bar)
    #define m(n) const char* n[] = 
    const char* bar = "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~";
    const char* ibar = "    ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~";
    const char* fib_example = "\n\nfn codegen fib(int n)->int:\n\tif(n < 2)\n\t\treturn 1;\n\tend\n\tint a\n\tint b\n\tint c\n\ta = 1;\n\tb = 1;\n\tc = 1;\n\tn = n-2;\n\twhile(n)\n\t\tc = a + b;\n\t\ta = b;\n\t\tb = c;\n\t\tn--;\n\tend\n\treturn c;\nend\n\nfunction codegen codegen_main():\n\tchar[500] buf\n\tint mynum\n\tif(__builtin_getargc() < 3)\n\t    __builtin_puts(\"USAGE: cbas fib.cbas 3\");\n\t    __builtin_puts(\"replace 3 with whatever fib number you want.\");\n\t    __builtin_exit(1);\n\tend\n\tmynum = __builtin_atoi(__builtin_getargv()[2]);\n\t__builtin_itoa(buf, fib(mynum));\n\t__builtin_puts(\"The fibonacci number is:\");\n\t__builtin_puts(buf);\n\treturn;\t\nend";
    
    
    
    m(license){
        b(license)
        l(license_text)
        l(bar)
    };
    m(help){
        b(help)
        l("Welcome to the CBAS Metaprogramming Tool's Manual Pages!")
        nl
        l("This is the official documentation for the SEABASS metacompiler, as written in C99")
        l("This program was authored for the Public Domain, using the blessings of our Lord Jesus Christ.")
        nl
        l("To begin viewing documentation, view the index:")
        nl
        ll("cbas -m index")
        nl
        l("This will show you all available help topics.")
        l(bar)
    };
    m(index){
        b(index)
        l("usage:\n    cbas -m topicname")
        l(bar)
        o(help)
        o(license)
        o(index)
        o(terminology)
        o(syntax)
        o(for_c_programmers)
        o(error_messages)
        o(optimization)
        o(function_pointers)
        o(types)
        o(constexpr)
        o(switch_syntax)
        o(goto_labels)
        o(codegen)
        o(metaprogramming)
        o(qualifier_order)
        o(short_circuiting)
        o(reflection)
        o(parsehook)
        o(keywords)
        o(undefined_behavior)
        o(dedicatory)
        l(bar)
    };
    m(terminology){
        b(terminology)
        l(bar)
        l("These are some technical terms (jargon) which you need to be familiar with")
        l("to fully comprehend these manpages and work with SEABASS.")
        l(bar)
        l("programming language")
        ll("Any useful means of communication with a computer capable of teaching")
        ll("the computer to perform new tasks. Programming languages are used by")
        ll("computer programmers to write software. Programming languages are")
        ll("needed to make writing complex computer programs comprehensible to")
        ll("man. Most computers do not fundamentally understand any language")
        ll("that is even remotely readable by humans. Instead, they only know")
        ll("machine code. Without programming languages, it would be impossible")
        ll("for computer programmers to write complex programs.")
        nl
        l("machine code")
        ll("The unreadable gobblety-gook of seemingly random numbers which a")
        ll("computer executes 'natively'. 'The natural tongue of silicon'.")
        ll("Machine code is not only very difficult for anyone to understand,")
        ll("but it is usually very limited in portability and may even be specific")
        ll("to a particular piece of hardware.")
        nl
        l("compiler")
        ll("A translator. Reads in text in one language and spits out another.")
        ll("In the context of computer programming, this is the process of turning")
        ll("one programming language into another, or even into machine code")
        nl
        l("macro")
        ll("A pattern for defining new languages within an existing language by creating")
        ll("shorthands. Useful only for simple syntaxes. Does not use an abstract syntax tree.")
        nl
        ll("NOTE: Many people use the word macro to refer to any metaprogramming system.")
        ll("      However, this fails to differentiate between compilers and macros, and")
        ll("      in fact, confuses them. So I have created this definition.")
        nl
        l("parser + parsing")
        ll("The parser is the part of a compiler which discerns the structure")
        ll("of a computer program given its source code. The verb-form of this")
        ll("process is called 'parsing'.")
        nl
        l("metaprogramming")
        ll("Writing code that writes code, such as compilers.")
        ll("Macros are a very primitive form of metaprogramming.")
        nl
        l("token / lexical element")
        ll("The atoms of parsing a computer programming language. Tiny bits of text such")
        ll("as variable names, plus signs, and slashes. Tokens are generated from source")
        ll("code by a `tokenizer`, a program which takes program source code as input")
        ll("and emits tokens as output. The process of generating tokens is called")
        ll("'tokenization' and the verb form is 'tokenize'. It is also called 'lexing'.")
        nl
        l("preprocessing")
        ll("the part of a compiler which performs simple manipulations of tokens before")
        ll("they are ultimately passed to the parser. This typically includes expansion")
        ll("of macros, for instance.")
        nl
        l("compilation unit")
        ll("Abstract concept; the set of all `things` which exist within the code being")
        ll("processed by the compiler during a particular execution. The root file and all includes.")
        ll("It can refer to the raw contents of the source code files, the individual lexical elements")
        ll("of the files, or it can refer to the symbols defined within the source code.")
        nl
        l("code generation time / compile time")
        ll("When the compiler is running, and not the final output program.")
        nl
        l("target")
        ll("the platform for which the `final program` is to be written, as opposed to the")
        ll("development platform, which is where metaprograms (compilers) run.")
        nl
        l("symbol")
        ll("A function, method, or global variable (including data) for the target. It may also,")
        ll("rarely, refer to a type declaration, although if you find that in these pages, it is")
        ll("a mistake on my part.")
        nl
        l("parsehook")
        ll("see the `parsehook` manpage.")
        nl
        l("ast")
        ll("Short for `Abstract syntax tree`. The branching structure internal representation of")
        ll("a computer program, that exists within the compiler, at compile time. Generally these")
        ll("are the output of a parser.")
        nl
        l("reflection")
        ll("Abstract concept; the ability of a computer program to use information about its own")
        ll("structure, ranging from types to line numbers.")
        nl
        l("metacompiler")
        ll("A compiler designed for metaprogramming. Capable of arbitrary code execution at")
        ll("compiletime as well as emitting arbitrary output. A metacompiler must meet these")
        ll("criterion to be a true metacompiler:")
        ll("    1. It must be possible to write totally new syntaxes")
        ll("    2. It must be possible to generate arbitrary output")
        ll("    3. It must be possible to write compiletime compilers in the language.")
        ll("    4. It must be possible to combine, mix, or merge compiletime compilers.")
        ll("If a compiler does not meet these criteria, it is not a true metacompiler.")
        nl
        l("seabass")
        ll("A metaprogramming-oriented programming language with lua-like syntax, understood")
        ll("by this metacompiler.")
        nl
        l("cbas")
        ll("The SEABASS metaprogramming tool, metacompiler. Able to lex, pre-process, parse,")
        ll("and execute SEABASS source code at compiletime.")
        l(bar)
    };
    m(syntax){
        b(syntax)
        l("Seabass's base-level syntax is a mixture of lua and C. It is designed to be friendly and pragmatic.")
        l("However, Seabass is no ordinary programming language. It allows you to write entirely new")
        l("syntaxes within the language, even entire new programming languages.")
        nl
        l("In order to do that however, you need to know the base level language's syntax.")       
        l("To give you a rough gist of the syntax, here is a runnable minimal complete example program")
        l(bar)
        l(fib_example)
        l(bar)
        l("[to isolate this example for piping to file, run cbas -m syntax_fib_example]")
        nl
        nl
        l("If the program's source code is stored in `fib.txt` you can then run it like this:")
        nl
        ll("cbas fib.txt 20")
        nl
        l("Replace 20 with the number of the fibonacci number you want to view.")
        nl
        l("Do you like object oriented programming? Try this on for size...")
        l(bar)
        l(oop_syntax_example)
        l(bar)
        l("[to isolate this example for piping to file, run cbas -m syntax_oop_example]")
        nl
        l(bar)
    };
    m(dedicatory){
        b(dedicatory)
        l("All Glory, might, praise, power, honor, and glory to Jesus of Nazareth, King of Kings and Lord of Lords.")
        l("Without the blessings of God our Father and our Lord Jesus Christ, none of this would be possible.")
        l("Never forget him!")
        l(bar)
    };
    m(reflection){
    	b(reflection)
    	l("Seabass allows you to access the internals of the compiler. However, in order to do this,")
    	l("A support library is needed. Luckily, it is embedded within this very binary.")
    	l("You can ask for it by doing...")
    	nl
    	ll("cbas -m reflection_library_code")
    	nl
    	l("This will provide you with the full compiler reflection library, allowing you")
    	l("To access the AST during parsing and write your own code generators.")
    	nl
    	l("It even includes a built-in reflection test in the function `cgast_struct_size_test()`")
    	l("If the maintainer has failed to update the reflection library after a code update,")
    	l("then that function should serve to help detect that.")
    	l(bar)
    };
    m(parsehook){
        b(parsehook)
        l("Seabass enables metaprogramming by allowing your code to 'hook' into the parser.")
        nl
        l("You can write a function and then use it to do parsing.")
        nl
        l("It takes a fair chunk of code to set this up, but luckily for you, a library is")
        l("embedded in this binary which is designed to make writing parsehooks easy, called")
        l("'cgbuilder'")
        nl
        l("You can retrieve this library for piping to a file with the following command:")
        nl
	    ll("cbas -m builder_library_code")    
        nl
        l("note that it also requires the reflection library, which can be retrieved with:")
        nl
	    ll("cbas -m reflection_library_code")
        nl
        l("These should be called cgbuilder.hbas and cgast.hbas respectively.")
        nl
        l("Here is a minimal complete example program which demonstrates using the 'builder' to")
        l("create and use parsehooks:")
        l(bar)
        l(builder_example_contents)
        l(bar)
        l("[to isolate this example for piping to file, run cbas -m builder_example_code]")
        nl
        nl
        l("Hopefully, that example gives you some ideas.")
        l("Enjoy Programming!")
        l(bar)
    };
    m(keywords){
        b(keywords)
        l("Seabass has a very large number of keywords, at least in part")
        l("Because it has so many type aliases.")
        nl
        l("Here is a list of all keywords:")
        l(bar)
            ll( "fn") /*Extension*/
            ll( "function") /*Extension*/
            ll( "func") /*Extension*/
            ll( "procedure") /*Extension*/
            ll( "proc") /*Extension*/
            ll( "cast") /*Extension*/
            ll( "u8") /*Extension*/
            ll( "i8") /*Extension*/
            ll( "u16") /*Extension*/
            ll( "i16") /*Extension*/
            ll( "u32") /*Extension*/
            ll( "i32") /*Extension*/
            ll( "u64") /*Extension*/
            ll( "i64") /*Extension*/
            ll( "f32") /*Extension*/
            ll( "f64") /*Extension*/
            ll( "char") /*Extension*/
            ll( "uchar") /*Extension*/
            ll( "schar") /*Extension*/
            ll( "byte") /*Extension*/
            ll( "ubyte") /*Extension*/
            ll( "sbyte") /*Extension*/
            ll( "uint") /*Extension*/
            ll( "int") /*Extension*/
            ll( "sint") /*Extension*/
            ll( "long") /*Extension*/
            ll( "slong") /*Extension*/
            ll( "ulong") /*Extension*/
            ll( "llong") /*Extension*/
            ll( "ullong") /*Extension*/
            ll( "qword") /*Extension*/
            ll( "uqword") /*Extension*/
            ll( "uptr") /*Extension*/
            ll( "sqword") /*Extension*/
            ll( "noexport") /*Extension*/
            ll( "float") /*Extension*/
            ll( "double") /*Extension*/
            ll( "short") /*Extension*/
            ll( "ushort") /*Extension*/
            ll( "sshort") /*Extension*/
            ll( "break")
            ll( "data")
            ll( "string") /*used for the data statement.*/
            ll( "end") /*We're doing lua-style syntax!*/
            ll( "continue")
            ll( "if")
            ll( "elif")
            ll( "elseif")
            ll( "else")
            ll( "while") 
            ll( "for") 
            ll( "goto")
            ll( "jump")
            ll( "switch") /*very different from C, optimized jump table*/
            ll( "return") /*same as C*/
            ll( "tail") /*tail call*/
            ll( "sizeof") /*get size of expression*/
            ll( "static") /*Static storage. Not the C usage.*/
            ll( "pub") /*used to declare as exporting.*/
            ll( "public") /*used to declare as exporting.*/
            ll( "predecl") /*used to predeclare*/
            ll( "struct")
            ll( "class") 
            ll( "union")
            ll( "method")
            ll( "codegen")
            ll( "constexpri") 
            ll( "constexprf") 
            ll( "pure") /*enforce purity.*/
            ll( "inline") /*inline*/
            ll( "atomic") /*qualifier*/
            ll( "volatile") /*qualifier*/
            ll( "getfnptr") /*gets a function pointer.*/
            ll( "callfnptr") /*calls a function pointer.*/
            ll( "getglobalptr") /*gets pointer to global*/
            ll( "asm") /*extension*/
            l(bar)
            l("The following are also reserved, but are operators:")
            l(bar)
            ll( "streq") /*extension*/
            ll( "strneq") /*extension*/
            ll( "eq") /*extension*/
            ll( "neq") /*extension*/
        l(bar)
    };
    m(types){
        b(types)
        l("Seabass has primitive types (integers and floats) pointers, arrays, and structs.")
        nl
        l("The primitive types are the integer types u8,i8,u16,i16,u32,i32,u64,i64")
        l("and the floating-point types f32 and f64.")
        nl
        l("Types are typically associated with variables or struct members. Type declarations look like this:")
        nl
        ll("myTypename")
        ll("myTypename2*")
        ll("myOtherTypeName***")
        ll("myGrandmaWroteThisClass[50]")
        ll("i32[143 + (72 | 1) * 2 + my_codegen_global_variable]")
        ll("f64****[143 + (72 | 1) * 2]")
        nl
        l("Let's use these types to declare some variables, shall we?")
        nl
        ll("myTypename a")
        ll("myTypename2* WhatAJollyNameForAVariable")
        ll("myOtherTypeName*** AFantasticVariableName")
        ll("myGrandmaWroteThisClass[50] bad_variable_name_unexplanatory_37")
        ll("i32[143 + (72 | 1) * 2 + my_codegen_global_variable] my_friend_wrote_this")
        ll("f64****[143 + (72 | 1) * 2] crazy_floatptr_array")
        nl
        l("Notice the following:")
        ll("1. All type declarations begin with a typename.")
        ll("2. Arrays are not specified next to a variable's name, they are written next to the type.")
        ll("3. you can't have a pointer to an array, only an array of pointers, or pointer to pointer.")
        ll("4. Variable names follow the C identifier rules, underscore or alphabetical for the first character,")
        ll("   and alphanumeric or underscore for the rest.")
        ll("5. You can put complex expressions inside of an array length specifier. You can even use")
        ll("   codegen integer or float variables.")
        l("Inside the compiler, variable and expression types are represented by the 'type' struct")
        l("You can see more clearly how types are represented in the language")
        l("by viewing the reflection library.")
        nl
        ll("cbas -m reflection_library_code")
        nl
        l(bar)
        ll("POINTER DECAY")
        l(bar)
        l("Arrays and structs both decay into rvalue pointers in expressions.")
        l(bar)
    };
    m(constexpr){
        b(constexpr)
        l("Seabass supports constexpr expressions. A constexpr expression can be embedded within any")
        l("expression by using constexpri or constexprf (for float or integer, respectively)")
        nl
        l("You would want to use constant expressions for the following reasons:")
        ll("1. You are declaring the size of an array, or the initializer of a global variable,")
        lll("which are always constexpr. Note that you don't explicitly have to use constexpri")
        lll("when declaring an array- it is automatically assumed.")
        ll("2. You have a complicated expression to compute which you only want evaluated once during parsing.")
        ll("3. You are writing a `data` statement storing values computed at compiletime.")
        l(bar)
    };
    m(codegen){
        b(codegen)
        l("Seabass is a metaprogramming language- it lets you write code that writes code. ")
        l("This is achieved by allowing the compilation unit to define functions which run")
        l("during parsing and compiletime, which have almost unlimited access to the compiler's internals")
        l(bar)
        nl
        l("While the unit is being parsed, Seabass will check for the metaprogramming operator (@) and if")
        l("it finds it, it will attempt to call a codegen function. These are called 'parsehooks'.")
        l("(See the parsehook manpage for more information on those)")
        nl
        l("After the entire unit has been parsed, the compiler will look for a function called")
        l("'codegen_main'. If it is able to find that function it will call it. The job of codegen_main")
        l("is to compile all non-codegen functions, variables, data statements, types, etcetera into")
        l("target code.")
        nl
        l("You can define global variables, functions, classes, and methods which are codegen.")
        l("In order to declare a symbol `codegen` simply attach the qualifier to it.")
        l("You can check out some example code which uses `codegen` by looking at the `syntax` manpage.")
        nl
        l("codegen functions generally serve two different purposes: To aid in parsing, or to aid in")
        l("final compilation.")
        l(bar)
    };
    m(qualifier_order){
        b(qualifier_order)
        l("When defining global symbols, it is possible to attach qualifiers to them, such as `codegen` or `volatile`.")
        l("Some qualifiers contradict, such as `pub` and `static` or `pub` and `inline`.")
        l("The order of qualifier specification is rigidly defined. They must be in-order. Here are the orders:")
        nl
        ll("fn predecl codegen noexport [pub/static] inline pure myFunction()->int;")
        ll("method predecl codegen noexport [pub/static] inline pure myClass.myMethod()->float;")
        ll("predecl codegen noexport [pub/static] [atomic/volatile] i64 myVariable =  73 * 2;")
        ll("data predecl codegen noexport [pub/static] i64 myData 1, 2, 3+5%2;")
        nl
        l("in general, predecl goes before codegen, which goes before noexport, which goes before visibility")
        l("qualifiers, which go before inline and pure.")
        nl
        l("Here is an explanation of what every qualifier means:")
        nl
        ll("predecl: Symbol is being predeclared, no definition will be provided now. It may be defined later.")
        ll("codegen: Symbol is used only at compiletime.")
        ll("noexport: Symbol should not be exported by codegen_main, regardless of it being codegen or not.")
        ll("pub: Symbol has public visibility with other compilation units. Symbols defined in this unit are externally visible.")
        ll("static: Symbol has private visibility with other compilation units. This is the default.")
        ll("atomic: Symbol can be read/written atomically in a multithreaded environment.")
        ll("volatile: redundant reads and writes to this variable will not be optimized out. It could be")
        ll("          a hardware register, or a 'ghetto atomic boolean'")
        l(bar)
    };
    m(undefined_behavior){
        b(undefined_behavior)
        l("Seabass, like C and C++, has some operations which are undefined behavior.")
        l("For target code, this is implementation specific, but for the codegen environment, this")
        l("is more well-defined.")
        nl
        l("The following things are all obviously undefined behavior:")
        ll("dereferencing a null pointer")
        ll("reading/writing an invalid pointer, or past the bounds of an array")
        ll("Type-punning a pointer and performing weird operations on the data")
        ll("Division or modulo by zero")
        ll("Implementation-trapped Floating point errors")
        ll("yada-yada...")
        nl
        l("The following are more insidious...")
        ll("Putting the compilation unit in an invalid state")
        ll("modifying the currently-executing function in the AST")
        ll("using five underscores in-a-row (_____) in the name of a variable")
        nl
        l("You should also be careful to avoid...")
        ll("Namespace collisions with hidden variables starting with two underscores")
        nl
        l("There are other things that are undefined behavior, of course, but")
        l(bar)
    };
    m(short_circuiting){
        b(short_circuiting)
        l("Seabass does short circuiting. If you have an && or || and the first argument is 0 (for &&)")
        l("or not zero(for ||) then the rest of the and/or chain is not evaluated.")
        l(bar)
    };
    m(syntax_fib_example){
        l(fib_example)
    };
    m(syntax_oop_example){
        l(oop_syntax_example)
    };
    m(reflection_library_code){
        l(reflection_library_contents)
    };
    m(builder_library_code){
        l(builder_library_contents)
    };
    m(builder_example_code){
        l(builder_example_contents)
    };
    m(function_pointers){
        b(function_pointers)
        l("Seabass, like C, has function pointers.")
        l("However, unlike C, they are not part of the type system.")
        l("All function pointers are `byte*`")
        l("To get a functions function pointer, do:")
        nl
        ll("getfnptr(name_of_your_function);")
        nl
        l("to use the function pointer, do...")
        ll("callfnptr[function_with_prototype_you_want_to_use](expression_resolving_to_byte_ptr())(arg1, arg2, arg3,);")
        nl
        l("Notice of course that you need to have a function whose prototype is identical to the function")
        l("you're calling in order to call it. You can make it a predeclared function with an empty body if you like.")
        nl
        l("You may not get a pointer to a method.")
        l(bar)
    };
    m(for_c_programmers){
        b(for_c_programmers)
        l("Welcome to the introduction to Seabass for C programmers!")
        nl
        l("This manpage will give you the basic 'gist' of what Seabass is, as well as")
        l("discuss its performance capabilities compared to raw C.")
        nl
        l("WHAT IS SEABASS?")
        ll("A metaprogramming language. However, this is an abstract and vague definition,")
        ll("So let me give you some rough analogies:")
        l("TWO LANGUAGES IN ONE")
        ll("Code in seabass can be classified two different ways: Target or Codegen.")
        ll("Target code is meant to be compiled into some lower level language (such as")
        ll("C or assembly) while codegen code is executed during compiletime. These")
        ll("languages are pretty much totally identical in syntax.")
        l("AN INTERPRETED LANGUAGE")
        ll("Seabass is, principally, an interpreted language. When the `cbas` tool is invoked")
        ll("on source code, it is immediately 'executed'.")
        ll("This execution, as we'll discuss later, is rather unusual compared to most other")
        ll("interpreted languages (Code written for the target is not executed, for instance) but those")
        ll("details will hopefully become clear later.")
        l("A STRICTLY COMPILED LANGUAGE")
        ll("You can also think of Seabass as a compiled language. Code which is not")
        ll("'codegen' is not executable until after it has been compiled into something")
        ll("which is. The responsibility of performing this code generation step rests")
        ll("with the interpreted language.")
        l("A RETARGETABLE COMPILER")
        ll("You can also think of Seabass as a retargetable compiler. You can write your own")
        ll("code generators which take the AST as input and generate code for a target.")
        ll("This means that Seabass programs are theoretically extremely portable.")
        l("A SELF-MODIFYING SCRIPTING LANGUAGE")
        ll("Seabass lets you modify the internal structure of the compiler as well")
        ll("as the source code of the program being parsed, while it is being parsed,")
        ll("using interpreted code which runs immediately.")
        l("A COMPILER-COMPILER")
        ll("Since Seabass is built around the idea of writing custom syntaxes, you can")
        ll("think of Seabass sort of like Bison or Yacc in the sense that it enables")
        ll("you to write parsers for new programming languages.")
        nl
        l("STAGES OF COMPILATION")
        l("Here are the general stages:")
        ll("* Tokenization and Preprocessing")
        ll("* Parsing")
        ll("* Code generation")
        nl
        l("TOKENIZATION")
        l("In general, the tokenization and pre-processing stages are identical to C, with")
        l("a few noteworthy caveats:")
        ll("#define statements do not allow specifying arguments.")
        ll("Adjacent string literals are not merged.")
        ll("you are not allowed to use '\\0' in string or character literals. In fact, neither")
        ll(" hex nor octal character codes are allowed. If you desire to store arbitrary")
        ll(" bytes in a sequence, use a 'data' statement.")
        ll("There are no trigraphs or anything like that to worry about.")
        nl
        l("PREPROCESSING")
        ll("Includes are handled. Note that system directory includes (#include <>) are")
        ll(" directed to /usr/include/seabass/ by default.")
        ll("String literals are expanded including escape sequences.")
        ll("Character literals are converted to integer literals.")
        nl
        l("PARSING")
        ll("Seabass is implemented with a recursive descent parser. However, it is possible")
        ll("to write `codegen` functions called 'parsehooks' which allow writing custom syntaxes.")
        ll("if the parser sees \"@myidentifier\" it tries to look for parsehook_myidentifier, or")
        ll("parsehook_expr_myidentifier if you are using it inside of an expression.")
        ll("parsehooks have the ability to arbitrarily manipulate the Abstract Syntax Tree and")
        ll("manipulate the token stream fed to the parser. parsehook_expr_ functions are expected")
        ll("to assign to an expr_node** that they are given as input.")
        nl
        l("CODE GENERATION")
        ll("Seaass requires every compilation unit define a codegen function `codegen_main` taking no")
        ll("arguments and returning nothing, which is responsible for compiling all global symbols")
        ll("in the unit not defined 'codegen'.")
        nl
        l("COMPILATION UNIT")
        ll("The equivalents of the C compilation unit in Seabass are the symbol_table and type_table.")
        ll("the symbol_table and type_table contain everything* in Seabass which needs to be compiled for")
        ll("the target.")
        ll("* CAVEAT: Because Seabass is a metaprogramming language and allows for arbitrary compiletime")
        ll("code and data structures, it is possible that the unit has defined its own AST(s).")
        ll("In such a case, the code generator written must account for these other ASTs and compile them")
        ll("alongside or instead of the main compilation unit. This would be needed to define DSLs which")
        ll("compile to efficient JS, for instance, because some operations in SEABASS do not")
        ll("exist in JS (pointers, for instance).")
        nl
        l("BASE LEVEL LANGUAGE")
        ll("Seabass's base level syntax is a mixture of Lua, C++, and a tad of Rust.")
        ll("It is designed to be 'friendly' and 'easy to read/type', while also being")
        ll("generally more powerful than C. That said, there are some features from C")
        ll("which Seabass does not have. Listed by order of importance:")
        lll("* Use of structs by-value in expressions")
        lll("* 128 bit integral types.")
        lll("* long doubles (10 byte floats)")
        lll("* Complex numbers")
        lll("* sizeof(expression)")
        l("REASONS FOR THESE OMISSIONS:")
        ll("* Structs-by-value was omitted originally because the computer architecture I")
        ll("  designed (SISA-64) did not have registers larger than 64 bit. However, the")
        ll("  decision stuck once the method invocation syntax came around. I was worried")
        ll("  That this would have negative performance implications, however from my own testing,")
        ll("  this is shouldn't be an issue for performance. The compiler tends to optimize structs")
        ll("  passed by value into splitting the struct across multiple registers. If you really")
        ll("  need this, you can do it in seabass already.")
        ll("* 128 bit integral types were originally omitted for the same reason, because")
        ll("  S-ISA-64 does not have them. If I ever write Seabass in itself, I will make sure")
        ll("  to support 128 bit integral types.")
        ll("  HOWEVER! Due to the presence of the `ASM` statement in Seabass, it is possible")
        ll("  to simply write C code in the ASM statement which performs the 128 bit operations.")
        ll("  128 bit integers can, of course, be trivially split into two 64 bit integers, and as")
        ll("  stated before, it should compile the same way for `static inline`. Pre-compiled DLL")
        ll("  functions are the only real concern.")
        ll("* Long doubles were omitted because I think they are rarely useful. Once again, if you")
        ll("  desire to use them, you can write code which uses them inside of an `ASM` statement.")
        ll("  Long doubles can also be split into two arguments (a u64 and a short) so if you really")
        ll("  need to pass them into a function, it is possible.")
        ll("* Complex numbers were omitted because I find it unnecessary for the base-level of Seabass.")
        ll("* sizeof(expression) was omitted because I felt that sizeof(type) was enough.")
        nl
        l("WILL WRITING SEABASS INSTEAD OF C SLOW DOWN MY CODE?")
        ll("In general, no. The opposite should generally be the case.")
        ll("Seabass implements switch/case using pure dispatch tables which are faster than C, and")
        ll("implements explicit tail calls. This should make _most_ code much faster.")
        ll("However, in the specific and unusual circumstance that you absolutely must pass 128 bit")
        ll("values by register into a function which is not declared `inline` you *might* see a")
        ll("(miniscule) overhead, and really only if the number of function arguments is large.")
        ll("This might affect dynamically linked cryptography libraries which heavily use BigNums,")
        ll("however even that is unlikely.")
        ll("See the `optimization` manpage for more information.")
        nl
        l("LANGUAGE STRUCTURE DIFFERENCES FROM C")
        l("Here is a small summary of major differences from C as you are likely used to it:")
        ll("* Structs may never be lvalues or rvalues. The name of a struct variable is an rvalue pointer.")
        ll("* Structs may not be passed into or from functions, only pointers. The implications")
        ll("  of this were already explained before.")
        ll("* switch defines a dispatch table and does not performs a bounds check.")
        ll("* most statements do not require semicolons. They can even be omitted from expression")
        ll("  statements if it is not followed by an expression statement.")
        ll("* it is possible to use a variable within a scope before its declaration. This is true even")
        ll("  even before an initializer. It's an artifact of the parser/validator.")
        ll("* Constructors and destructors exist, like C++. They always take zero arguments")
        ll("  and return nothing.")
        nl
        l("SYNTAX")
        ll("see the 'syntax' manpage.")
        l(bar)
    };
    m(optimization){
        b(optimization)
        l("So, you would like to know how to make your code run fast?")
        l("Well, the general guidelines for all programming apply (better algorithms,")
        l("better data structures, n is usually small) but there are some which are")
        l("unique to Seabass. Those are provided here.")
        l(bar)
        l("TIP 1. USE SWITCH")
        ll("switch in Seabass uses pure unchecked dispatch tables, meaning it should")
        ll("generally be much faster than C's 'switch'.")
        l(bar)
        l("TIP 2. YOU CAN WRAP ANY FUNCTION CALL TO MAKE IT INTO PASS-BY-VALUE")
        ll("It is possible to write 2 wrapper functions to turn any pass-by-pointer function")
        ll("into a pass-by-value one, even in a separate compilation unit. You could automate")
        ll("the authoring of these wrappers yourself using metaprogramming.")
        nl
        ll("As an example, let us say there is an api call which takes in a struct")
        ll("and returns one of the same type which you wish to wrap. Then...")
        nl
        lll("myStruct myAPIcall__API(myStruct p); //C function wrapper for real impl")
        nl
        lll("fn inline myAPIcall__INTERN(myStruct* retval, myStruct* p); //real internal implementation")
        nl
        lll("fn inline myAPIcall(myStruct* retval, myStruct* p): //external wrapper")
        lll("asm(\"*__seabass_local_variable_retval = myAPIcall_API(*__seabass_local_variable_p);\");")
        lll("end")
        nl
        ll("would fully wrap myAPIcall as well as provide the standard interface to callers. all")
        ll("you need to do is replicate the external wrapper in the compilation units of callers.")
        ll("This pretty much fully bypasses any cost. You could automate the creation of these")
        ll("wrappers (as well as .hbas headers for callers) using metaprogramming, obviously.")
        l(bar)
        l("TIP 3. USE TAIL CALLS")
        ll("In seabass, it is possible to explicitly perform a tail call. When compiling")
        ll("C code at -O2 or higher, this should produce the proper tail call optimization.")
        l(bar)
        l("TIP 4. DON'T BE AFRAID TO USE METHODS")
        ll("the method syntax's similarity to C++ might scare you into believing")
        ll("that it will be slower than a normal function call. This isn't true.")
        ll("methods in Seabass simply pass a secret first argument called 'this', which")
        ll("is a pointer to the object. If you declare your method 'inline' it should be")
        ll("exactly as fast as a macro. `this` doesn't get used in the generated code, so")
        ll("you don't need to worry about ruining your C++ either.")
        l(bar)
        l("TIP 5. WRITE YOUR OWN ABSTRACTIONS")
        ll("It's what it's all about! If you realize there is a systemic inefficiency")
        ll("in your code which could be solved by creating some new piece of syntax, just")
        ll("implement it! Be your own compiler engineer!")
        
        l(bar)
    };
    m(switch_syntax){
        b(switch_syntax)
        l("Switch statements look like this:")
        nl
        ll("switch(expr) label_is0, label_is1, label_is2 ;")
        nl
        l("If 'expr' is evaluated to 0, it will go to the first label,")
        l("If 'expr' is evaluated to 1, it will go to the second label, etcetera")
        l("If 'expr' is greater than (or equal to) the number of labels, then it is undefined behavior.")
        l("This usually means crashing.")
        l("Switch in seabass is a pure dispatch table.")
        l(bar)
    };
    m(goto_labels){
        b(goto_labels)
        l("Goto labels have a colon BEFORE the name, not after:")
        nl
        ll(":myLabel")
        nl
        l(bar)
    };
    m(error_messages){
        b(error_messages)
        l("The `cbas` tool will emit syntax error messages which can sometimes be confusing.")
        l("This is especially true when you are working on metaprograms, where you don't even")
        l("get a filename or line number!")
        l("Here are some tips for working with cbas's error message system:")
        nl
        l("1. Read the message carefully")
        ll("I have written some useful information which may help you")
        ll("in the syntax error code, including some syntax tutorials.")
        ll("I myself am guilty of ignoring my own error messages, however")
        ll("in general, you should listen to what the compiler tells you.")
        l("2. Don't just look at that specific line, look at nearby lines!")
        ll("If you forget a semicolon or end, it can be especially difficult")
        ll("to tell from the error message what went wrong. It is particularly")
        ll("difficult to tell when you forget an `end`.")
        ll("If you get the dreaded `expr_parse_terminal failed` error, then")
        ll("your best shot is to look around.")
        l("3. Don't assume there are no compiler bugs.")
        ll("This entire compiler was written by-hand in C99 by a single person.")
        ll("While I have tested pretty much everything in the language I can think")
        ll("of, it's almost certain that the compiler still has bugs by the time")
        ll("you get it. When/if Seabass is ported into itself, this may change.")
        ll("All this said, the compiler has been tested on a _lot_ of code. You")
        ll("should check and re-check your code before you conclude that this")
        ll("tool has a bug.")
        l("4. This isn't C")
        ll("If you're anything like me, then the number one source of errors for")
        ll("you is probably your muscle memory. Using -> on pointers, trying to")
        ll("take the size of a variable, putting the array specifier after the")
        ll("name of a variable, you name it. You will have to either unlearn")
        ll("or re-learn your habits.")
        l("5. No curly braces")
        ll("Curly braces are _never_ used in seabass's base level language.")
        ll("If you are using curly braces, then you are doing it wrong.")
        l("6. \"Unexpected end of compilation unit\"")
        ll("Look at the end of the compilation unit. There's an unterminated symbol there.")
        ll("If you want a clearer error message, put an `end` at the very end of the compilation")
        ll("unit. If the error message doesn't change, then you have multiple unterminated `end`")
        ll("statements, or it's a compiler bug. If it's a compiler bug, try putting a semicolon")
        ll("on the very last line instead.")
        l("7. if, while, and for all require an `end`")
        ll("If you have a habit of writing single-liner if's from c, you will likely")
        ll("get hit with this one. It is very likely the cause of the aforementioned")
        ll("error message (Unexpected end of compilation unit).")
        l(bar)
    };
    m(metaprogramming){
        b(metaprogramming)
        l("Seabass's main 'power sources' are:")
        ll("1. Parsehooks")
        ll("2. Arbitrary Compiletime Execution")
        ll("3. Compiletime Reflection")
        ll("4. User-defined code generators")
        l("All of these are metaprogramming features")
        l(bar)
        ll("PARSEHOOKS")
        l(bar)
        l("Parsehooks are special codegen functions which can be invoked")
        l("during parsing.")
        l("see the `parsehook` manpage for more information.")
        l(bar)
        ll("COMPILETIME EXECUTION")
        l(bar)
        l("through the use of codegen functions/methods, it is possible to write seabass code")
        l("which runs at compiletime.")
        l("see the `codegen` manpage for more information.")
        l(bar)
        ll("COMPILETIME REFLECTION")
        l(bar)
        l("Codegen code has full access to the token stream of the parser, as well as the AST.")
        l("see the `reflection` manpage for more information.")
        l(bar)
        ll("CODE GENERATORS")
        l(bar)
        l("After parsing is finished, the compiler searches for `codegen_main` and calls it.")
        l("That function is responsible for emitting the final output.")
        l(bar)
    };
    
    
    ;
    r(help)
    r(license)
    r(index)
    r(terminology)
    r(syntax)
    r(error_messages)
    r(dedicatory)
    r(reflection)
    r(parsehook)
    r(keywords)
    r(types)
    r(qualifier_order)
    r(constexpr)
    r(switch_syntax)
    r(goto_labels)
    r(metaprogramming)
    r(codegen)
    r(undefined_behavior)
    r(function_pointers)
    r(short_circuiting)
    r(optimization)
    r(for_c_programmers)
    r(reflection_library_code)
    r(syntax_oop_example)
    r(builder_library_code)
    r(builder_example_code)
    r(syntax_fib_example)
    
    p("I didn't recognize that subject... try this:\ncbas -m help");
    exit(1);
}




