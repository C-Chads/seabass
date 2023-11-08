
#include "stringutil.h"
#include "targspecifics.h"

#include "parser.h"
#include "data.h"
#include "metaprogramming.h"
#include "astexec.h"

strll* unit;
strll* next;
int peek_always_not_null = 0;
uint64_t symbol_generator_count = 1;


/*
    
struct mystruct{
    _Alignas(16) int a;
};

struct mystruct* myfunction(struct mystruct* b){
    return b+1;
}
union myunion{
    _Alignas(16) int a;
    float b;
};

union myunion* myfunction2(union myunion* b){
    return b+1;
}
*/


void parse_expr(expr_node** targ);
void validate_function(symdecl* funk);
/*global statement parsers*/
static void parse_gvardecl();
static void parse_datastmt();
static void parse_structdecl();
/*struct-level parsing*/
static inline void parse_struct_member(uint64_t sid); /*type ident*/


/*returns an owning pointer!*/
static inline char* gen_reserved_sym_name(){
    char buf[100];
    char* retval;
    if(symbol_generator_count == 0){
        parse_error("Too many anonymous symbols");
    }
    mutoa(buf, symbol_generator_count++);
    buf[99] = 0;
    retval = strcata("__seabass_anonymous_symbol_",buf);
    return retval;
}

strll* peek(){
    if(peek_always_not_null)
    if(next == NULL) {
        puts("Uh oh! Unexpected end of compilation unit!");
        if(active_function)
            puts("The parser was inside of a function. You probably forgot an end.");
        puts("I suggest two strategies for investigating this error:");
        puts("1. Stick `end` statements at the end of the unit until the error");
        puts("   goes away or changes.");
        puts("2. Stick a semicolon at the end of the unit and see if that gets");
        puts("   rid of the error.");
        puts("If it's 2, then it's very likely you've encountered a (well-known)");
        puts("recurring compiler bug. code_validator.c is probably at fault.");
        puts("\nThis error often happens because you wrote single-liner ifs, elifs, or whiles.");
        puts("This is not C. You may not write single-liner if statements. If a statement");
        puts("implicitly creates a scope, that scope is terminated either by the next element");
        puts("in an if/elif/else chain, or `end`. Search the source code in your editor");
        puts("for such a mismatch.");
        parse_error("Unexpected end of compilation unit");
    
    }
    return next;
}
strll** getnext(){
    return &next;
}

strll* peekn(unsigned n){
    strll* retval = next;;
    while(n){
        n--;
        if(retval == NULL) return NULL;
        retval = retval->right;
    }
    return retval;
}

//print a string out.
static void print_string_out(char* text){
    putchar('\"');
    for(;text[0];text++){
        if(text[0] == '\n'){
            putchar('\\');
            putchar('n');
        } else if(text[0] == '\a'){
            putchar('\\');
            putchar('a');
        }else if(text[0] == '\"'){
            putchar('\\');
            putchar('\"');
        }else if(text[0] == '\b'){
            putchar('\\');
            putchar('b');
        }else if(text[0] == '\f'){
            putchar('\\');
            putchar('f');
        }else if(text[0] == '\e'){
            putchar('\\');
            putchar('e');
        }else if(text[0] == '\\'){
            putchar('\\');
            putchar('\\');
        }else if(text[0] == '\r'){
            putchar('\\');
            putchar('r');
        }else if(text[0] == '\t'){
            putchar('\\');
            putchar('t');
        }else if(text[0] == '\v'){
            putchar('\\');
            putchar('v');
        } else{
            putchar(text[0]);
        }
    }
    putchar('\"');
}

//handle 

strll* consume(){
    strll* i;
    strll* right;
    i = next;
    right = next->right;

#ifdef COMPILER_CLEANS_UP
    {
        if(next->text){
            free(next->text);
        }
    }
    free(next);
#endif
        
    next = right;
    return i;
}






static inline void parse_do_metaprogramming(){
        char* t;
        uint64_t i;
        uint64_t id;
        int found = 0;
        consume();
        require(peek()->data == TOK_IDENT, "Parserhook requires identifier");
        //require(peek()->text != NULL, "Internal error, identifier does not have text?");
        t = strdup(peek()->text);
        t = strcataf2("parsehook_",t);
        for(i = 0; i < nsymbols; i++){
            if(streq(symbol_table[i]->name, t)){
                id = i;
                found = 1;
                break;
            }
        }
        require(found != 0, "Could not find parsehook_ function corresponding with ");
        free(t);
        consume(); //eat the identifier!
        require(symbol_table[id]->t.is_function != 0, "parsehook must be a function.");
        require(symbol_table[id]->is_codegen != 0, "parsehook must be is_codegen.");
        require(symbol_table[id]->is_incomplete == 0, "parsehook definition must be completed.");
        require(symbol_table[id]->fbody != NULL, "parsehook function body must not be null.");
        require(symbol_table[id]->t.basetype == BASE_VOID, "parsehook must return nothing!");
        require(symbol_table[id]->t.pointerlevel == 0 , "parsehook must return nothing, not even a pointer to nothing!");
        require(symbol_table[id]->nargs == 0, "parsehook must take zero arguments. That's how I call it.");
        ast_vm_stack_push_temporary();
        ast_execute_function((symbol_table+id)[0]);
        ast_vm_stack_pop();
        return;
}

static void parse_repeatedly_try_metaprogramming(){
    while(1){
        if(peek() == NULL) return;
        if(peek()->data != TOK_OPERATOR) return;
        if(!streq(peek()->text, "@")) return;
        parse_do_metaprogramming();
    }
}


void require_peek_notnull(char* msg){if(peek() == NULL)parse_error(msg);}
void astdump();
void set_target_word(uint64_t val);
void set_max_float_type(uint64_t val);

void compile_unit(strll* _unit){
    unit = _unit;
    next = unit;
    while(1){
        peek_always_not_null = 0;
        if(peek() == NULL) {
            break;
        }
        if(peek()->data == TOK_IDENT)
        if(streq(peek()->text, "__cbas_run_fn")){
            char* t;
            uint64_t i;
            uint64_t id;
            int found = 0;
            consume();
            require(peek() != NULL, "__cbas_run_fn requires identifier.");
            require(peek()->data == TOK_IDENT, "__cbas_run_fn requires identifier");
            t = strdup(peek()->text);
            for(i = 0; i < nsymbols; i++){
                if(streq(t, symbol_table[i]->name)){
                    id = i;
                    found = 1;
                    break;
                }
            }
            require(found != 0, "Could not find __cbas_run_fn function");
            free(t);
            consume();
            require(symbol_table[id]->t.is_function != 0, "__cbas_run_fn- must be a function.");
            require(symbol_table[id]->is_codegen == 1, "__cbas_run_fn- must be is_codegen.");
            require(symbol_table[id]->is_incomplete == 0, "__cbas_run_fn- definition must be completed.");
            require(symbol_table[id]->fbody != NULL, "__cbas_run_fn- function body must not be null.");
            require(symbol_table[id]->t.basetype == BASE_VOID, "__cbas_run_fn- must return nothing!");
            require(symbol_table[id]->t.pointerlevel == 0 , "__cbas_run_fn- must return nothing");
            require(symbol_table[id]->nargs == 0, "__cbas_run_fn- must take zero arguments. That's how I call it.");
            //ast_execute_function(symbol_table+id);
                        ast_vm_stack_push_temporary();
                  ast_execute_function((symbol_table+id)[0]);
                  ast_vm_stack_pop();
            continue;
        }

        if(peek()->data == TOK_SEMIC){
            //ignore it!
            consume();
            continue;
        }
        
        if(peek()->data == TOK_OPERATOR)
            if(streq(peek()->text, "@")){
                char* t;
                uint64_t i;
                uint64_t id;
                int found = 0;
                consume();
                require(peek() != NULL, "parsehook requires identifier.");
                require(peek()->data == TOK_IDENT, "parsehook requires identifier");
                t = strdup(peek()->text);
                t = strcataf2("parsehook_",t);
                for(i = 0; i < nparsehooks; i++){
                    uint64_t the_parsehook = parsehook_table[i];
                    if(streq(symbol_table[the_parsehook]->name, t)){
                        id = the_parsehook;
                        found = 1;
                        break;
                    }
                }
                require(found != 0, "Could not find parsehook_ function");
                free(t);
                consume();
                require(symbol_table[id]->is_incomplete == 0, "parsehook definition must be completed.");
                require(symbol_table[id]->fbody != NULL, "parsehook function body must not be null.");

                //ast_execute_function(symbol_table+id);
                ast_vm_stack_push_temporary();
                ast_execute_function((symbol_table+id)[0]);
                ast_vm_stack_pop();
                continue;
            }


        if(peek()->data == TOK_KEYWORD)
        if(peek_match_keyw("end"))
            {
                parse_error("stray 'end' in global scope.\nProbably a scope mismatch.");
            }

        if(peek()->data == TOK_MACRO_OP)
        if(streq(peek()->text, "#")){
            consume();
            if(peek() == NULL){
                parse_error("stray # at end of compilation unit.");
            }
            /**/
            if(peek()->data == TOK_IDENT){
                if(streq(peek()->text, "__CBAS_TARGET_WORDSZ")){
                    uint64_t a;
                    consume();
                    peek_always_not_null = 1;
                    require(peek()->data == TOK_INT_CONST, "Expected integer constant- target word size.");
                    a = matou(peek()->text);
                    consume();
                    if(a == 64){
                        set_target_word(BASE_U64);
                    }else if(a == 32){
                        set_target_word(BASE_U32);
                    }else if(a == 16){
                        set_target_word(BASE_U16);
                    }else if(a == 8){
                        set_target_word(BASE_U8);
                    } else{
                        parse_error("Invalid target word size. Valid values are 64, 32, 16, and 8. 128 bit is not supported.");
                    }
                    peek_always_not_null = 0;
                    continue;
                } else if(streq(peek()->text, "__CBAS_TARGET_MAX_FLOAT")){
                    uint64_t a;
                    consume();
                    peek_always_not_null = 1;
                    require(peek()->data == TOK_INT_CONST, "Expected integer constant- target max float size.");
                    a = matou(peek()->text);
                    consume();
                    if(a == 64){
                        set_max_float_type(BASE_F64);
                    } else if(a == 32){
                        set_max_float_type(BASE_F32);
                    } else{
                        parse_error("Invalid target max float size. 64 and 32 bit are allowed- no 10 byte floats!");
                    }
                    peek_always_not_null = 0;
                    continue;
                } else if(streq(peek()->text, "__CBAS_TARGET_DISABLE_FLOAT")){
                    consume();
                    set_max_float_type(0);
                    continue;
                } else if(streq(peek()->text, "__CBAS_TERMINATE")){
                    puts("//Compilation Terminated!");
                    exit(0);
                } else {
                    parse_error("Unrecognized configuration option.");
                }
            } else{
                parse_error("# followed by non-keyword parsed by program.");
            }
            continue;
        }

        if(peek()->data == TOK_STRING){
            parse_error("String literal at global scope");
            continue;
        }
        if(peek()->data == TOK_KEYWORD){
            if(peek_match_keyw("data")){
                peek_always_not_null = 1;
                parse_datastmt();
                peek_always_not_null = 0;
                continue;
            }
            if(peek_match_keyw("struct")){
                peek_always_not_null = 1;
                parse_structdecl();
                peek_always_not_null = 0;
                continue;
            }
            if(peek_match_keyw("fn")){
                peek_always_not_null = 1;
                parse_fn(0);
                peek_always_not_null = 0;
                continue;
            }
            if(peek_match_keyw("method")){
                peek_always_not_null = 1;
                parse_method();
                peek_always_not_null = 0;
                continue;
            }
        }

        peek_always_not_null = 0;
        if(peek() == NULL) {
            break;
        }
        /*lastly, it must be the variable declaration.*/
        peek_always_not_null = 1;
        parse_gvardecl();
        peek_always_not_null = 0;
    }

    //astdump();
    //search for the codegen function and try to execute it.
    {unsigned long i;
        for(i = 0; i < nsymbols; i++){
            if(streq(symbol_table[i]->name, "codegen_main")){
                require(symbol_table[i]->is_codegen > 0, "codegen_main must be declared codegen.");
                require(symbol_table[i]->t.is_function, "codegen_main must be a function.");
                require(symbol_table[i]->t.basetype == BASE_VOID, "codegen_main must return nothing!");
                require(symbol_table[i]->t.pointerlevel == 0 , "codegen_main must return nothing, not even a pointer to nothing!");
                require(symbol_table[i]->nargs == 0, "codegen_main must take zero arguments. That's how I call it.");
                //IT'S TIME!!!!
                //ast_execute_function(symbol_table+i);
                ast_vm_stack_push_temporary();
                //ast_execute_function(symbol_table+i);
                ast_execute_function((symbol_table+i)[0]);
                ast_vm_stack_pop();
                exit(0);
            }
        }
    }
    astdump();
    exit(0);
}

void parse_global(){
        if(peek()->data == TOK_IDENT){
            if(streq(peek()->text, "__cbas_run_fn")){
                char* t;
                uint64_t i;
                uint64_t id;
                int found = 0;
                    consume();
                    require(peek() != NULL, "__cbas_run_fn requires identifier.");
                    require(peek()->data == TOK_IDENT, "__cbas_run_fn requires identifier");
                    t = strdup(peek()->text);
                    for(i = 0; i < nsymbols; i++){
                        if(streq(t, symbol_table[i]->name)){
                            id = i;
                            found = 1;
                            break;
                        }
                    }
                    require(found != 0, "Could not find __cbas_run_fn function");
                    free(t);
                    consume();
                    require(symbol_table[id]->t.is_function != 0, "__cbas_run_fn- must be a function.");
                    require(symbol_table[id]->is_codegen == 1, "__cbas_run_fn- must be is_codegen.");
                    require(symbol_table[id]->is_incomplete == 0, "__cbas_run_fn- definition must be completed.");
                    require(symbol_table[id]->fbody != NULL, "__cbas_run_fn- function body must not be null.");
                    require(symbol_table[id]->t.basetype == BASE_VOID, "__cbas_run_fn- must return nothing!");
                    require(symbol_table[id]->t.pointerlevel == 0 , "__cbas_run_fn- must return nothing");
                    require(symbol_table[id]->nargs == 0, "__cbas_run_fn- must take zero arguments. That's how I call it.");
                    //ast_execute_function(symbol_table+id);
                    ast_vm_stack_push_temporary();
                    ast_execute_function((symbol_table+id)[0]);
                    ast_vm_stack_pop();
                    return;
            }
        }
        if(peek()->data == TOK_SEMIC){
            //ignore it!
            consume();
            return;
        }
        parse_repeatedly_try_metaprogramming(); //allow parsehooks at global scope as invoked by @global
    if(peek_match_keyw("data")){
        parse_datastmt();
        return;
    }
    if(peek_match_keyw("struct")){
        parse_structdecl();
        return;
    }
    if(peek_match_keyw("fn")){
        parse_fn(0);
        return;
    }
    if(peek_match_keyw("method")){
        parse_method();
        return;
    }
    if(peek()->data == TOK_MACRO_OP)
            if(streq(peek()->text, "#")){
                consume();
                if(peek() == NULL){
                    parse_error("stray # at end of compilation unit.");
                }
                /**/
                if(peek()->data == TOK_IDENT){
                    if(streq(peek()->text, "__CBAS_TARGET_WORDSZ")){
                        uint64_t a;
                        consume();
                        require(peek()->data == TOK_INT_CONST, "Expected integer constant- target word size.");
                        a = matou(peek()->text);
                        consume();
                        if(a == 64){
                            set_target_word(BASE_U64);
                        }else if(a == 32){
                            set_target_word(BASE_U32);
                        }else if(a == 16){
                            set_target_word(BASE_U16);
                        }else if(a == 8){
                            set_target_word(BASE_U8);
                        } else{
                            parse_error("Invalid target word size. Valid values are 64, 32, 16, and 8. 128 bit is not supported.");
                        }
                        return;
                    } else if(streq(peek()->text, "__CBAS_TARGET_MAX_FLOAT")){
                        uint64_t a;
                        consume();
                        require(peek()->data == TOK_INT_CONST, "Expected integer constant- target max float size.");
                        a = matou(peek()->text);
                        consume();
                        if(a == 64){
                            set_max_float_type(BASE_F64);
                        } else if(a == 32){
                            set_max_float_type(BASE_F32);
                        } else{
                            parse_error("Invalid target max float size. 64 and 32 bit are allowed- no 10 byte floats!");
                        }
                        return;
                    } else if(streq(peek()->text, "__CBAS_TARGET_DISABLE_FLOAT")){
                        consume();
                        set_max_float_type(0);
                        return;
                    } else if(streq(peek()->text, "__CBAS_TERMINATE")){
                        puts("//Compilation Terminated!");
                        exit(0);
                    } else {
                        parse_error("Unrecognized configuration option.");
                    }
                } else{
                    parse_error("# followed by non-keyword parsed by program.");
                }
                return;
            }
    parse_error("<CODEGEN ERROR> Codegen code called __builtin_parse_global() I didn't find any global to parse!");
}

static inline uint64_t peek_basetype(uint64_t* sid){
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
                sid[0] = i;
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
            if(symbol_table[i]->t.is_function)
                if(streq(symbol_table[i]->name, peek()->text)){
                    return BASE_FUNCTION;
                }
    }
}


type parse_type(){
    uint64_t sid;
    type t = type_init();
    require(peek_is_typename(),"type parsing requires a type name!");
    t.basetype = peek_basetype(&sid); //TODO: optimize this, don't search the list of structs twice! (161 of data.c)
    if(t.basetype == BASE_STRUCT){
        t.structid = sid;
    }
    consume();
    while(
            peek()->data == TOK_OPERATOR && 
            peek()->text && 
            streq(peek()->text, "*")
    ){
        t.pointerlevel++;
        consume();
    }
    if(peek()->data == TOK_OBRACK){
        consume(); /*Eat the opening bracket!*/
        parse_repeatedly_try_metaprogramming(); //Allow parsehooks to be invoked to define the contents of an array declaration...
        t.arraylen = parse_cexpr_int();
        require((t.arraylen > 0), "Array must be of greater than zero length.");
        require(peek()->data == TOK_CBRACK, 	"Array type requires closing bracket.");
        consume(); /*eat that closing bracket!*/
    }
    return t;
}

static void parse_gvardecl(){
    uint64_t is_pub = 0;
    uint64_t is_new_symbol = 0;
    uint64_t is_predecl = 0;
    uint64_t is_codegen = 0;
    uint64_t is_atomic = 0;
    uint64_t is_noexport = 0;
    uint64_t is_volatile = 0;
    int had_equals = 0;
    type t = {0};
    int64_t cval;
    double fval;
    uint64_t symid = 0xFFffFFffFFff;
    set_is_codegen(0);
    gvar_qualifier_top:;
    parse_repeatedly_try_metaprogramming();
    if(peek()->data == TOK_KEYWORD)
        if(ID_KEYW(peek()) == ID_KEYW_STRING("predecl")){
            consume();
            is_predecl = 1;
            goto gvar_qualifier_top;
        }
    if(peek()->data == TOK_KEYWORD)
        if(ID_KEYW(peek()) == ID_KEYW_STRING("codegen")){
            consume();
            is_codegen = 1;
            set_is_codegen(1);
            goto gvar_qualifier_top;
        }
    if(peek()->data == TOK_KEYWORD)
        if(ID_KEYW(peek()) == ID_KEYW_STRING("noexport")){
            consume();
            is_noexport = 1;
            goto gvar_qualifier_top;
        }
    if(peek()->data == TOK_KEYWORD)
        if(ID_KEYW(peek()) == ID_KEYW_STRING("pub")){
            is_pub = 1;
            consume();
            goto gvar_qualifier_top;
        }
    if(peek()->data == TOK_KEYWORD)
        if(ID_KEYW(peek()) == ID_KEYW_STRING("static")){
            require(!is_pub, "Cannot have public and static, they are opposites!");
            is_pub = 0;
            consume();
            goto gvar_qualifier_top;
        }
    if(peek()->data == TOK_KEYWORD)
        if(ID_KEYW(peek()) == ID_KEYW_STRING("atomic")){
            require(is_codegen == 0, "Cannot have both codegen and atomic qualifiers on a global variable declaration.");
            require(is_volatile == 0, "Cannot have both atomic and volatile qualifiers on a declaration.");
            is_atomic = 1;
            consume();
            goto gvar_qualifier_top;
        }
    if(peek()->data == TOK_KEYWORD)
        if(ID_KEYW(peek()) == ID_KEYW_STRING("volatile")){
            require(is_atomic == 0, "Cannot have both atomic and volatile qualifiers on a declaration.");
            is_volatile = 1;
            consume();
            goto gvar_qualifier_top;
        }
    if(!peek_is_typename()){
        puts("You messed up something at global scope.");
        puts("The parser thinks you're trying to declare a global variable and failed at the syntax");
        puts("This often happens because you have too many `end` statements");
        puts("Try looking above where this error is happening for a duplicate `end`.");
        puts("REMEMBER:");
        puts("if(),elif(),else, while(), for(,,) all require an `end`\n    (or for if/elif/else, they need the next element in the chain). This is not C.");
        parse_error("Some shenanigans with scopes...");
    }
    
    //require(peek_is_typename(),"Global variable declaration parsing requires a typename");
    t = parse_type();
    require(type_can_be_variable(t), "Invalid type for global variable.");
    require(peek()->data == TOK_IDENT, "Identifier required in global variable declaration.");
    require(!is_builtin_name(peek()->text),"Hey, What are you tryna pull? Defining the builtins? As global variables? Huh?");
    t.is_lvalue = 1;
    if(t.arraylen > 0) 				t.is_lvalue = 0;
    if(t.pointerlevel == 0){
        if(t.basetype == BASE_STRUCT){
            t.is_lvalue = 0;
            require(type_table[t.structid].is_incomplete == 0, "Internal error- this type is tagged incomplete. Huh?");
        }
    }

    if(peek_ident_is_already_used_globally()){
        unsigned long i;
        int found = 0;
        is_new_symbol = 0;
        /*Check if it is an incomplete compatible declaration.*/
        for(i = 0; i < nsymbols; i++){
            if(streq(symbol_table[i]->name, peek()->text)){
                found = 1;
                /*check for compatible declaration.*/
                if(!is_predecl)
                    require(!!symbol_table[i]->is_incomplete, "Attempted to redefine complete Global Variable.");
                
                require(symbol_table[i]->is_codegen == is_codegen,"Global Variable Predeclaration-definition mismatch (is_codegen)");
                require(symbol_table[i]->is_noexport == is_noexport,"Global Variable Predeclaration-definition mismatch (is_noexport)");
                require(symbol_table[i]->is_volatile == is_volatile,"Global Variable Predeclaration-definition mismatch (is_volatile)");
                require(symbol_table[i]->is_atomic == is_atomic,"Global Variable Predeclaration-definition mismatch (is_atomic)");
                require(symbol_table[i]->is_pub == is_pub,"Global Variable Predeclaration-definition mismatch (is_pub)");
                require(symbol_table[i]->t.basetype == t.basetype, "Global Variable Predeclaration-definition mismatch (basetype)");
                require(symbol_table[i]->t.pointerlevel == t.pointerlevel, "Global Variable Predeclaration-definition mismatch (pointerlevel)");
                require(symbol_table[i]->t.arraylen == t.arraylen, "Global Variable Predeclaration-definition mismatch (arraylen)");
                require(symbol_table[i]->t.is_function == t.is_function, "Global Variable Predeclaration-definition mismatch (is_function)");
                require(symbol_table[i]->t.is_lvalue == t.is_lvalue, "Global Variable Predeclaration-definition mismatch (is_lvalue)");
                if(t.basetype == BASE_STRUCT){
                    require(symbol_table[i]->t.structid == t.structid, "Global Variable Predeclaration-definition mismatch");
                }
                if(t.basetype == BASE_FUNCTION){
                    require(symbol_table[i]->t.funcid == t.funcid, "Global Variable Predeclaration-definition mismatch (funcid)");
                }
                symid = i;
                break;
            }
        }
        if(found == 0)
            parse_error("Unknown conflicting identifier usage for data stmt name...");
    }else{
        require(
            peek_ident_is_already_used_globally() == 0,
            "Identifier for global variable is already in use."
        );
        is_new_symbol = 1;
        symid = nsymbols;
        
        symbol_table = re_allocX(symbol_table, (++nsymbols) * sizeof(symdecl*));
        //printf("Symbol %zu\n", nsymbols);
        symbol_table[symid] = malloc(sizeof(symdecl));
        symbol_table[symid][0] = symdecl_init();
        symbol_table[symid]->t = t;
        symbol_table[symid]->name = strdup(peek()->text);
        symbol_table[symid]->is_pub = is_pub;
        symbol_table[symid]->is_codegen = is_codegen;
        symbol_table[symid]->is_noexport = is_noexport;
        symbol_table[symid]->is_volatile = is_volatile;
        symbol_table[symid]->is_atomic = is_atomic;
        symbol_table[symid]->is_data = 0;
    }
    /*If we are predeclaring something which was already defined, we don't want to suddenly make it incomplete again.*/
    /*A new symbol always inherits our is_predecl*/
    if(is_new_symbol){
        symbol_table[symid]->is_incomplete = is_predecl;
        //puts("Using New Symbol rule...");
    }
    /*A previously defined symbol which was incomplete inherits our is_predecl*/
    if(!is_new_symbol)
    if(symbol_table[symid]->is_incomplete){
        symbol_table[symid]->is_incomplete = is_predecl;
        //puts("Using Old Symbol rule...");
    }

    consume(); /*eat the identifier.*/
    if(peek()->data == TOK_OPERATOR)
    if(streq(peek()->text, "="))
    {
        had_equals = 1;
        require(!is_predecl, "Predeclaration of global variable may not have a definition.");
        consume(); //eat the equals sign!
        parse_repeatedly_try_metaprogramming(); //allow parsehooks to generate the constant expression used by a global variable.
        if(type_should_be_assigned_float_literal(t)){
            //
            symbol_table[symid]->cdata = c_allocX(8);
            symbol_table[symid]->cdata_sz = 8;
            fval = parse_cexpr_double();
            if(t.basetype == BASE_F32){
                float t;
                t = fval;
                memcpy(symbol_table[symid]->cdata, &t, 4);
                symbol_table[symid]->cdata_sz = 4;
                goto end;
            }
            if(t.basetype == BASE_F64){
                double t;
                t = fval;
                memcpy(symbol_table[symid]->cdata, &t, 8);
                symbol_table[symid]->cdata_sz = 8;
                goto end;
            }
            parse_error("Internal error, failed to assign value to symbol cdata");
        }
        if(type_can_be_assigned_integer_literal(t))
        {
            symbol_table[symid]->cdata = c_allocX(8);
            symbol_table[symid]->cdata_sz = 8;
            cval = parse_cexpr_int();
            if(t.pointerlevel == 0){
                if(t.basetype == BASE_U8 || t.basetype == BASE_I8){
                    int8_t t;
                    t = cval;
                    memcpy(symbol_table[symid]->cdata, &t, 1);
                    symbol_table[symid]->cdata_sz = 1;
                    goto end;
                }
                if(t.basetype == BASE_U16 || t.basetype == BASE_I16){
                    int16_t t;
                    t = cval;
                    memcpy(symbol_table[symid]->cdata, &t, 2);
                    symbol_table[symid]->cdata_sz = 2;
                    goto end;
                }
                if(t.basetype == BASE_U32 || t.basetype == BASE_I32){
                    int32_t t;
                    t = cval;
                    memcpy(symbol_table[symid]->cdata, &t, 4);
                    symbol_table[symid]->cdata_sz = 4;
                    goto end;
                }
                if(t.basetype == BASE_U64 || t.basetype == BASE_I64){
                    int64_t t;
                    t = cval;
                    memcpy(
                        symbol_table[symid]->cdata, 
                        &t, 
                        8
                    );
                    symbol_table[symid]->cdata_sz = 8;
                    goto end;
                }
            }
            if(t.pointerlevel > 0){
                uint64_t t;
                t = cval;
                memcpy(symbol_table[symid]->cdata, &t, POINTER_SIZE);
                symbol_table[symid]->cdata_sz = POINTER_SIZE;
                goto end;
            }
            parse_error("Internal error, failed to assign value to symbol cdata");
        }
        parse_error("Cannot assign literal to type.");
    }
    
    /*No initial assignment is allowed.*/
    end:;
    if(had_equals)require(symbol_table[symid]->cdata != NULL, "Global Variable Initial value failed!");
    consume_semicolon("Global Variable declaration statement must end in a semicolon.");
    return;
}

static void parse_datastmt(){
    type t = {0};
    uint64_t is_pub = 0;
    uint64_t was_string = 0;
    uint64_t is_predecl = 0;
    uint64_t is_new_symbol = 0;
    uint64_t is_codegen = 0;
    uint64_t is_noexport = 0;
    uint64_t symid;
    require(peek()->data == TOK_KEYWORD, "data statement must begin with \"data\" ");
    require(ID_KEYW(peek()) == ID_KEYW_STRING("data"), "data statement must begin with \"data\" ");
    consume();
    set_is_codegen(0);
    data_qualifiers_top:;
    parse_repeatedly_try_metaprogramming(); //allow parsehooks to generate an entire data statement, immediately after `data` as well as before/after qualifiers.
    if(peek()->data == TOK_KEYWORD)
        if(ID_KEYW(peek()) == ID_KEYW_STRING("predecl")){
            consume();
            is_predecl = 1;
            goto data_qualifiers_top;
        }
    if(peek()->data == TOK_KEYWORD)
        if(ID_KEYW(peek()) == ID_KEYW_STRING("codegen")){
            consume();
            set_is_codegen(1);
            is_codegen = 1;
            goto data_qualifiers_top;
        }
    if(peek()->data == TOK_KEYWORD)
        if(ID_KEYW(peek()) == ID_KEYW_STRING("noexport")){
            consume();
            is_noexport = 1;
            goto data_qualifiers_top;
        }
    if(peek()->data == TOK_KEYWORD)
        if(ID_KEYW(peek()) == ID_KEYW_STRING("pub")){
            is_pub = 1;
            consume();
            goto data_qualifiers_top;
        }
    if(peek()->data == TOK_KEYWORD)
        if(ID_KEYW(peek()) == ID_KEYW_STRING("static")){
            require(!is_pub, "Cannot have public and static at the same time! They are opposites!");
            is_pub = 0;
            consume();
            goto data_qualifiers_top;
        }
    require(peek_is_typename() || peek()->data == TOK_KEYWORD, "data statement requires typename or keyword.");
    if(ID_KEYW(peek()) == ID_KEYW_STRING("string")){
        t.basetype = BASE_U8;
        consume();
        was_string = 1;
        t.is_lvalue = 0;
    } else {
        t = parse_type();
        t.is_lvalue = 0;
    }
    require(type_can_be_literal(t), "The data type supplied to this data statement cannot be a literal! Note that pointers are not valid for a data statement.");
    require(type_can_be_variable(t), "Invalid type for data statement.");
    t.pointerlevel = 1;
    parse_repeatedly_try_metaprogramming(); //allow a parsehook immediately after the type specifier of a data statement...
    require(peek()->data == TOK_IDENT, "Identifier required in data statement declaration");


    require(!is_builtin_name(peek()->text),"Hey, What are you tryna pull? Defining the builtins? As data? huh?");

    if(peek_ident_is_already_used_globally()){
        unsigned long i;
        is_new_symbol=0;
        int found = 0;
        /*Check if it is an incomplete compatible declaration.*/
        for(i = 0; i < nsymbols; i++){
            if(streq(symbol_table[i]->name, peek()->text)){
                found = 1;
                /*check for compatible declaration.*/
                if(is_predecl == 0)
                    require(
                        symbol_table[i]->is_incomplete != 0, 
                        "Attempted to redefine complete data statement."
                    );
                require(symbol_table[i]->is_volatile == 0,"data statement Predeclaration-definition mismatch (is_volatile)");
                require(symbol_table[i]->is_atomic == 0,"data statement Predeclaration-definition mismatch (is_atomic)");
                require(symbol_table[i]->is_codegen == is_codegen,"data statement Predeclaration-definition mismatch (is_codegen)");
                require(symbol_table[i]->is_noexport == is_noexport,"data statement Predeclaration-definition mismatch (is_noexport)");
                require(symbol_table[i]->is_pub == is_pub,"data statement Predeclaration-definition mismatch (is_pub)");
                require(symbol_table[i]->t.basetype == t.basetype, "data statement Predeclaration-definition mismatch (basetype)");
                require(symbol_table[i]->t.pointerlevel == t.pointerlevel, "data statement Predeclaration-definition mismatch (pointerlevel)");
                require(symbol_table[i]->t.arraylen == t.arraylen, "data statement Predeclaration-definition mismatch (arraylen)");
                require(symbol_table[i]->t.is_function == t.is_function, "data statement Predeclaration-definition mismatch (is_function)");
                require(symbol_table[i]->t.is_lvalue == t.is_lvalue, "data statement Predeclaration-definition mismatch (is_lvalue)");
                if(t.basetype == BASE_STRUCT){
                    require(symbol_table[i]->t.structid == t.structid, "data statement Predeclaration-definition mismatch");
                }
                if(t.basetype == BASE_FUNCTION){
                    require(symbol_table[i]->t.funcid == t.funcid, "data statement Predeclaration-definition mismatch (funcid)");
                }
                symid = i;
                break;
            }
        }
        if(found == 0)
            parse_error("Unknown conflicting identifier usage for data stmt name...");
    }else{
        /*
        require(
            !peek_ident_is_already_used_globally(),
            "Identifier for data statement is already in use."
        );
        */
        symid = nsymbols;
        is_new_symbol=1;
        ++nsymbols;
        //removed
        symbol_table = re_allocX(symbol_table, (nsymbols) * sizeof(symdecl*));
        symbol_table[symid] = malloc(sizeof(symdecl));

        symbol_table[symid][0] = symdecl_init();
        symbol_table[symid]->t = t;
        symbol_table[symid]->name = strdup(peek()->text);
        symbol_table[symid]->is_pub = is_pub;
        symbol_table[symid]->is_codegen = is_codegen;
        symbol_table[symid]->is_noexport = is_noexport;
        symbol_table[symid]->is_data = 1;
        symbol_table[symid]->cdata = 0;
        symbol_table[symid]->cdata_sz = 0;
    }


    /*If we are predeclaring something which was already defined, we don't want to suddenly make it incomplete again.*/
    /*A new symbol always inherits our is_predecl*/
    if(is_new_symbol){
        symbol_table[symid]->is_incomplete = is_predecl;
    }
    /*A previously defined symbol which was incomplete inherits our is_predecl*/
    if(!is_new_symbol)
    if(symbol_table[symid]->is_incomplete){
        symbol_table[symid]->is_incomplete = is_predecl;
    }
    consume(); /*eat the identifier.*/
    parse_repeatedly_try_metaprogramming(); //here is where we would auto-generate data statement data...

    /*Change t so that pointer level is now zero.*/
    t.pointerlevel = 0;
    if(is_predecl){
        require(
            peek_is_semic(), 
            "data statement Predeclaration must end in a semicolon."
        );
    }
    /*Now, we have to parse a comma-separated list */
    if(!is_predecl)
    if(was_string){
        require(
            peek()->data == TOK_STRING, 
            "Syntax is data string ident (expected string....) [data statement error]"
        );
        symbol_table[symid]->cdata = (uint8_t*)strdup(peek()->text);
        consume(); /*Eat the string.*/
        symbol_table[symid]->cdata_sz = strlen((char*)symbol_table[symid]->cdata) + 1; /*includes null terminator*/
        symbol_table[symid]->is_incomplete = 0; /*It's definitely complete now!*/
    }
    
    if(!is_predecl)
    if(!was_string)
    {
        t.pointerlevel = 0;
        double db;
        float ft;
        uint64_t ut;
        int64_t it;
        uint64_t i = 0;
        
        /*Must be parsed based specifically on what it was...*/
        if(
            t.basetype == BASE_U8 ||
            t.basetype == BASE_U16 ||
            t.basetype == BASE_U32 ||
            t.basetype == BASE_U64 ||
            t.basetype == BASE_I8 ||
            t.basetype == BASE_I16 ||
            t.basetype == BASE_I32 ||
            t.basetype == BASE_I64
        ){
            while(1){
                i++;
                parse_repeatedly_try_metaprogramming(); //allow individual elements of a data statement's data to be metaprogrammed.
                if(peek_is_semic()){
                    break;
                }
                //require(peek()->data == TOK_INT_CONST, "data statement with integer type requires unsigned integer literal.");
                symbol_table[symid]->cdata = re_allocX(symbol_table[symid]->cdata, type_getsz(t) * (i));
                
                symbol_table[symid]->cdata_sz = type_getsz(t) * (i);
                it = parse_cexpr_int(); /*Allow for constant expressions.*/
                ut = it;
                //printf("%lld",(long long)it);
                /*write the data to the array.*/
                if(t.basetype == BASE_U8){
                    uint8_t* arr = (uint8_t*)symbol_table[symid]->cdata;
                    arr[i-1] = ut;
                }
                if(t.basetype == BASE_U16){
                    uint16_t* arr = (uint16_t*)symbol_table[symid]->cdata;
                    arr[i-1] = ut;
                }
                if(t.basetype == BASE_U32){
                    uint32_t* arr = (uint32_t*)symbol_table[symid]->cdata;
                    arr[i-1] = ut;
                }
                if(t.basetype == BASE_U64){
                    uint64_t* arr = (uint64_t*)symbol_table[symid]->cdata;
                    arr[i-1] = ut;
                }
                if(t.basetype == BASE_I8){
                    int8_t* arr = (int8_t*)symbol_table[symid]->cdata;
                    arr[i-1] = it;
                }
                if(t.basetype == BASE_I16){
                    int16_t* arr = (int16_t*)symbol_table[symid]->cdata;
                    arr[i-1] = it;
                }
                if(t.basetype == BASE_I32){
                    int32_t* arr = (int32_t*)symbol_table[symid]->cdata;
                    arr[i-1] = it;
                }
                if(t.basetype == BASE_I64){
                    int64_t* arr = (int64_t*)symbol_table[symid]->cdata;
                    arr[i-1] = it;
                }
                if(peek_is_semic()){
                    break;
                }
                require(peek()->data == TOK_COMMA, "data statement expected semicolon or comma.");
                consume();
                continue;
            } //eof while
        } //eof integer types
        if(
            t.basetype == BASE_F32 ||
            t.basetype == BASE_F64
        ){
            while(1){
                i++;
                parse_repeatedly_try_metaprogramming(); //allow individual elements of a data statement's data to be metaprogrammed.
                if(peek_is_semic()){
                    break;
                }
                //removed
                symbol_table[symid]->cdata = re_allocX(symbol_table[symid]->cdata, type_getsz(t) * (i));
                
                symbol_table[symid]->cdata_sz = type_getsz(t) * (i);
                db = parse_cexpr_double();
                ft = db;
                if(t.basetype == BASE_F32){
                    float* arr = (float*)symbol_table[symid]->cdata;
                    arr[i-1] = ft;
                }
                if(t.basetype == BASE_F64){
                    double* arr = (double*)symbol_table[symid]->cdata;
                    arr[i-1] = db;
                }
                if(peek_is_semic()){
                    break;
                }
                require(peek()->data == TOK_COMMA, "data statement expected semicolon or comma.");
                consume();
                continue;
            }
        }
    }


    if(!is_predecl)
        require(symbol_table[symid]->cdata_sz > 0, "Zero-length data statement.");
    consume_semicolon("data statement must end in a semicolon.");
}


//parse_typedecl
static void parse_structdecl(){
    typedecl* me;
    require(peek()->data == TOK_KEYWORD, "Struct declaration must begin with keyword");
    require(ID_KEYW(peek()) == ID_KEYW_STRING("struct"),"Struct declaration must begin with 'struct' or 'class'");
    consume();
    require(peek()->data == TOK_IDENT, "Struct declaration without identifier");
    require(!peek_ident_is_already_used_globally(), "Struct declaration uses already-in-use identifier");

    type_table = re_allocX(type_table, (++ntypedecls) * sizeof(typedecl));
    me = type_table + ntypedecls-1;
    me[0] = typedecl_init();
    me->name = strdup(peek()->text);
    me->is_incomplete = 1;
    me->is_noexport = 0;
    me->is_union = 0;
    me->algn = 0;
    consume(); /*eat the identifier*/

    while(1){
        if(peek_match_keyw("end")){
            consume();
            break;
        }
        if(peek_match_keyw("noexport")){
            me->is_noexport = 1;
            consume();
            continue;
        }
        if(peek_match_keyw("union")){
            me->is_union = 1;
            consume();
            continue;
        }
        /*Parse a struct member. This may include a semicolon.*/
        while(peek()->data == TOK_SEMIC) {consume();continue;}
        if(peek()->data == TOK_INT_CONST){
            if(me->algn)
                parse_error("Duplicate alignment specifier.");
            me->algn = matou(peek()->text);
            consume();
            continue;
        }
        parse_struct_member(ntypedecls-1);
        while(peek()->data == TOK_SEMIC) {consume();continue;}
    }
    require(me[0].nmembers > 0, "Struct may not have zero members.");
    me->is_incomplete = 0;
    push_empty_metadata(); //HEHE

    return;
}

static inline void parse_struct_member(uint64_t sid){
    type t;
    uint64_t i;
    t = parse_type();
    require(type_can_be_variable(t), "Struct member is not of a type which can be a struct member!");

    if(t.arraylen == 0) t.is_lvalue = 1;
    parse_repeatedly_try_metaprogramming(); //after the type specifier of a struct member, allow a parsehook.
    
    require(peek()->data == TOK_IDENT, "Struct member must have an identifier name.");
    t.membername = strdup(peek()->text);
    if(t.basetype == BASE_STRUCT)
        if(t.pointerlevel == 0){
            
            require(
                t.structid != sid, 
                "A struct may not have itself as a member..."
            );
            require(
                type_table[t.structid].is_incomplete == 0, 
                "You may not declare a struct with an incomplete struct type as its member..."
            );
        }
    consume();
    if(type_table[sid].nmembers){
        for(i = 0; i < type_table[sid].nmembers; i++)
            if(streq(t.membername, type_table[sid].members[i].membername))
                parse_error("Struct member duplicate member declaration");
    }
    type_table[sid].members = re_allocX(type_table[sid].members, (++type_table[sid].nmembers)*sizeof(type));
    type_table[sid].members[type_table[sid].nmembers-1] = t;
    t.is_lvalue = 1;
    if(t.arraylen > 0) t.is_lvalue = 0;
    if(t.pointerlevel == 0) if(t.basetype == BASE_STRUCT) t.is_lvalue = 0; //struct member of struct is not lvalue.
    //consume_semicolon("Struct member declaration requires semicolon.");
}

void parse_fn(int is_method){
    type t = {0};
    type t_method_struct = {0};
    type t_temp = {0};
    symdecl s = {0};
    char* n;
    uint64_t is_pub = 0;
    uint64_t is_predecl = 0;
    uint64_t is_noexport = 0;
    uint64_t is_codegen = 0;
    uint64_t is_pure = 0;
    uint64_t is_inline = 0;
    uint64_t symid;
    uint64_t nargs = 0;
    uint64_t k;
    t.basetype = BASE_VOID; //made explicit...
    set_is_codegen(0);
    if(!is_method){
        require(peek()->data == TOK_KEYWORD, "fn statement must begin with \"fn\"");
        require(ID_KEYW(peek()) == ID_KEYW_STRING("fn"), "fn statement must begin with \"fn\"");
    }
    if(is_method){
        require(peek()->data == TOK_KEYWORD, "method statement must begin with \"method\"");
        require(ID_KEYW(peek()) == ID_KEYW_STRING("method"), "fn statement must begin with \"fn\"");
    }
    consume(); //Eat fn or method
    fn_qualifier_top:;
    parse_repeatedly_try_metaprogramming(); //Immediately after fn/method, allow a parsehook invocation...

    //optionally eat any qualifiers...
    if(peek()->data == TOK_KEYWORD)
        if(ID_KEYW(peek()) == ID_KEYW_STRING("predecl")){
            consume();
            is_predecl = 1;
            goto fn_qualifier_top;
        }
    if(peek()->data == TOK_KEYWORD)
        if(ID_KEYW(peek()) == ID_KEYW_STRING("codegen")){
            consume();
            is_codegen = 1;
            set_is_codegen(1);
            goto fn_qualifier_top;
        }
    if(peek()->data == TOK_KEYWORD)
        if(ID_KEYW(peek()) == ID_KEYW_STRING("noexport")){
            consume();
            is_noexport = 1;
            goto fn_qualifier_top;
        }
    if(peek()->data == TOK_KEYWORD)
        if(ID_KEYW(peek()) == ID_KEYW_STRING("pub")){
            consume();
            is_pub = 1;
            goto fn_qualifier_top;
        }
    if(peek()->data == TOK_KEYWORD)
        if(ID_KEYW(peek()) == ID_KEYW_STRING("static")){
            require(!is_pub, "Cannot have public and static, they are opposites!");
            is_pub = 0;
            consume();
            goto fn_qualifier_top;
        }
    if(peek()->data == TOK_KEYWORD)
        if(ID_KEYW(peek()) == ID_KEYW_STRING("inline")){
            require(!is_pub, "Cannot have public and inline.");
            is_inline = 1;
            consume();
            goto fn_qualifier_top;
        }
    if(peek()->data == TOK_KEYWORD)
        if(ID_KEYW(peek()) == ID_KEYW_STRING("pure")){
            is_pure = 1;
            consume();
            goto fn_qualifier_top;
        }

    if(is_method){
        require(peek_is_typename(), "method statement requires a typename- the struct it operates on");
        t_method_struct = parse_type();
        
        require(t_method_struct.pointerlevel == 0, "must be a struct name");
        require(t_method_struct.arraylen == 0, "must be a struct name");
        require(t_method_struct.basetype == BASE_STRUCT, "must be a struct name");
        require(t_method_struct.structid < ntypedecls, "Must be a valid struct");
        


        t_method_struct.pointerlevel = 1;
        /*it is indeed an lvalue thing.*/
        t_method_struct.is_lvalue = 1;

        parse_repeatedly_try_metaprogramming(); //allow a parsehook after the type specifier of a method.
        /*Optionally consume a colon the method mystruct:myfunction*/
        if(peek()->data == TOK_OPERATOR){
            if(streq(peek()->text,":")){
                consume();
            } else if(streq(peek()->text,".")){
                consume();
            }
        }
    }
    parse_repeatedly_try_metaprogramming(); //allow a parsehook after the : or . of a method.
    require(peek()->data == TOK_IDENT, "fn requires identifier. Syntax is not C-like.");
    require(!is_builtin_name(peek()->text),"Hey, What are you tryna pull? Defining the builtins?");

    n = strdup(peek()->text);
    if(streq(n, "ctor") || streq(n, "dtor")){
        //Force an update.
        oop_metadata[t_method_struct.structid].have_checked = 0;
    }
    if(is_method){ /*mangle method names.*/
        char* t_;
        /*mangle the name*/
        t_ = strcata("__method_", type_table[t_method_struct.structid].name);
        t_ = strcataf1(t_, "_____");
        n = strcatafb(t_,n);
    }
    consume(); /*eat the identifier*/
    //do not allow a parsehook here...
    require(peek()->data == TOK_OPAREN, "fn requires opening parentheses");
    consume();

    if(is_method){
        t_method_struct.membername = strdup("this");
        s.fargs[0] = c_allocX(sizeof(type));
        
        s.fargs[0][0] = t_method_struct;
        nargs = 1;
    }
    while(1){
        parse_repeatedly_try_metaprogramming(); //allow individual function prototype arguments to be metaprogrammed.
        if(peek()->data == TOK_CPAREN) break;
        /*there is an argument to parse, it has both a type and a name.*/
        t_temp = type_init();
        t_temp = parse_type();
        require(t_temp.arraylen == 0, "Cannot pass array to function.");
        if(t_temp.basetype == BASE_STRUCT && t_temp.pointerlevel == 0){
            t_temp.pointerlevel=1; //fn myfunc(mystruct x): turns into fn myfunc(mystruct* x):
        }
        if(t_temp.basetype == BASE_VOID)
            require(t_temp.pointerlevel > 0, "Cannot pass void into function.");
        
        parse_repeatedly_try_metaprogramming(); //allow the name of a function prototype variable to be metaprogrammed.
        require(peek()->data == TOK_IDENT, "fn argument requires identifier. Syntax is not C-like.");
        require(!peek_is_fname(), "fn argument cannot be named after a function.");
        require(!peek_is_typename(), "fn argument cannot be named after a type.");
        t_temp.membername = strdup(peek()->text);
        t_temp.is_lvalue = 1;
        require(nargs < MAX_FARGS, "fn has too many arguments.");
        /*validate that function arguments thus far are not by the same name.*/
        for(k = 0; k < nargs; k++){
            if(streq(s.fargs[k]->membername, t_temp.membername)){
                puts("Function Parsing Error:");
                puts("An argument is duplicated by name.");
                puts(t_temp.membername);
                puts("In function:");
                puts(n);
                parse_error("Syntax Error: Function Arg Duplicated");
            }
        }

        s.fargs[nargs] = c_allocX(sizeof(type));
        s.fargs[nargs][0] = t_temp;
        nargs++;
        consume(); /*Eat the identifier.*/
        if(peek()->data == TOK_CPAREN) break;
        require(peek()->data == TOK_COMMA, "fn argument list is comma separated.");
        consume(); //eat the comma

    }
    require(peek()->data == TOK_CPAREN, "fn requires closing parentheses");
    consume();

    s.nargs = nargs;


    if(peek()->data != TOK_SEMIC){
        require(peek()->data == TOK_OPERATOR, "fn requires either a semicolon (predeclaration), colon(void return), or ->type");
        if(streq(peek()->text,"->")){
            consume();
            parse_repeatedly_try_metaprogramming(); //allow parsehooks after ->
            t = parse_type();
            require(t.arraylen == 0, "Function return type cannot be an array.");
            if(t.basetype == BASE_STRUCT)
            if(t.pointerlevel == 0){
                //increase pointerlevel...
                t.pointerlevel = 1;
            }
            require(type_can_be_in_expression(t), 
                "Function return type must be a primitive non-void"
            );
            t.is_function = 1;
            if(peek()->data == TOK_SEMIC){
                /*the declaration has ended, right here.*/
                is_predecl = 1;
                goto after_thing1;
            }
            require(peek()->data == TOK_OPERATOR, "fn requires colon here");
            require(streq(peek()->text,":"), "fn requires colon here");
            consume();
            goto after_thing1;
        }
        if(streq(peek()->text,":")){
            //DO NOT allow parsehooks here... they could be confused with parsehooks that operate
            //inside the body of a function...
            t.basetype = BASE_VOID;
            t.pointerlevel = 0;
            t.arraylen = 0;
            t.is_function = 1;
            t.structid = 0;
            require(is_predecl == 0, "Cannot define in predeclaration.");
            consume();
            goto after_thing1;
        }
        parse_error("Unhandled symbol after parentheses in function declaration");
    }
    if(peek()->data == TOK_SEMIC){
        /*the declaration has ended, right here.*/
        is_predecl = 1;
    }
    if(peek()->data != TOK_SEMIC){
        if(is_predecl)
            parse_error("Confused about whether this is actually predeclared or not.");
    }
    after_thing1:;
    t.arraylen = 0;
    t.is_function = 1;
    s.is_incomplete = is_predecl;
    s.is_pub = is_pub;
    s.is_pure = is_pure;
    s.is_codegen = is_codegen;
    s.is_inline = is_inline;
    s.is_noexport = is_noexport;
    if(ident_is_already_used_globally(n)){
        unsigned long i;
        unsigned long j;
        int found = 0;
        /*Check if it is an incomplete compatible declaration.*/
        for(i = 0; i < nsymbols; i++){
            if(streq(symbol_table[i]->name, n)){
                found = 1;
                /*check for compatible declaration.*/
                if(is_predecl == 0)
                    require(
                        symbol_table[i]->is_incomplete != 0, 
                        "Attempted to redefine complete Function"
                    );
                require(symbol_table[i]->is_codegen == is_codegen,"fn Predeclaration-definition mismatch (is_codegen)");
                require(symbol_table[i]->is_noexport == is_noexport, "fn Predeclaration-definition mismatch (is_noexport)");
                require(symbol_table[i]->is_pub == is_pub,"fn Predeclaration-definition mismatch (is_pub)");
                require(symbol_table[i]->is_pure == is_pure,"fn Predeclaration-definition mismatch (is_pure)");
                require(symbol_table[i]->is_inline == is_inline,"fn Predeclaration-definition mismatch (is_inline)");
                require(symbol_table[i]->t.basetype == t.basetype, "fn Predeclaration-definition mismatch (basetype)");
                require(symbol_table[i]->t.pointerlevel == t.pointerlevel, "fn Predeclaration-definition mismatch (pointerlevel)");
                require(symbol_table[i]->t.arraylen == t.arraylen, "fn Predeclaration-definition mismatch (arraylen)");
                require(symbol_table[i]->t.is_function == t.is_function, "fn Predeclaration-definition mismatch (is_function)");
                require(symbol_table[i]->t.is_lvalue == t.is_lvalue, "fn Predeclaration-definition mismatch (is_lvalue)");
                require(
                    symbol_table[i]->nargs == nargs,
                    "fn Predeclaration-definition mismatch (nargs)"
                );
                for(j = 0; j < nargs; j++){
                    require(s.fargs[j]->basetype ==symbol_table[i]->fargs[j]->basetype, "Argument mismatch (basetype)");
                    require(s.fargs[j]->pointerlevel ==symbol_table[i]->fargs[j]->pointerlevel, "Argument mismatch (ptrlevel)");
                    require(s.fargs[j]->arraylen ==symbol_table[i]->fargs[j]->arraylen, "Argument mismatch (arraylen)");
                    require(s.fargs[j]->is_function ==symbol_table[i]->fargs[j]->is_function, "Argument mismatch (is_function)");
                    require(s.fargs[j]->funcid ==symbol_table[i]->fargs[j]->funcid, "Argument mismatch (funcid)");
                    require(
                        streq(
                            s.fargs[j]->membername,
                            symbol_table[i]->fargs[j]->membername
                        ),
                        "Argument name mismatch"
                    );
                }
                symid = i;
                { uint64_t j;
                for(j = 0; j < nargs; j++)
                    {
                        if(s.fargs[j]->membername)
                            free(s.fargs[j]->membername);
                        free(s.fargs[j]);
                        s.fargs[j] = NULL;
                    }

                }
                free(n);
                break;
            }
        }
        if(found == 0)
        parse_error("Unknown conflicting identifier usage for function name...");
    }else{
        symid = nsymbols;
        ++nsymbols;
        //Removed.
        symbol_table = re_allocX(symbol_table, (nsymbols) * sizeof(symdecl));
        
        t.funcid = symid;
        s.t = t;
        s.nargs = nargs;
        s.name = n;
        symbol_table[symid] = malloc(sizeof(symdecl));
        symbol_table[symid][0] = s;
        if(s.is_codegen)
        if(strprefix("parsehook_", n))
        if(!strprefix("parsehook_expr_", n)){
            //check for validity as a parsehook...
            require(s.t.is_function != 0, "parsehook must be a function.");
            //require(s.is_codegen == 1, "parsehook must be is_codegen.");
            //require(s->is_incomplete == 0, "parsehook definition must be completed.");
            //require(s.fbody != NULL, "parsehook function body must not be null.");
            require(s.t.basetype == BASE_VOID, "parsehook must return nothing!");
            require(s.t.pointerlevel == 0 , "parsehook must return nothing, not even a pointer to nothing!");
            require(s.nargs == 0, "parsehook must take zero arguments. That's how I call it.");
            uint64_t rada = nparsehooks;
            parsehook_table = re_allocX(parsehook_table, (++nparsehooks) * sizeof(uint64_t));
            parsehook_table[rada] = symid;
        }
    }

    t.funcid = symid;
    if(is_predecl)
        consume_semicolon("Function predeclaration requires semicolon");
    if(!is_predecl){
        symbol_table[symid]->is_incomplete = 1; /*important for codegen functions...*/
        active_function = symid;
        parse_fbody();
        active_function = -1;
        symbol_table[symid]->is_incomplete = 0;
    }
}

void parse_method(){parse_fn(1);}


int peek_is_operator(){
    if(peek() == NULL) return 0;
    if(peek()->data == TOK_OPERATOR) return 1;
    return 0;
}

int peek_is_semic(){
    if(peek() == NULL) return 0;
    if(peek()->data == TOK_SEMIC) return 1;
    return 0;
}

strll* consume_semicolon(char* msg){
    require(peek() != NULL, msg);
    require(peek()->data == TOK_SEMIC, msg);
    return consume();
}

static stmt* parser_push_statement(){
    stmt* me;
    scopestack[nscopes-1]->stmts = re_allocX(
        scopestack[nscopes-1]->stmts, 
        (++scopestack[nscopes-1]->nstmts) * sizeof(stmt)
    );
    
    me = ((stmt*)scopestack[nscopes-1]->stmts)
                + scopestack[nscopes-1]->nstmts-1;
    *me =  stmt_init();
    me->whereami = scopestack[nscopes-1]; /*The scope to look in!*/
    me->filename = peek()->filename;
    if(me->filename == NULL)
        me->filename = ("<none>");
    me->linenum = peek()->linenum;
    me->colnum = peek()->colnum;
    me->kind = STMT_BAD; /*In case it's forgotten to set it.*/
    return me;
}
stmt* parser_push_statement_nop(){
    stmt* me;
    scopestack[nscopes-1]->stmts = re_allocX(
        scopestack[nscopes-1]->stmts, 
        (++scopestack[nscopes-1]->nstmts) * sizeof(stmt)
    );
    
    me = ((stmt*)scopestack[nscopes-1]->stmts)
                + scopestack[nscopes-1]->nstmts-1;
    *me =  stmt_init();
    me->whereami = scopestack[nscopes-1]; /*The scope to look in!*/
    me->filename = ("<none>");
    me->linenum = 0;
    me->colnum = 0;
    me->kind = STMT_NOP; /*In case it's forgotten to set it.*/
    return me;
}
void scope_insert_nops(scope* me, unsigned pos, unsigned n){

    unsigned i;
    unsigned nstmts_before;
    if(n == 0) return;
    if(pos >= me->nstmts){
        scopestack_push(me);
        for(i = 0; i < n;i++){
            parser_push_statement_nop();
        }
        scopestack_pop(me);
        return;
    }
    //We have to resize the buffer a
    nstmts_before = me->nstmts;
    me->stmts = re_allocX(
            me->stmts, 
            (me->nstmts + n) * sizeof(stmt)
    );
    me->nstmts = me->nstmts + n;
    /*
    printf("\n~~~~~~~~~~~~~~~~~~\nSCOPE EXPANSION EVENT\n");
    printf("\npos: %u\nn: %u",pos,n);
    printf("\nnstmts_before: %u\nsizeof(stmt): %u",nstmts_before, (unsigned)sizeof(stmt));
    printf("\ncopying to %u from %u, %u slots",pos+n, pos, nstmts_before - pos);
    fflush(stdout);
    */
    memmove(
        ((stmt*)me->stmts)+pos+n,
        ((stmt*)me->stmts)+pos,
        sizeof(stmt) * (nstmts_before - (pos)) //this doesn't depend on n? RIGHT. It's just the difference...
    ); 

    /*
    10 - 4 = 6
            p n=2
            V - -
    0 1 2 3 4 5 6 7 8 9 10
    0 1 2 3 4 5 6 7 8 9 10 fre fre (extend)
    0 1 2 3 4 5 4 5 6 7 8  9   10  (memmove)
    0 1 2 3 n n 4 5 6 7 8  9   10  (write nops)
    */
    for(i = 0; i < n; i++){
        ((stmt*)(me->stmts))[pos+i] = stmt_init();
        ((stmt*)(me->stmts))[pos+i].kind = STMT_NOP;
        ((stmt*)(me->stmts))[pos+i].whereami = me;
        ((stmt*)(me->stmts))[pos+i].filename = ("<none>");
        ((stmt*)(me->stmts))[pos+i].linenum = 0;
        ((stmt*)(me->stmts))[pos+i].colnum = 0;
    }
}


void parse_continue_break(){
    //HUGE NOTE: we cannot save the statement pointer here, because we are using re_allocX!
    stmt* me;
    require(peek()->data == TOK_KEYWORD, "continue/break both require a keyword!");
    require(nloops != 0, "continue/break must be inside of a loop.");
    require(nscopes != 0, "continue/break must be inside of a scope.");
    //there is at least one loop, and we are in it.
    if(peek_match_keyw("continue")){
        consume();
        me = parser_push_statement();
        me->kind = STMT_CONTINUE;
        me->whereami = scopestack[nscopes-1]; /*The scope to look in!*/
        return;
    }
    if(peek_match_keyw("break")){
        consume();
        me = parser_push_statement();
        me->kind = STMT_BREAK;
        me->whereami = scopestack[nscopes-1]; /*The scope to look in!*/
        return;
    }
    parse_error("Internal error: parse_continue_break failed");
}


/*
    EXPRESSION PARSING
*/

//used for the primitives...
#define EXPR_PARSE_BOILERPLATE\
    *targ = c_allocX(sizeof(expr_node));\
    **targ = f;


/*
    parse a non-function symbol.
*/
void expr_parse_ident(expr_node** targ){
    expr_node f = {0};
    require(peek()->data ==  TOK_IDENT, "expr_parse_ident needs an identifier");
    require(!peek_is_typename(), "expr_parse_ident expected non-typename");
    f.kind = EXPR_SYM; /*unidentified symbol*/
    f.is_function = 0;
    f.symname = strdup(peek()->text);
    consume();
    if(
        peek()->data == TOK_OPAREN
    ){
        parse_error("ERROR! This identifer IS NOT a function name!");
    }
    EXPR_PARSE_BOILERPLATE
    return;
}

void expr_parse_fcall(expr_node** targ){
    uint64_t required_arguments;
    int found_function = 0;
    unsigned long i;
    expr_node f = {0};
    require(peek()->data ==  TOK_IDENT, "expr_parse_fcall needs an identifier");
    f.kind = EXPR_FCALL; /*unidentified symbol*/
    f.is_function = 1;
    f.symname = strdup(peek()->text);

    for( i = 0; i < nsymbols; i++){
        if(streq(f.symname, symbol_table[i]->name)){
            require(
                symbol_table[i]->t.is_function != 0, 
                "expr_parse_fcall: not a function"
            );
            require(
                symbol_table[i]->is_codegen == symbol_table[active_function]->is_codegen ,
                "expr_parse_fcall: is_codegen mismatch."
            );
            found_function = 1;
            f.symid = i;
            break;
        }
    }
    require(found_function != 0, "expr_parse_fcall could not find referenced function");
    required_arguments = symbol_table[f.symid]->nargs;
    consume(); /*eat the identifier*/
    require(peek()->data == TOK_OPAREN, "expr_parse_fcall requires opening parentheses");
    consume();

    /*
        Parse a list of arguments.
    */
    for( i = 0; i < required_arguments; i++){
        parse_expr(f.subnodes + i);
        if(i != required_arguments-1){
            require(peek()->data == TOK_COMMA, "Function call arguments must be comma separated");
            consume(); /*eat comma*/
        }
    }
    
    require(peek()->data == TOK_CPAREN, "expr_parse_fcall requires closing parentheses");
    consume();
    EXPR_PARSE_BOILERPLATE
}

void expr_parse_builtin_call(expr_node** targ){
    uint64_t required_arguments = 0;
    unsigned long i = 0;
    expr_node f = {0};
    require(peek()->data ==  TOK_IDENT, "expr_parse_builtin_call needs an identifier");
    require(is_builtin_name(peek()->text), "That's not a builtin call!");

    if(symbol_table[active_function]->is_codegen == 0)
        parse_error("Non-codegen functions may not invoke __builtin functions.");
    f.kind = EXPR_BUILTIN_CALL;
    f.symname = strdup(peek()->text);
    required_arguments =  get_builtin_nargs(f.symname);
    consume();
    require(peek()->data == TOK_OPAREN, "expr_parse_builtin_call needs opening parentheses!");
    consume();

    /*
        Parse a list of arguments.
    */
    for( i = 0; i < required_arguments; i++){
        parse_expr(f.subnodes + i);
        if(i != required_arguments-1){
            require(peek()->data == TOK_COMMA, "Function call arguments must be comma separated");
            consume(); /*eat comma*/
        }
    }



    require(peek()->data == TOK_CPAREN, "expr_parse_builtin_call needs closing parentheses!");
    consume();
    EXPR_PARSE_BOILERPLATE
}

void expr_parse_intlit(expr_node** targ){
    expr_node f = {0};
    require(peek()->data == TOK_INT_CONST, "expr_parse_intlit needs an integer literal");
    f.kind = EXPR_INTLIT;
    f.is_function = 0;
    f.idata = matou(peek()->text);
    consume();
    EXPR_PARSE_BOILERPLATE
}

void expr_parse_floatlit(expr_node** targ){
    expr_node f = {0};
    require(peek()->data == TOK_FLOAT_CONST, "expr_parse_floatlit needs a float literal");
    f.kind = EXPR_FLOATLIT;
    f.is_function = 0;
    f.fdata = matof(peek()->text);
    consume();
    EXPR_PARSE_BOILERPLATE
}

void expr_parse_stringlit(expr_node** targ){
    expr_node f = {0};
    //require(peek()->data == TOK_STRING, "expr_parse_stringlit needs a string literal");
    //f.symid = parse_stringliteral();
    f.symname = strdup(peek()->text);
    consume();
    f.kind = EXPR_STRINGLIT;
    EXPR_PARSE_BOILERPLATE
}

void expr_parse_sizeof(expr_node** targ){
    expr_node f = {0};
    f.kind = EXPR_SIZEOF;
    require(peek_match_keyw("sizeof"),"expr_parse_sizeof requires the keyword sizeof");
    consume();
    require(peek()->data == TOK_OPAREN, "expr_parse_sizeof requires opening parentheses");
    consume();
    parse_repeatedly_try_metaprogramming(); //allow invoking metaprogramming inside of sizeof...
    f.type_to_get_size_of = parse_type();
    //write idata.
    f.idata = type_getsz(f.type_to_get_size_of);
    require(peek()->data == TOK_CPAREN, "expr_parse_sizeof requires closing parentheses");
    consume();
    EXPR_PARSE_BOILERPLATE
}

void expr_parse_paren(expr_node** targ){
    require(peek()->data == TOK_OPAREN, "expr_parse_paren requires opening parentheses");
    consume();
    //If the user tried a C-style cast, tell them it's invalid
    
    if(peek_is_typename()){
        puts("HEY! BUDDY! THIS ISN'T C!");
        puts("All casts must be explicitly declared `cast(type)`");
        puts("cast(myType*****)(3+5)");
        parse_error("User believes he is writing C...");
    }
    parse_expr(targ); //we dont need to introduce a secret level of separation...
    require(peek()->data == TOK_CPAREN, "expr_parse_paren requires closing parentheses");
    consume();
}

void expr_parse_constexpri(expr_node** targ){
    expr_node f = {0};
    f.kind = EXPR_CONSTEXPR_INT;
    require(peek_match_keyw("constexpri"),"expr_parse_constexpri requires the keyword sizeof");
    consume();
    require(peek()->data == TOK_OPAREN, "expr_parse_constexpri requires opening parentheses");
    consume();

    f.idata = parse_cexpr_int();
    
    require(peek()->data == TOK_CPAREN, "expr_parse_constexpri requires closing parentheses");
    consume();
    
    
    EXPR_PARSE_BOILERPLATE
}

void expr_parse_constexprf(expr_node** targ){
    expr_node f = {0};
    f.kind = EXPR_CONSTEXPR_FLOAT;
    require(peek_match_keyw("constexprf"),"expr_parse_constexprf requires the keyword sizeof");
    consume();
    require(peek()->data == TOK_OPAREN, "expr_parse_constexprf requires opening parentheses");
    consume();
    f.fdata = parse_cexpr_double();
    
    require(peek()->data == TOK_CPAREN, "expr_parse_constexprf requires closing parentheses");
    consume();
    
    EXPR_PARSE_BOILERPLATE
}

//getfnptr(fname)
void expr_parse_getfnptr(expr_node** targ){
    expr_node f = {0};
    f.kind = EXPR_GETFNPTR;
    int found_function = 0;
    uint64_t i;
    require(peek_match_keyw("getfnptr"), "expr_parse_getfnptr requires the keyword getfnptr");
    consume();
    require(peek()->data == TOK_OPAREN, "expr_parse_getfnptrf requires opening parentheses");
    consume();
    parse_repeatedly_try_metaprogramming(); //allow parsehooks inside of getfnptr
    require(peek_is_fname(), "getfnptr requires a function pointer!");
        f.symname = strdup(peek()->text);
        for( i = 0; i < nsymbols; i++){
            if(streq(f.symname, symbol_table[i]->name)){
                require(
                    symbol_table[i]->t.is_function != 0, 
                    "expr_parse_getfnptr: not a function"
                );
                require(
                    symbol_table[i]->is_codegen == symbol_table[active_function]->is_codegen ,
                    "expr_parse_getfnptr: is_codegen mismatch."
                );
                require(
                    symbol_table[i]->is_inline == 0,
                    "expr_parse_getfnptr: cannot get pointer to inline function."
                );
                found_function = 1;
                f.symid = i;
                break;
            }
        }
        require(found_function, "Did not find function to get pointer to!");
    consume();
    require(peek()->data == TOK_CPAREN, "expr_parse_getfnptr requires closing parentheses");
    consume();
    EXPR_PARSE_BOILERPLATE
}

void expr_parse_getglobalptr(expr_node** targ){
    expr_node f = {0};
    f.kind = EXPR_GETGLOBALPTR;
    int found_sym = 0;
    uint64_t i;
    require(peek_match_keyw("getglobalptr"), "expr_parse_getglobal requires the keyword getglobalptr");
    consume();
    require(peek()->data == TOK_OPAREN, "expr_parse_getglobalptr requires opening parentheses");
    consume();
    parse_repeatedly_try_metaprogramming(); //allow a parsehook to create the identifier for getglobalptr
    require(!peek_is_fname(), "expr_parse_getglobalptr requires a global variable name, not a function!");
    
    f.symname = strdup(peek()->text);
    for( i = 0; i < nsymbols; i++){
        if(streq(f.symname, symbol_table[i]->name)){
            require(
                symbol_table[i]->t.is_function == 0, 
                "expr_parse_getglobalptr: is a function"
            );
            require(
                symbol_table[i]->is_codegen == symbol_table[active_function]->is_codegen ,
                "expr_parse_getglobalptr: is_codegen mismatch."
            );
            found_sym = 1;
            f.symid = i;
            break;
        }
    }
    require(found_sym, "Did not find global symbol!");
    consume();
    require(peek()->data == TOK_CPAREN, "expr_parse_getglobalptr requires closing parentheses");
    consume();
    EXPR_PARSE_BOILERPLATE
}
//callfnptr[protofn](expr)(optional:expr)
void expr_parse_callfnptr(expr_node** targ){
    expr_node f = {0};
    int64_t i;
    uint64_t nargs = 0;
    int found_function = 0;
    f.kind = EXPR_CALLFNPTR;
    require(peek_match_keyw("callfnptr"), "expr_parse_callfnptr requires the keyword callfnptr");
    consume();
    require(peek()->data == TOK_OBRACK, "expr_parse_callfnptr requires opening bracket.");
    consume();
    parse_repeatedly_try_metaprogramming(); //allow parsehooks to be used in the prototype field.
    require(peek_is_fname(), "function name required: function matching prototype of function pointer.");

    f.is_function = 1;
    f.symname = strdup(peek()->text);

    for( i = 0; i < (int64_t)nsymbols; i++){
        if(streq(f.symname, symbol_table[i]->name)){
            require(
                symbol_table[i]->t.is_function != 0, 
                "expr_parse_callfnptr: prototype is not a function"
            );
            require(
                symbol_table[i]->is_codegen == symbol_table[active_function]->is_codegen,
                "expr_parse_callfnptr: prototype function has mismatch on is_codegen."
            );
            nargs = symbol_table[i]->nargs;
            require(nargs < (MAX_FARGS-1), "expr_parse_callfnptr: prototype function has too many arguments (MAX_FARGS-1). Not allowed!");
            found_function = 1;
            f.symid = i;
            f.fnptr_nargs = nargs;
            break;
        }
    }
    require(found_function != 0, "expr_parse_callfnptr could not find referenced prototype function");
    consume(); /*eat the prototype name*/

    
    require(peek()->data == TOK_CBRACK, "expr_parse_callfnptr requires closing bracket.");
    consume();
    require(peek()->data == TOK_OPAREN, "expr_parse_callfnptr requires opening parentheses");
    consume();
    //function pointer.
    parse_expr(f.subnodes);
    require(peek()->data == TOK_CPAREN, "expr_parse_callfnptr requires closing parentheses");
    consume();

    if(nargs){
        require(peek()->data == TOK_OPAREN, "expr_parse_callfnptr requires opening parentheses");
        consume();
        for(i = 0; i < (int64_t)nargs; i++){
            parse_expr(f.subnodes+1+i);
            if(i < ((int64_t)nargs-1) ){
                require(peek()->data == TOK_COMMA, "expr_parse_callfnptr: expected comma (need more arguments).");
                consume();
            }
        }
        require(peek()->data == TOK_CPAREN, "expr_parse_callfnptr requires closing parentheses");
        consume();
    } else {
        require(peek()->data == TOK_OPAREN, "expr_parse_callfnptr requires opening parentheses");
        consume();
        require(peek()->data == TOK_CPAREN, "expr_parse_callfnptr requires closing parentheses");
        consume();
    }
    EXPR_PARSE_BOILERPLATE
}

/*the terminal thing- a literal, identifier, function call, or sizeof*/
void expr_parse_terminal(expr_node** targ){
    if(peek_match_keyw("sizeof"))
    {
        expr_parse_sizeof(targ);
        return;
    }
    if(peek_match_keyw("constexpri")){
        expr_parse_constexpri(targ);
        return;
    }
    if(peek_match_keyw("constexprf")){
        expr_parse_constexprf(targ);
        return;
    }
    if(peek_match_keyw("getfnptr")){
        expr_parse_getfnptr(targ);
        return;
    }
    if(peek_match_keyw("callfnptr")){
        expr_parse_callfnptr(targ);
        return;
    }
    if(peek_match_keyw("getglobalptr")){
        expr_parse_getglobalptr(targ);
        return;
    }
    if(peek()->data == TOK_FLOAT_CONST){
        expr_parse_floatlit(targ);
        return;
    }	
    if(peek()->data == TOK_INT_CONST){
        expr_parse_intlit(targ);
        return;
    }
    /*
        It must BOTH be an fname, AND be followed by a parentheses...
    */
    if(
        peek_is_fname() &&
        peek()->right &&
        peek()->right->data == TOK_OPAREN
    ){
        expr_parse_fcall(targ);
        return;
    }
    if(peek()->data == TOK_IDENT)
        if(is_builtin_name(peek()->text)){
            expr_parse_builtin_call(targ);
            return;
        }
    if(peek()->data == TOK_IDENT){
        //emit a helpful eror
        
        expr_parse_ident(targ);
        return;
    }
    if(peek()->data == TOK_OPAREN){
        expr_parse_paren(targ);
        return;
    }

    
    if(peek()->data == TOK_STRING){
        expr_parse_stringlit(targ);
        return;
    }
    /*
        WHAT?!?! I ACTUALLY IMPLEMENTED THIS?!?!?!?!?!?!?
        NANI?!?!?!?!?!
    */
    if(peek()->data == TOK_OPERATOR){
        if(streq(peek()->text, "@")){
            char* t;
            uint64_t i;
            uint64_t id;
            int found = 0;
            consume();
            require(peek()->data == TOK_IDENT, "identifier required: parsehook_expr_XXXXX where XXXXX is written here.");
            //require(peek()->text != NULL, "Internal error, identifier does not have text?");
            t = strdup(peek()->text);
            t = strcataf2("parsehook_expr_",t);
            for(i = 0; i < nsymbols; i++){
                if(streq(symbol_table[i]->name, t)){
                    id = i;
                    found = 1;
                    break;
                }
            }
            require(found != 0, "Could not find parsehook_expr_ function");
            require(symbol_table[id]->t.is_function != 0, "parsehook_expr_ must be a function.");
            require(symbol_table[id]->is_codegen == 1, "parsehook_expr_ must be is_codegen.");
            require(symbol_table[id]->is_incomplete == 0, "parsehook_expr_ definition must be completed.");
            require(symbol_table[id]->fbody != NULL, "parsehook_expr_ function body must not be null.");
            require(symbol_table[id]->t.basetype == BASE_VOID, "parsehook_expr_ must return nothing!");
            require(symbol_table[id]->t.pointerlevel == 0 , "parsehook_expr_ must return nothing, not even a pointer to nothing!");
            require(symbol_table[id]->nargs == 1, "parsehook must take a char**. That's how I call it.");
            require(symbol_table[id]->fargs[0]->basetype == BASE_U8,"parsehook_expr_XXXXX requires a char** argument with void return");
            require(symbol_table[id]->fargs[0]->pointerlevel == 2,"parsehook_expr_XXXXX requires a char** argument with void return");
            ast_execute_expr_parsehook((symbol_table + id)[0],targ);
            require(targ[0] != NULL, "No expression came out of the parsehook being called?");
            return;
        }
        if(streq(peek()->text, "->")){
            puts("Found -> in expression... Huh? The syntax in Seabass is \".\" on pointers.");
            puts("Seabass does not require arrows on pointers...");
            parse_error("Unknown expression terminal.\n");
        }
    }
    puts("Welcome back to the dark side of the language... expr_parse_terminal failed...");
    if(peek()->data == TOK_CPAREN){
        puts("A closing parentheses was found where an expression terminal was expected.");
        puts("This usually happens because you either didn't finish an expression, or you");
        puts("omitted it entirely. Try looking at the location I send you to for an unfinished");
        puts("expression...");
        parse_error("Found a closing parentheses where an expression terminal was expected...");
    }
    if(peek() && peek()->data == TOK_SEMIC){
        puts("I was expecting an expression, not a semicolon!");
        puts("You probably did something like `return;` when the function");
        puts("has a return value.\nIn general, an expression was omitted.");
    }

    if(peek() && peek()->data == TOK_KEYWORD){
        puts("I seem to have found a keyword!");
        if(peek_is_typename())
            puts("It is a typename!");
        puts("Here's what I found, by the way:");
        puts(peek()->text);
        puts("It definitely doesn't belong here...");
    }else if(peek_is_typename()){
        puts("I found a typename that isn't a keyword...");
        puts("Here's what we've got:");
        puts(peek()->text);
    }
    puts("expr_parse_terminal failed.");
    if(peek()){
        if(peek()->text){
            puts("Here is the text of what I found:");
            puts(peek()->text);
        }
    }
    puts("Did you forget an 'end' somewhere? This typically happens if you forget to put an 'end' somewhere.");
    puts("That probably sounds pretty cryptic.");
    puts("Let me guess...");
    puts("1. Wrong number of function arguments. The function takes two, and you gave it one.");
    puts("2. You wrote part an expression (3+5+) and forgot to finish it.");
    puts("3. A statement (such as `return`) was expecting an expression, but you omitted it.");
    puts("'terminal' in this case, means that the expression parser was expecting either a");
    puts("literal of some sort, identifier, or a function call.");
    puts("But it didn't find anything it could match, and now here we are!");
    parse_error("Unknown expression terminal.\n");
}


void expr_parse_postfix(expr_node** targ){ //the valid postfixes are --, ++, [], .
    expr_node* f = NULL;
    expr_node* before_f = NULL;
    expr_parse_terminal(&f);
    unsigned long i;
    char* name;

    while(1){
        if(peek()->data == TOK_OBRACK){
            consume();
            before_f = c_allocX(sizeof(expr_node));
            parse_expr(before_f->subnodes+1); //subnodes[1] is the index expression...
            before_f->kind = EXPR_INDEX;
            before_f->subnodes[0] = f; //subnodes[0] is the thing to index...
            f = before_f;
            before_f = NULL;
            require(peek()->data == TOK_CBRACK, "Expected closing square bracket ]");
            consume();
            continue;
        }
        if(peek_is_operator()){
            if(streq(peek()->text, "++")){
                consume();
                before_f = c_allocX(sizeof(expr_node));
                before_f->kind = EXPR_POST_INCR;
                before_f->subnodes[0] = f;
                f = before_f;
                before_f = NULL;
                continue;
            }
            if(streq(peek()->text, "--")){
                consume();
                before_f = c_allocX(sizeof(expr_node));
                before_f->kind = EXPR_POST_DECR;
                before_f->subnodes[0] = f;
                f = before_f;
                before_f = NULL;
                continue;
            }
            if(streq(peek()->text,".")){
                consume();
                parse_repeatedly_try_metaprogramming(); //allow parsehooks to be invoked as .@myparsehook
                //TODO: Devise a scheme to allow calling of methods with .
                
                /*
                    .ident(
                   
                    means we are calling a method! Check for this case....
                    
                    )
                */
                before_f = c_allocX(sizeof(expr_node));
                require(peek()->data == TOK_IDENT, "Member access/method invocation syntax requires an identifier.");

                if(peek()->right)
                if(peek()->right->data == TOK_OPAREN){
                    before_f->kind = EXPR_METHOD;
                    f->is_function = 1;
                    before_f->subnodes[0] = f; /*A method is (secretly) passing "this" as the first argument.*/
                    /*gotta have a name*/
                    //require(peek()->data == TOK_IDENT, "Method syntax requires identifier.");
                    /*get the name*/
                    name = strdup(peek()->text);
                    before_f->method_name = name;
                    before_f->symname = NULL;
                    
                    consume(); //eat the identifier.
                    /*parse a comma-separated list of expressions...*/
                    //require(peek()->data == TOK_OPAREN, "Method invocation syntax requires opening parentheses.");
                    consume(); //eat the opening parentheses.
                    /*Parse a comma separated expression list.*/
                    for(i = 1; i < MAX_FARGS; i++){
                        if(peek()->data == TOK_CPAREN) break;
                        parse_expr(before_f->subnodes+i);
                        if(peek()->data == TOK_CPAREN) break;
                        if(i != (MAX_FARGS-1)){
                            /*the last function argument does not need a comma.*/
                            require(peek()->data == TOK_COMMA, "Method invocation syntax requires commas separating arguments.");
                            consume();
                        }
                    }
                    require(peek()->data == TOK_CPAREN, "Method invocation syntax requires closing parentheses.");
                    consume(); //eat the closing parentheses
                    f = before_f;
                    before_f = NULL;
                    continue;
                }
                
                before_f->kind = EXPR_MEMBER;
                before_f->subnodes[0] = f; /*expression evaluating to struct with 1 or more levels of indirection.*/
                require(peek()->data == TOK_IDENT, "Member access syntax requires an identifier.");
                before_f->symname = strdup(peek()->text);
                consume(); //eat the identifier.
                f = before_f;
                before_f = NULL;
                continue;
            }
            if(streq(peek()->text,".&")){
                consume();
                parse_repeatedly_try_metaprogramming(); //allow parsehooks like this: .&@myparsehooks
                before_f = c_allocX(sizeof(expr_node));
                before_f->kind = EXPR_MEMBERPTR;
                before_f->subnodes[0] = f; /*expression evaluating to struct with 1 or more levels of indirection.*/
                require(peek()->data == TOK_IDENT, "Member pointer access syntax requires an identifier.");
                before_f->symname = strdup(peek()->text);
                consume(); //eat the identifier.
                f = before_f;
                before_f = NULL;
                continue;
            }
            if(streq(peek()->text, ":")){
                consume(); //eat the colon
                parse_repeatedly_try_metaprogramming(); //allow parsehooks after the colon, myObject:@myParsehook
                before_f = c_allocX(sizeof(expr_node));
                before_f->kind = EXPR_METHOD;
                f->is_function = 1; //HUH?!?!
                before_f->subnodes[0] = f; /*A method is (secretly) passing "this" as the first argument.*/
                /*gotta have a name*/
                require(peek()->data == TOK_IDENT, "Method syntax requires identifier.");
                /*get the name*/
                name = strdup(peek()->text);
                before_f->method_name = name;
                before_f->symname = NULL;
                consume(); //eat the identifier.
                /*parse a comma-separated list of expressions...*/
                require(peek()->data == TOK_OPAREN, "Method invocation syntax requires opening parentheses.");
                consume(); //eat the opening parentheses.
                /*Parse a comma separated expression list.*/
                for(i = 1; i < MAX_FARGS; i++){
                    if(peek()->data == TOK_CPAREN) break;
                    parse_expr(before_f->subnodes+i);
                    if(peek()->data == TOK_CPAREN) break;
                    if(i != (MAX_FARGS-1)){
                        /*the last function argument does not need a comma.*/
                        require(peek()->data == TOK_COMMA, "Method invocation syntax requires commas separating arguments.");
                        consume();
                    }
                }
                require(peek()->data == TOK_CPAREN, "Method invocation syntax requires closing parentheses.");
                consume(); //eat the closing parentheses
                f = before_f;
                before_f = NULL;
                continue;
            }
            if(streq(peek()->text, "->")){
                parse_error("The Arrow (->) is not valid for member access in Seabass, use the Dot (.)");
            }
        }
        break;
    }
    *targ = f;
    return;
}

void expr_parse_prefix(expr_node** targ){
    //the valid prefixes are --, ++, !, ~, -, and cast (explicit or C-style)
    expr_node* f = NULL;
    if(peek_is_operator()){
        if(streq(peek()->text, "!")){
            consume();
            f = c_allocX(sizeof(expr_node));
            f->kind = EXPR_NOT;
            expr_parse_prefix(f->subnodes+0);
            *targ = f;
            return;
        }
        if(streq(peek()->text, "~")){
                    consume();
            f = c_allocX(sizeof(expr_node));
            f->kind = EXPR_COMPL;
            expr_parse_prefix(f->subnodes+0);
            *targ = f;
            return;
        }	
        if(streq(peek()->text, "-")){
            consume();
            f = c_allocX(sizeof(expr_node));
            f->kind = EXPR_NEG;
            expr_parse_prefix(f->subnodes+0);
            *targ = f;
            return;
        }		
        if(streq(peek()->text, "--")){
            consume();
            f = c_allocX(sizeof(expr_node));
            f->kind = EXPR_PRE_DECR;
            expr_parse_prefix(f->subnodes+0);
            *targ = f;
            return;
        }
        if(streq(peek()->text, "++")){
            consume();
            f = c_allocX(sizeof(expr_node));
            f->kind = EXPR_PRE_INCR;
            expr_parse_prefix(f->subnodes+0);
            *targ = f;
            return;
        }
    }
    if(peek_match_keyw("cast")){
        f = c_allocX(sizeof(expr_node));
        f->kind = EXPR_CAST;
        f->is_implied = 0;
        consume(); //eat 'cast'
        require(peek()->data == TOK_OPAREN, "cast requires opening parentheses.");
        consume(); //
        parse_repeatedly_try_metaprogramming(); //allow parsehooks inside of a cast...
        f->type_to_get_size_of = parse_type();
        require(peek()->data == TOK_CPAREN, "cast requires closing parentheses.");
        consume();
        expr_parse_prefix(f->subnodes+0);
        *targ = f;
        return;
    }
    //allow C-style casts...
    if(peek()->data == TOK_OPAREN)
    if(peek()->right != 0)
    if(peek()->right->text)
    if((peek()->right->data != TOK_STRING))
    if(string_is_typename(peek()->right->text)){
        f = c_allocX(sizeof(expr_node));
         f->kind = EXPR_CAST;
        f->is_implied = 0;
        consume(); //consume the parentheses...
        //parse_repeatedly_try_metaprogramming(); //DO NOT allow parsehooks inside of a C-style cast... they could be confused with expr parsehooks.
        f->type_to_get_size_of = parse_type();
        require(peek()->data == TOK_CPAREN, "C-style cast requires closing parentheses.");
        consume();
        expr_parse_prefix(f->subnodes+0);
        *targ = f;
        return;
    }
    /*
        The C-style cast...
    */
    expr_parse_postfix(targ);
    return;
}


void expr_parse_muldivmod(expr_node** targ){
    /*parse multiple, divide, and modulo*/
    expr_node* c;
    expr_node* a;
    expr_node* b;
    expr_parse_prefix(&a);
    while(1){
        if(peek()->data == TOK_OPERATOR){
            if(streq(peek()->text, "*")){
                consume();
                c = c_allocX(sizeof(expr_node));
                c->kind = EXPR_MUL;
                expr_parse_prefix(&b);
                c->subnodes[0] = a;
                c->subnodes[1] = b;
                //fold it in...
                a = c;
                c = NULL;
                b = NULL;
                continue;
            }
            if(streq(peek()->text, "/")){
                consume();
                c = c_allocX(sizeof(expr_node));
                c->kind = EXPR_DIV;
                expr_parse_prefix(&b);
                c->subnodes[0] = a;
                c->subnodes[1] = b;
                //fold it in...
                a = c;
                c = NULL;
                b = NULL;
                continue;
            }
            if(streq(peek()->text, "%")){
                consume();
                c = c_allocX(sizeof(expr_node));
                c->kind = EXPR_MOD;
                expr_parse_prefix(&b);
                c->subnodes[0] = a;
                c->subnodes[1] = b;
                //fold it in...
                a = c;
                c = NULL;
                b = NULL;
                continue;
            }
        }
        break;
    }
    *targ = a;
}
void expr_parse_addsub(expr_node** targ){
    /*parse multiple, divide, and modulo*/
    expr_node* c;
    expr_node* a;
    expr_node* b;
    expr_parse_muldivmod(&a);
    while(1){
        if(peek()->data == TOK_OPERATOR){
            if(streq(peek()->text, "+")){
                consume();
                c = c_allocX(sizeof(expr_node));
                c->kind = EXPR_ADD;
                expr_parse_muldivmod(&b);
                c->subnodes[0] = a;
                c->subnodes[1] = b;
                //fold it in...
                a = c;
                c = NULL;
                b = NULL;
                continue;
            }
            if(streq(peek()->text, "-")){
                consume();
                c = c_allocX(sizeof(expr_node));
                c->kind = EXPR_SUB;
                expr_parse_muldivmod(&b);
                c->subnodes[0] = a;
                c->subnodes[1] = b;
                //fold it in...
                a = c;
                c = NULL;
                b = NULL;
                continue;
            }
        }
        break;
    }
    *targ = a;
}

void expr_parse_bit(expr_node** targ){
    /*parse multiple, divide, and modulo*/
    expr_node* c;
    expr_node* a;
    expr_node* b;
    expr_parse_addsub(&a);
    while(1){
        if(peek()->data == TOK_OPERATOR){
            if(streq(peek()->text, "|")){
                consume();
                c = c_allocX(sizeof(expr_node));
                c->kind = EXPR_BITOR;
                expr_parse_addsub(&b);
                c->subnodes[0] = a;
                c->subnodes[1] = b;
                //fold it in...
                a = c;
                c = NULL;
                b = NULL;
                continue;
            }
            if(streq(peek()->text, "&")){
                consume();
                c = c_allocX(sizeof(expr_node));
                c->kind = EXPR_BITAND;
                expr_parse_addsub(&b);
                c->subnodes[0] = a;
                c->subnodes[1] = b;
                //fold it in...
                a = c;
                c = NULL;
                b = NULL;
                continue;
            }
            if(streq(peek()->text, "^")){
                consume();
                c = c_allocX(sizeof(expr_node));
                c->kind = EXPR_BITXOR;
                expr_parse_addsub(&b);
                c->subnodes[0] = a;
                c->subnodes[1] = b;
                //fold it in...
                a = c;
                c = NULL;
                b = NULL;
                continue;
            }
            if(streq(peek()->text, "<<")){
                consume();
                c = c_allocX(sizeof(expr_node));
                c->kind = EXPR_LSH;
                expr_parse_addsub(&b);
                c->subnodes[0] = a;
                c->subnodes[1] = b;
                //fold it in...
                a = c;
                c = NULL;
                b = NULL;
                continue;
            }
            if(streq(peek()->text, ">>")){
                consume();
                c = c_allocX(sizeof(expr_node));
                c->kind = EXPR_RSH;
                expr_parse_addsub(&b);
                c->subnodes[0] = a;
                c->subnodes[1] = b;
                //fold it in...
                a = c;
                c = NULL;
                b = NULL;
                continue;
            }
        }
        break;
    }
    *targ = a;
}


void expr_parse_compare(expr_node** targ){
    /*parse multiple, divide, and modulo*/
    expr_node* c;
    expr_node* a;
    expr_node* b;
    expr_parse_bit(&a);
    while(1){
        if(peek()->data == TOK_OPERATOR){
            if(
                streq(peek()->text, "==") ||
                streq(peek()->text, "===") ||
                streq(peek()->text, "eq")
            ){
                consume();
                c = c_allocX(sizeof(expr_node));
                c->kind = EXPR_EQ;
                expr_parse_bit(&b);
                c->subnodes[0] = a;
                c->subnodes[1] = b;
                //fold it in...
                a = c;
                c = NULL;
                b = NULL;
                continue;
            }
            if(
                streq(peek()->text, "streq")
            ){
                consume();
                c = c_allocX(sizeof(expr_node));
                c->kind = EXPR_STREQ;
                expr_parse_bit(&b);
                c->subnodes[0] = a;
                c->subnodes[1] = b;
                //fold it in...
                a = c;
                c = NULL;
                b = NULL;
                continue;
            }	
            if(
                streq(peek()->text, "strneq")
            ){
                consume();
                c = c_allocX(sizeof(expr_node));
                c->kind = EXPR_STRNEQ;
                expr_parse_bit(&b);
                c->subnodes[0] = a;
                c->subnodes[1] = b;
                //fold it in...
                a = c;
                c = NULL;
                b = NULL;
                continue;
            }
            if(
                streq(peek()->text, "!=") ||
                streq(peek()->text, "neq")
            ){
                consume();
                c = c_allocX(sizeof(expr_node));
                c->kind = EXPR_NEQ;
                expr_parse_bit(&b);
                c->subnodes[0] = a;
                c->subnodes[1] = b;
                //fold it in...
                a = c;
                c = NULL;
                b = NULL;
                continue;
            }
            if(streq(peek()->text, ">=")){
                consume();
                c = c_allocX(sizeof(expr_node));
                c->kind = EXPR_GTE;
                expr_parse_bit(&b);
                c->subnodes[0] = a;
                c->subnodes[1] = b;
                //fold it in...
                a = c;
                c = NULL;
                b = NULL;
                continue;
            }	
            if(streq(peek()->text, ">")){
                consume();
                c = c_allocX(sizeof(expr_node));
                c->kind = EXPR_GT;
                expr_parse_bit(&b);
                c->subnodes[0] = a;
                c->subnodes[1] = b;
                //fold it in...
                a = c;
                c = NULL;
                b = NULL;
                continue;
            }
            if(streq(peek()->text, "<=")){
                consume();
                c = c_allocX(sizeof(expr_node));
                c->kind = EXPR_LTE;
                expr_parse_bit(&b);
                c->subnodes[0] = a;
                c->subnodes[1] = b;
                //fold it in...
                a = c;
                c = NULL;
                b = NULL;
                continue;
            }
            if(streq(peek()->text, "<")){
                consume();
                c = c_allocX(sizeof(expr_node));
                c->kind = EXPR_LT;
                expr_parse_bit(&b);
                c->subnodes[0] = a;
                c->subnodes[1] = b;
                //fold it in...
                a = c;
                c = NULL;
                b = NULL;
                continue;
            }
        }
        break;
    }
    *targ = a;
}



void expr_parse_logbool(expr_node** targ){
    expr_node* c;
    expr_node* a;
    expr_node* b;
    expr_node* chain = NULL;
    expr_parse_compare(&a);
    while(1){
        if(peek()->data == TOK_OPERATOR){
            if(!chain){
                if(streq(peek()->text, "||")){
                    consume();
                    c = c_allocX(sizeof(expr_node));
                    c->kind = EXPR_LOGOR;
                    expr_parse_compare(&b);
                    c->subnodes[0] = a;
                    c->subnodes[1] = b;
                    chain = c;
                    a = chain;
                    continue;
                }
                if(streq(peek()->text, "&&")){
                    consume();
                    c = c_allocX(sizeof(expr_node));
                    c->kind = EXPR_LOGAND;
                    expr_parse_compare(&b);
                    c->subnodes[0] = a;
                    c->subnodes[1] = b;
                    chain = c;
                    a = chain;
                    continue;
                }
            } else {
                if(streq(peek()->text, "||")){
                    consume();
                    c = c_allocX(sizeof(expr_node));
                    c->kind = EXPR_LOGOR;
                    expr_parse_compare(&b);
                    c->subnodes[0] = a->subnodes[1];
                    c->subnodes[1] = b;
                    a->subnodes[1] = c;
                    a = a->subnodes[1];
                    continue;
                }
                if(streq(peek()->text, "&&")){
                    consume();
                    c = c_allocX(sizeof(expr_node));
                    c->kind = EXPR_LOGAND;
                    expr_parse_compare(&b);
                    c->subnodes[0] = a->subnodes[1];
                    c->subnodes[1] = b;
                    a->subnodes[1] = c;
                    a = a->subnodes[1];
                    continue;
                }
            }
        }
        break;
    }
    *targ = a;
    if(chain){
        *targ = chain;
    }
}



/*
void expr_parse_logbool(expr_node** targ){
    expr_node* c;
    expr_node* a;
    expr_node* b;
    expr_parse_compare(&a);
    {
        if(peek()->data == TOK_OPERATOR){
            if(streq(peek()->text, "||")){
                consume();
                c = c_allocX(sizeof(expr_node));
                c->kind = EXPR_LOGOR;
                expr_parse_logbool(&b);
                c->subnodes[0] = a;
                c->subnodes[1] = b;
                a = c;
            }
            if(streq(peek()->text, "&&")){
                consume();
                c = c_allocX(sizeof(expr_node));
                c->kind = EXPR_LOGAND;
                expr_parse_logbool(&b);
                c->subnodes[0] = a;
                c->subnodes[1] = b;
                a = c;
            }
        }
    }
    *targ = a;
}
*/


void expr_parse_assign(expr_node** targ){
    /*parse assignments*/
    expr_node* c;
    expr_node* a;
    expr_node* b;
    expr_parse_logbool(&a);
    //while(1){
    if(peek()->data == TOK_OPERATOR){
        if(streq(peek()->text, "=")){
            consume();
            c = c_allocX(sizeof(expr_node));
            c->kind = EXPR_ASSIGN;
            expr_parse_assign(&b);
            c->subnodes[0] = a;
            c->subnodes[1] = b;
            //fold it in...
            a = c;
            c = NULL;
            b = NULL;
        }
    }
//	}
    *targ = a;
}


void expr_parse_move(expr_node** targ){
    /*parse assignments*/
    expr_node* c;
    expr_node* a;
    expr_node* b;
    expr_parse_assign(&a);
    //while(1){
    if(peek()->data == TOK_OPERATOR){
        if(streq(peek()->text, ":=")){
            consume();
            c = c_allocX(sizeof(expr_node));
            c->kind = EXPR_MOVE;
            expr_parse_move(&b);
            c->subnodes[0] = a;
            c->subnodes[1] = b;
            //fold it in...
            a = c;
            c = NULL;
            b = NULL;
        }
    }
//	}
    *targ = a;
}


void parse_expr(expr_node** targ){
    expr_parse_move(targ);
    return;
}


void parse_expr_stmt(){
    stmt* me;
    me = parser_push_statement();
    me->kind = STMT_EXPR;
    me->nexpressions = 1;
    parse_expr((expr_node**)&me->expressions[0]);
    if(
        peek_match_keyw("end") || 
        peek_match_keyw("if") || 
        peek_match_keyw("elif") || 
        peek_match_keyw("else") ||
        peek_match_keyw("tail") ||
        peek_match_keyw("switch") ||
        peek_match_keyw("return") ||
        peek_match_keyw("asm") ||
        peek_match_keyw("continue") ||
        peek_match_keyw("break") ||
        peek_match_keyw("goto") ||
        peek_match_keyw("while") ||
        peek_match_keyw("for") ||
        peek_is_typename()
    ){
        //No possibility of ambiguity...
        return;
    }
    consume_semicolon(\
        "expression statement requires a semicolon."
        "\nThe place I was trying to find a semicolon was:"
    );
}

//declaring a local variable.
void parse_lvardecl(){
    symdecl s = {0};
    scope* cscope = NULL;

    require(nscopes > 0, "local variable declaration must be inside of a scope.");
    cscope = scopestack[nscopes - 1];
    //require(cscope != NULL, "Internal error. Cannot add local variable to NULL scope.");
    s.t = parse_type();
    parse_repeatedly_try_metaprogramming(); //allow parsehooks to define local variable names...
    require(type_can_be_variable(s.t), "cannot declare local variable of that type.");
    require(peek()->data == TOK_IDENT, "local variable requires identifier.");
    require(!is_builtin_name(peek()->text),"Hey, What are you tryna pull? Defining the builtins?");
    s.t.is_lvalue = 1;
    if(s.t.arraylen > 0) s.t.is_lvalue = 0;
    if(s.t.pointerlevel == 0){
        if(s.t.basetype == BASE_STRUCT){
            s.t.is_lvalue = 0;
            require(type_table[s.t.structid].is_incomplete == 0, "You may not declare a local variable using an incomplete struct type.");
        }
    }
    //TODO: check for invalid declarations... ?
    //TODO: Fix bug... you can actually shadow variables...
    require(!ident_forbidden_declaration_check(peek()->text), "Identifier is already in use for a local variable in the current scope!");

    s.name = strdup(peek()->text);
    
    /*
        Check to see if this thing is immediately followed by an equals sign...
    */
    if(peek()->right)
    if(peek()->right->data == TOK_OPERATOR)
    if(streq(peek()->right->text, "=")){
        //Duplicate this token to its right, allowing us to
        //simulate the effects of assignment...
        strll* pp;
        pp = c_allocX(sizeof(strll));
        pp->right = peek()->right;
        pp->data = peek()->data;
        pp->text = strdup(peek()->text);
        peek()->right = pp;
    }
    consume();
    cscope->syms = re_allocX(cscope->syms, (++cscope->nsyms) * sizeof(symdecl));
    cscope->syms[cscope->nsyms-1] = s;
    return;
}

void parse_elif();
void parse_else();
int parse_stmts_allow_else_chain();

void parse_if(){
    int endmode = 0;
    //TODO
    stmt* me;
    me = parser_push_statement();
    consume_keyword("if");
    me->kind = STMT_IF;
    me->nexpressions = 1;
    require(peek()->data == TOK_OPAREN, "if needs an opening parentheses"); consume();
    parse_expr((expr_node**)(me->expressions + 0) );
    require(peek()->data == TOK_CPAREN, "if needs a closing parentheses"); consume();

    me->myscope = c_allocX(sizeof(scope));
        scopestack_push(me->myscope);
            endmode = parse_stmts_allow_else_chain();
        scopestack_pop();
    if(endmode == 0){
        consume(); return; //eat the end!
    }
    if(endmode == 1){
        parse_else(); return;
    }
    if(endmode == 2){
        parse_elif(); return;
    }
    parse_error("Internal error: parse_if endmode fell through (should be impossible)");
    //TODO: parse_elif()/parse_else()
}

void parse_elif(){
    int endmode = 0;
    stmt* me;
    me = parser_push_statement();
    consume_keyword("elif");
    me->kind = STMT_ELIF;
    me->nexpressions = 1;
    require(peek()->data == TOK_OPAREN, "if needs an opening parentheses"); consume();
    parse_expr((expr_node**)(me->expressions + 0) );
    require(peek()->data == TOK_CPAREN, "if needs a closing parentheses"); consume();

    me->myscope = c_allocX(sizeof(scope));
        scopestack_push(me->myscope);
            endmode = parse_stmts_allow_else_chain();
        scopestack_pop();
    if(endmode == 0){
        consume(); return; //eat the end!
    }
    if(endmode == 1){
        parse_else(); return;
    }
    if(endmode == 2){
        parse_elif(); return;
    }
    parse_error("Internal error: parse_if endmode fell through (should be impossible)");
    //TODO: parse_elif()/parse_else()
}
void parse_else(){
    stmt* me;
    me = parser_push_statement();
    me->kind = STMT_ELSE;
    consume_keyword("else");
        me->myscope = c_allocX(sizeof(scope));
        scopestack_push(me->myscope);
            parse_stmts();
        scopestack_pop();
    return;
}

void parse_while(){
    stmt* me;
    me = parser_push_statement();
    consume_keyword("while");
    me->kind = STMT_WHILE;
    me->nexpressions = 1;

    require(peek()->data == TOK_OPAREN, "while needs an opening parentheses"); consume();
    parse_expr((expr_node**)(me->expressions + 0) );
    require(peek()->data == TOK_CPAREN, "while needs a closing parentheses"); consume();

    me->myscope = c_allocX(sizeof(scope));
        loopstack_push(me);
        scopestack_push(me->myscope);
            parse_stmts();
        scopestack_pop();
        loopstack_pop();
}

void parse_for(){
    stmt* me;
    me = parser_push_statement();
    consume_keyword("for");
    me->kind = STMT_FOR;
    me->nexpressions = 3;
    require(peek()->data == TOK_OPAREN, "for needs an opening parentheses"); 
    consume(); //eat the oparen
    //three expressions.
    parse_expr((expr_node**)(me->expressions + 0) );
    if(peek()->data != TOK_COMMA){
        if(peek()->data == TOK_SEMIC){
            //allow it!
        } else {
            parse_error("for syntax- expected comma or semicolon!");    
        }
    }
    //require(peek()->data == TOK_COMMA, "expected comma in for statement after initial expression."); 
    consume();
    parse_expr((expr_node**)(me->expressions + 1) );
    //require(peek()->data == TOK_COMMA, "expected comma in for statement after condition expression."); 
    if(peek()->data != TOK_COMMA){
        if(peek()->data == TOK_SEMIC){
            //allow it!
        } else {
            parse_error("for syntax- expected SECOND comma or semicolon!");    
        }
    }
    consume();
    parse_expr((expr_node**)(me->expressions + 2) );
    
    require(peek()->data == TOK_CPAREN, "for needs a closing parentheses"); 
    consume(); //eat the cparen

    me->myscope = c_allocX(sizeof(scope));
        loopstack_push(me);
        scopestack_push(me->myscope);
            parse_stmts();
        scopestack_pop();
        loopstack_pop();
}

void parse_return(){
    //TODO
    stmt* me;
    me = parser_push_statement();
    consume_keyword("return");
    me->kind = STMT_RETURN;
    if(
        symbol_table[active_function]->t.basetype == BASE_VOID &&
        symbol_table[active_function]->t.pointerlevel == 0
    ){
        me->nexpressions = 0;
        //do require a semicolon.
        if(
            peek_match_keyw("end") || 
            peek_match_keyw("if") || 
            peek_match_keyw("elif") || 
            peek_match_keyw("else") ||
            peek_match_keyw("tail") ||
            peek_match_keyw("switch") ||
            peek_match_keyw("return") ||
            peek_match_keyw("asm") ||
            peek_match_keyw("continue") ||
            peek_match_keyw("break") ||
            peek_match_keyw("goto") ||
            peek_match_keyw("while") ||
            peek_match_keyw("for") ||
            peek_is_typename()
        ){
            //No possibility of ambiguity...
            return;
        }
        consume_semicolon(
            "Return expected semicolon. This is a function/method with no return value.\n"
            "The Syntax for declaring a function with a return value is as follows:\n"
            "fn myFunction(int arg1, int arg2)->int:\n"
            "    return arg1 + arg2;\n"
            "end\n\n"
            "Notice how there's that part with an arrow, \"->int\" ?\n"
            "That tells the compiler what the return type of the function is.\n"
            "You can also predeclare functions with return types:\n"
            "fn predecl myFunction(int arg1, int arg2)->int;\n"
        );
        return;
    }
    me->nexpressions = 1;
    parse_expr((expr_node**)(me->expressions + 0) );
    if(
        peek_match_keyw("end") || 
        peek_match_keyw("if") || 
        peek_match_keyw("elif") || 
        peek_match_keyw("else") ||
        peek_match_keyw("tail") ||
        peek_match_keyw("switch") ||
        peek_match_keyw("return") ||
        peek_match_keyw("asm") ||
        peek_match_keyw("continue") ||
        peek_match_keyw("break") ||
        peek_match_keyw("goto") ||
        peek_match_keyw("while") ||
        peek_match_keyw("for") ||
        peek_is_typename()
    ){
        //No possibility of ambiguity...
        return;
    }
    consume_semicolon("return requires semicolon.");

}

void parse_tail(){
    //TODO
    stmt* me;
    int found = 0;
    me = parser_push_statement();
    consume_keyword("tail");
    parse_repeatedly_try_metaprogramming(); //allow parsehooks in the function name field of a tail statement.
    me->kind = STMT_TAIL;
    me->nexpressions = 0;
    require(peek_is_fname(), "tail requires a function name.");
    me->referenced_label_name = strdup(peek()->text);
    consume();
    //puts("WE FOUND THIS:");
    //puts(me->referenced_label_name);
    /*Check to see if the return type of the function is compatible with us.*/
    for(unsigned long i = 0; i < nsymbols; i++)
        if(symbol_table[i]->t.is_function)
            if(
                streq(
                    me->referenced_label_name, 
                    symbol_table[i]->name
                )
            ){
                found = 1;
                /*
                    check the number of arguments, as well as the return type.
                */
                require(
                    symbol_table[i]->nargs == symbol_table[active_function]->nargs, 
                    "tail requires a function taking the same number of arguments and an identical return type to the function it's being called from."
                );
                require(symbol_table[i]->t.basetype == symbol_table[active_function]->t.basetype, "Tail function type mismatch (basetype)");
                if(symbol_table[i]->t.basetype == BASE_STRUCT)
                    require(symbol_table[i]->t.structid == symbol_table[active_function]->t.structid, "Tail function type mismatch (structid)");
                require(symbol_table[i]->t.pointerlevel == symbol_table[active_function]->t.pointerlevel, "Tail function type mismatch (pointerlevel)");
                require(symbol_table[i]->is_codegen == symbol_table[active_function]->is_codegen, "Tail function type mismatch (is_codegen)");
                me->symid = i;
                break;
            }

    require(found, "tail with an invalid function name.");
    return;
}

void parse_fbody(){
    symbol_table[active_function]->fbody = c_allocX(sizeof(scope));
    ((scope*)symbol_table[active_function]->fbody)->is_fbody = 1;
    scopestack_push(symbol_table[active_function]->fbody);
        parse_stmts();
    scopestack_pop();
    if(nscopes > 0 || nloops > 0){
        parse_error("INTERNAL PARSER ERROR: Bad scopestack or loopstack.");
    }
    /*
        handle type checking and most of that language-y stuff.
    */
    validate_function((symbol_table + active_function)[0]);
}





void parse_stmts(){

    while(!peek_match_keyw("end")) parse_stmt();

    require(peek_match_keyw("end"), "Statement list ends with 'end'");
    consume();
    /*special case- empty body.*/
    if(scopestack_gettop()->nstmts == 0){
        
        parser_push_statement_nop();
    }
}

int  parse_stmts_allow_else_chain(){
    while(
        !peek_match_keyw("end")
        &&
        !peek_match_keyw("else")
        &&
        !peek_match_keyw("elif")
    ) parse_stmt();

    /*special case- empty body.*/
    if(scopestack_gettop()->nstmts == 0){
        parser_push_statement_nop();
    }

    if(peek_match_keyw("end")) return 0;
    if(peek_match_keyw("else")) return 1;
    if(peek_match_keyw("elif")) return 2;

    parse_error(
    "If/elif Statement list ends with 'end' or 'else' or 'elif'"
    );	
}

void parse_label(){
    //TODO
    stmt* me;
    require(peek()->data == TOK_OPERATOR, "label syntax is :myLabel");
    require(streq(peek()->text,":"), "Label syntax is :myLabel");
    consume();
    require(peek()->data == TOK_IDENT, "label syntax takes identifer");
    require(!peek_is_typename(), "labels may not be declared with the name of a type!");

    me = parser_push_statement();
    me->kind = STMT_LABEL;
    require(!label_name_is_already_in_use(peek()->text),"Label name is already in use.");
    me->referenced_label_name = strdup(peek()->text);
    consume();
    return;
}

void parse_goto(){
    stmt* me;
    me = parser_push_statement();
    me->kind = STMT_GOTO;
    me->nexpressions = 0;
    consume_keyword("goto");
    parse_repeatedly_try_metaprogramming(); //allow parsehooks in the label name field of a goto statement.
    require(peek()->data == TOK_IDENT, "goto takes label identifer");
    me->referenced_label_name = strdup(peek()->text);
    consume();
    //we don't need a semicolon, don't eat one!
}

void parse_asm_stmt(){
    stmt* me;
    me = parser_push_statement();
    me->kind = STMT_ASM;
    me->nexpressions = 0;
    consume_keyword("asm");
    require(peek()->data == TOK_OPAREN, "asm needs opening parentheses"); 
    consume();
    parse_repeatedly_try_metaprogramming(); //allow parsehooks inside of an asm() statement.
    require(peek()->data == TOK_STRING, "asm takes a string.");
    me->referenced_label_name = strdup(peek()->text);
    consume();
    require(peek()->data == TOK_CPAREN, "asm needs closing parentheses"); 
    consume();
}

void parse_switch(){
    stmt* me;
    expr_node* e = NULL;
    require(peek()->data == TOK_KEYWORD, "switch statement requires switch keyword.");
    require(ID_KEYW(peek()) == ID_KEYW_STRING("switch"), "switch statement requires switch keyword.");
    consume();

    me = parser_push_statement();
    me->kind = STMT_SWITCH;
    me->nexpressions = 1;
    require(peek()->data == TOK_OPAREN, "switch statement needs opening parentheses");
    consume();
    parse_expr(&e);
    //require(e != NULL, "expression required for switch statement.");
    require(peek()->data == TOK_CPAREN, "switch statement needs closing parentheses");
    consume();
    while(1){
        parse_repeatedly_try_metaprogramming(); //allow parsehooks to generate switch labels...
        require(peek()->data == TOK_IDENT, "switch statement expected identifier");
        require(!peek_is_typename(), "switch statement not allowed to take a typename...");

        me->switch_label_list = re_allocX(me->switch_label_list, (++me->switch_nlabels) * sizeof(char*));

        me->switch_label_list[me->switch_nlabels-1] = strdup(peek()->text);
        consume();
        if(peek_is_semic()) {consume();break;}
        if(peek()->data == TOK_COMMA){
            consume();
            if(peek_is_semic()) {consume();break;}
        }
    }
    require(me->switch_nlabels > 0, "switch statement must have at least one label.");
    me->switch_label_indices = c_allocX(8 * me->switch_nlabels);
    //end:;
    me->expressions[0] = e;
}




void parse_stmt(){
    if(peek()->data == TOK_SEMIC) {consume();return;}
    if(peek_match_keyw("fn")){
        parse_error("ERROR! Function declaration inside of a scope!");
    }
    if(peek_match_keyw("method")){
        parse_error("ERROR! Method declaration inside of a scope!");
    }
    if(peek_match_keyw("data")){
        parse_error("ERROR! Data declaration inside of a scope!");
    }
    if(peek_match_keyw("codegen")){
        parse_error("ERROR! codegen keyword found while parsing scope! This is never valid!");
    }
    if(peek_match_keyw("end"))
    {
        parse_error("Internal error: 'end' reached parse_stmt.");
    }
    if(peek_match_keyw("else")){
        puts("I found an `else` but we are not continuing a if-elif-else chain....");
        puts("In seabass, you do if-else like this:");
        puts("if(1)\n    myCode();\nelse\n    myCode2();\nend");
        puts("You can also put `elif` statements in...");
        puts("if(0)\n    myCode();\nelif(2)\n    myCode2()\nelse\n    myCode3();\nend");
        parse_error("stray else in program.");
    }
    if(peek_match_keyw("elif")){
        //print the exact thing they typed...
        if(streq(peek()->text, "elif"))
            puts("I found an `elif` but we are not continuing a if-elif-else chain....");
        if(streq(peek()->text, "elseif"))
            puts("I found an `elseif` but we are not continuing a if-elif-else chain....");
        puts("In seabass, you do if-else like this:");
        puts("if(1)\n    myCode();\nelse\n    myCode2();\nend");
        puts("You can also put `elif` statements in...");
        puts("if(0)\n    myCode();\nelif(2)\n    myCode2()\nelse\n    myCode3();\nend");
        parse_error("stray elseif/elif in program.");
    }
    if(peek_match_keyw("if")) {
        parse_if();
        return;
    }
    if(peek_match_keyw("goto")) {
        parse_goto();
        return;
    }
    if(peek()->data == TOK_OCB){
        parse_error("This is Seabass, not C. Curly braces are never used for scoping.");
    }
    if(peek()->data == TOK_OPERATOR)
    if(streq(peek()->text , ":")) {
        parse_label();
        return;
    }
    if(peek_match_keyw("for")){
        parse_for();
        return;
    }
    if(peek_match_keyw("while")) {
        parse_while();
        return;
    }
    if(peek_match_keyw("continue")){
        parse_continue_break();
        return;
    }
    if(peek_match_keyw("break")){
        parse_continue_break();
        return;
    }
    if(peek_match_keyw("switch")){
        parse_switch();
        return;
    }
    if(peek_match_keyw("return")){
        parse_return();
        return;
    }	
    if(peek_match_keyw("tail")){
        parse_tail();
        return;
    }
    if(peek_match_keyw("asm")){
        parse_asm_stmt();
        return;
    }
    if(peek_is_typename()){
        parse_lvardecl();
        return;
    }
    
    //parse_repeatedly_try_metaprogramming(); //allow parsehooks at local scope.
    if(peek()->data == TOK_OPERATOR){
    if(streq(peek()->text, "@")){
        char* t;
        uint64_t i;
        uint64_t id;
        int found = 0;
        consume();
        require(peek()->data == TOK_IDENT, "Parserhook requires identifier");
        //require(peek()->text != NULL, "Internal error, identifier does not have text?");
        t = strdup(peek()->text);
        t = strcataf2("parsehook_",t);
        for(i = 0; i < nsymbols; i++){
            if(streq(symbol_table[i]->name, t)){
                id = i;
                found = 1;
                break;
            }
        }
        require(found != 0, "Could not find parsehook_ function corresponding with ");
        free(t);
        consume();
        require(symbol_table[id]->t.is_function != 0, "parsehook must be a function.");
        require(symbol_table[id]->is_codegen != 0, "parsehook must be is_codegen.");
        require(symbol_table[id]->is_incomplete == 0, "parsehook definition must be completed.");
        require(symbol_table[id]->fbody != NULL, "parsehook function body must not be null.");
        require(symbol_table[id]->t.basetype == BASE_VOID, "parsehook must return nothing!");
        require(symbol_table[id]->t.pointerlevel == 0 , "parsehook must return nothing, not even a pointer to nothing!");
        require(symbol_table[id]->nargs == 0, "parsehook must take zero arguments. That's how I call it.");
        ast_vm_stack_push_temporary();
        ast_execute_function((symbol_table+id)[0]);
        ast_vm_stack_pop();
        return;
    }}
    
    parse_expr_stmt();
    return;
}


void* m_allocX(uint64_t sz){
    void* p = malloc(sz);
    /*
    if(p == NULL){
        puts("malloc failed");
        exit(1);
    }
    */
    return p;
}
void* c_allocX(uint64_t sz){
    void* p = calloc(sz, 1);
    /*
    if(p == NULL){
        puts("calloc failed");
        exit(1);
    }*/
    return p;
}


void* re_allocX(void* p, uint64_t sz){
    p = realloc(p, sz);

    return p;
}


void consume_keyword(char* s){
    char* msg;
    msg = strcata("Failed Keyword Requirement:",s);
    require(peek_match_keyw(s), msg);
    free(msg);
    consume();
    return;
}
