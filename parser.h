#ifndef PARSER_H
#define PARSER_H

#include "stringutil.h"
#include <stdint.h>
#include "targspecifics.h"
#include "libmin.h"
/*
    token type table
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

/*operator identifications*/
#define ID_OP(a) ((uint64_t)(a))
#define ID_OP2(a,b) (((uint64_t)(a)<<8)|(uint64_t)(b))
#define ID_OP3(a,b,c) (((uint64_t)(a)<<16)|((uint64_t)(b)<<8)|(uint64_t)(c))
#define ID_OP4(q,a,b,c) (((uint64_t)(q)<<32)|((uint64_t)(a)<<16)|((uint64_t)(b)<<8)|(uint64_t)(c))

void set_parser_echo(int v);
int get_parser_echo(void);


static inline int ID_IS_OP1(strll* i){
    if(i->data != TOK_OPERATOR) return 0;
    if(strlen(i->text) == 1) return 1;
    return 0;
}
static inline int ID_IS_OP2(strll* i){
    if(i->data != TOK_OPERATOR) return 0;
    if(strlen(i->text) == 2) return 1;
    return 0;
}
static inline int ID_IS_OP3(strll* i){
    if(i->data != TOK_OPERATOR) return 0;
    if(strlen(i->text) == 3) return 1;
    return 0;
}
static inline int ID_IS_OP4(strll* i){
    if(i->data != TOK_OPERATOR) return 0;
    if(strlen(i->text) == 4) return 1;
    return 0;
}


static inline uint64_t ID_KEYW_STRING(const char* s){
    if(streq("fn",s)) return 40000;
    if(streq("function",s)) return 40000;
    if(streq("func",s)) return 40000;
    if(streq("procedure",s)) return 40000;
    if(streq("proc",s)) return 40000;
    
    if(streq("cast",s)) return 1;
    
    if(streq("u8",s)) return 2;
    if(streq("char",s)) return 2;
    if(streq("byte",s)) return 2;
    if(streq("ubyte",s)) return 2;
    if(streq("uchar",s)) return 2;
    
    if(streq("i8",s)) return 3;
    if(streq("schar",s)) return 3;
    if(streq("sbyte",s)) return 3;
    
    if(streq("u16",s)) return 4;
    if(streq("ushort",s)) return 4;
    
    if(streq("i16",s)) return 5;
    if(streq("short",s)) return 5;
    if(streq("sshort",s)) return 5;
    
    if(streq("u32",s)) return 6;
    if(streq("uint",s)) return 6;
    if(streq("ulong",s)) return 6;
    
    if(streq("i32",s)) return 7;
    if(streq("int",s)) return 7;
    if(streq("sint",s)) return 7;
    if(streq("long",s)) return 7;
    if(streq("slong",s)) return 7;
    
    if(streq("u64",s)) return 8;
    if(streq("ullong",s)) return 8;
    if(streq("uqword",s)) return 8;
    if(streq("uptr",s)) return 80;
    if(streq("qword",s)) return 8;
    
    if(streq("i64",s)) return 9;
    if(streq("sllong",s)) return 9;
    if(streq("llong",s)) return 9;
    if(streq("sqword",s)) return 9;
    
    if(streq("f32",s)) return 10;
    if(streq("float",s)) return 10;
    
    if(streq("f64",s)) return 11;
    if(streq("double",s)) return 11;
    
    if(streq("break",s)) return 12;
    if(streq("data",s)) return 13;
    if(streq("string",s)) return 14;
    if(streq("end",s)) return 15;
    if(streq("continue",s)) return 16;
    if(streq("if",s)) return 17;
    if(streq("else",s)) return 18;
    if(streq("while",s)) return 19;

    if(streq("goto",s)) return 20;
    if(streq("jump",s)) return 20;

    if(streq("return",s)) return 22;
    if(streq("tail",s)) return 23;
    if(streq("sizeof",s)) return 24;
    if(streq("static",s)) return 25;
    
    if(streq("pub",s)) return 26;
    if(streq("public",s)) return 26;
    
    if(streq("struct",s)) return 27;
    if(streq("class",s)) return 27;
    if(streq("asm",s)) return 28;
    if(streq("method",s)) return 29;
    if(streq("predecl",s)) return 30;
    if(streq("codegen",s)) return 31;
    //constexpr
    if(streq("constexpri",s)) return 32;
    if(streq("constexprf",s)) return 33;
    if(streq("switch",s)) return 34;
    if(streq("for",s)) return 35;
    
    if(streq("elif",s)) return 36;
    if(streq("elseif",s)) return 36;

    if(streq("pure",s)) return 37;
    if(streq("inline",s)) return 38;
    /*atomic and volatile keywords.*/
    if(streq("atomic",s)) return 39;
    if(streq("volatile",s)) return 40;

    if(streq("getfnptr",s)) return 41;
    if(streq("callfnptr",s)) return 42;
    if(streq("getglobalptr",s)) return 43;
    if(streq("noexport",s)) return 44;
    if(streq("union",s)) return 45;
    puts("Internal Error: Unknown keyw_string, add it:");
    puts(s);
    exit(1);
    return 0;
}

static inline uint64_t ID_KEYW(strll* s){
    if(s->data != TOK_KEYWORD) return 0;
    return ID_KEYW_STRING(s->text);
}


extern int peek_always_not_null;
/*Used for automatically generating symbols for anonymous data.*/
extern uint64_t symbol_generator_count;
//char* gen_reserved_sym_name();

extern int saved_argc;
extern char** saved_argv;

strll* peek();
strll** getnext();
/*lookahead by n*/
strll* peekn(unsigned n);
strll* consume();
extern strll* next;
static void parse_error(char* msg){
    char buf[128];
    if(msg)
    puts(msg);
    if(next){
        if(next->filename){
            fputs("File:Line:Col\n",stdout);
            fputs(next->filename,stdout);
            fputs(":",stdout);
        } else{
            exit(1);
        }
        mutoa(buf,next->linenum);
        fputs(buf,stdout);
        fputs(":",stdout);

        mutoa(buf,next->colnum);
        fputs(buf,stdout);
        fputs("\n",stdout);

        puts("~~~~\nNote that the line number and column number");
        puts("are where the parser invoked parse_error.");
        puts("\n\n(the actual error may be slightly before,\nor on the previous line)");
        puts("I recommend looking near the location provided,\nnot just that exact spot!");
    }
    //TODO: emit informative errors...
    if(msg){
        if(strfind(msg, "__cbas_run_fn") != -1){
            puts("__cbas_run_fn allows you to execute a codegen function by name at global scope.");
            puts("the syntax is \n__cbas_run_fn my_function");
            puts("The function must return nothing and have zero arguments");
            puts("Consider writing a parsehook and using the metaprogramming operator (@) instead.");
            goto very_end;
        }
        if(strfind(msg, "parsehook") != -1){
            puts("parsehooks are special codegen function which are designed to hook into the parser.");
            puts("They are declared like this\nfn codegen parsehook_myParsehook():\n    /*body..*/\nend");
            puts("You could then call it like this: @myParsehook");
            puts("What a parsehook does is entirely up to you!");
            goto very_end;
        }
        if(strfind(msg, "stray") != -1){
            puts("it would appear you've put something where you're not supposed to...");
            goto very_end;
        }
        if(strfind(msg, "stray") != -1){
            puts("it would appear you've put something where you're not supposed to...");
        }
        if(
            (strfind(msg, "Expected") != -1) ||
            (strfind(msg, "expected") != -1)
        ){
            puts("The parser was expecting something different than it found...");
        }
        if(
            (strfind(msg, "Invalid") != -1) ||
            (strfind(msg, "invalid") != -1)
        ){
            puts("The parser knew what it wanted, but you didn't provide it...");
        }if(
            (strfind(msg, "Internal") != -1) ||
            (strfind(msg, "internal") != -1)
        ){
            puts("Metaprogram (or compiler) bugs are haunting you. Oooo! spooky!");
        }
        if(
            (strfind(msg, "Unrecognized configuration option") != -1)
        ){
            puts("The # configuration options are as follows:");
            puts("__CBAS_TARGET_WORDSZ");
            puts("__CBAS_TARGET_MAX_FLOAT");
            puts("__CBAS_TARGET_DISABLE_FLOAT");
            puts("__CBAS_BAKE");
            puts("__CBAS_STOP_BAKE");
            puts("__CBAS_TERMINATE");
        }
        if(strfind(msg, "String literal at global scope") != -1){
            puts("You're not allowed to put string literals at global scope.");
            puts("If you want a global string variable, use a `data` statement.");
        }
        if(strfind(msg, "codegen_main") != -1){
            puts("Here is an empty codegen_main for you to paste into your code:");
            puts("fn codegen codegen_main():\nend");
        }
        if(strfind(msg, "type parsing requires a type name!") != -1){
            puts("You most likely mistyped the name of your type.");
            if(next)
            if(next->text){
                puts("I can tell you right now, this is not a typename:");
                fflush(stdout); //prepare for disaster
                puts(next->text);
            }
        }
        if(strfind(msg, "public and static") != -1){
            puts("Been drinking too much coffee lately?");
            puts("public static void main(String[] args) much?");
            puts("public means 'externally visible' while static means");
            puts("'privately visible'. They're opposites.");
        }
        if(strfind(msg, "Defining the builtins?") != -1){
            puts("It appears you are trying to declare a symbol with a name reserved");
            puts("by the compiler. Either you know perfectly well what you're doing, or");
            puts("this happened by mistake. If you know what you're doing, you brought this");
            puts("on yourself. If this is a mistake, know simply that __builtin is a reserved");
            puts("namespace in seabass. If you don't believe you've violated this rule, then");
            puts("a metaprogram or macro is likely at fault.");
        }
        if(strfind(msg, "expected semicolon or comma") != -1 ){
            puts("Data statements and switch statements expect a comma-separated list of entries terminated with a semicolon.");
            puts("For data statements, it's a comma-separated list of constant expressions.");
            puts("For switch statements, it's a list of labels. You can actually omit the commas, by the way.");
            puts("But the semicolon is necessary...");
        }
        else if(strfind(msg, "Struct declaration") != -1 ){
            puts("Struct declarations are not like C. You don't use curly braces.");
            puts("Here is an example struct declaration:");
            puts("struct mystruct\n    int myInt\n    float[3] myFloats\nend");
            puts("you can insert the following qualifiers inside the body of the struct declaration:");
            puts("noexport union");
            puts("noexport says that codegen_main shouldn't compile the type into generated code.");
            puts("union says that all members of the struct should share the same memory.");
        }
        else if(strfind(msg, "expected semicolon") != -1 ){
            puts("A missing semicolon! Ah, the joys of programming...");
        }
        if(strfind(msg, "mismatch") != -1 ){
            puts("It would appear you've predeclared a symbol before, and then tried to re-define its prototype/type.");
            puts("This obviously is not allowed.");
        }
        if(strfind(msg, "data statement") != -1 ){
            puts("Ah, the data statement!");
            puts("Seabass does not support array or struct initializers. If you want a block of global data,");
            puts("you use the data statement.");
            puts("the data statement begins with the keyword 'data' followed by some number of");
            puts("qualifiers (IN ORDER- predecl codegen noexport pub static) followed by a type");
            puts("or 'string' and finally the data itself.");
            puts("\ndata statements come in two flavors: string and normal.");
            puts("Normal data statements look like this:");
            puts("    data int myInts 1+1, 2+2, 47 ^ 39, 255 * 3;");
            puts("But string data statements look like this:");
            puts("    data string mytext \"Hello World!\";");
            puts("\nThe purpose of the data statement is to allow large amounts of data to be embedded within");
            puts("the source code of your program. You may also recognize it as a relic of BASIC- from which");
            puts("the original idea for the 'data' statement comes.");
        }
        if(strfind(msg, "codegen and atomic") != -1){
            puts("This is a single-threaded compiler. You don't need to worry about that.");
        }
        if(strfind(msg, "atomic and volatile") != -1){
            puts("Atomic means another thread can change it at any time, implicitly.");
            puts("Therefore, it doesn't make sense to have both qualifiers.");
        }
        if(strfind(msg, "Cannot assign literal to type.") != -1){
            puts("You can't provide an initializer to a struct or array.");
        }
        if(strfind(msg, "Array") != -1){
            puts("Ah! Arrays! They are declared like this in Seabass:");
            puts("    int[50] myintegers");
            puts("you can replace '50' with a constant expression.");
            puts("The array size must be greater than zero.");
        }
        if(
            (strfind(msg, "cexpr") != -1) ||
            (strfind(msg, "constexpr") != -1) ||
            (strfind(msg, "const expr") != -1) ||
            (strfind(msg, "Constexpr") != -1) ||
            (strfind(msg, "Const expr") != -1) ||
            (strfind(msg, "Const Expr") != -1) ||
            (strfind(msg, "const Expr") != -1) ||
            (strfind(msg, "Constant Expr") != -1) ||
            (strfind(msg, "Constant expr") != -1) ||
            (strfind(msg, "constant Expr") != -1) ||
            (strfind(msg, "constant expr") != -1)
        ){
            puts("Constant expressions are evaluated immediately at parse-time.");
            puts("You may not use local variables in a constant expression, or");
            puts("indeed any variable not defined 'codegen'. Furthermore, only");
            puts("primitive codegen variables (not pointers or structs or arrays)");
            puts("may be used.");
        }
        if(
            (strfind(msg, "Global Variable") != -1) ||
            (strfind(msg, "global Variable") != -1) ||
            (strfind(msg, "Global variable") != -1) ||
            (strfind(msg, "global variable") != -1) 
        ){
            puts("Ah, global variables! The worst nightmare of every Haskell programmer.");
            puts("Remember the qualifier order:");
            puts("    predecl codegen noexport pub static atomic volatile int***[49*3] myvarname;");
            puts("\nOf course, it's not valid to have all those qualifiers at once... it's just to show you");
            puts("the syntax...");
        }
        
    }
    very_end:;
    exit(1);
}
static inline void require(int cond, char* msg){
    if(!cond)parse_error(msg);
}
int peek_is_operator();
int peek_is_semic();
strll* consume_semicolon(char* msg);
void require_peek_notnull(char* msg);



/*Specific functions provided by other files...*/

int peek_is_typename();
int string_is_typename(char* text);


/*Consumption*/

int64_t parse_cexpr_int();
double parse_cexpr_double();
void unit_throw_if_had_incomplete();
uint64_t parse_stringliteral();
void parse_continue_break();
void parse_fn(int is_method);
void parse_method();
void parse_fbody();
void parse_stmts();
void parse_stmt();


/*safe memory allocation*/
void* m_allocX(uint64_t sz);
void* c_allocX(uint64_t sz);
void* re_allocX(void* p, uint64_t sz);

void consume_keyword(char* s);





#endif
