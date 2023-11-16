#include "stringutil.h"
#include "targspecifics.h"

#include "libmin.h"
#include "parser.h"
#include "data.h"
#include "metaprogramming.h"
#include "ast_opt.h"

/*
    Code validator.

    This stuff makes sure that the parsed code is syntactically correct and adds all the proper linkages.

    For instance, "break" and "continue" need to reference what loop they're breaking or continuing.

    Goto needs to identify what label it's going to.

    and every jump needs to keep track of how much scope it's popping.
*/


/*from the VM.*/
uint64_t vm_get_offsetof(
    typedecl* the_struct, 
    char* membername
);

char** discovered_labels = NULL;
uint64_t n_discovered_labels = 0;
uint64_t discovered_labels_capacity = 0;
static stmt* curr_stmt = NULL;
/*target word and sword. Used for bitwise operations and if/switch/elif/while.*/
static uint64_t TARGET_WORD_BASE = BASE_U64;
static uint64_t TARGET_WORD_SIGNED_BASE = BASE_I64;
static uint64_t TARGET_MAX_FLOAT_TYPE = BASE_F64;
static uint64_t TARGET_DISABLE_FLOAT = 0;


void set_target_word(uint64_t val){
    TARGET_WORD_BASE = val;
    TARGET_WORD_SIGNED_BASE = val + 1;
}

uint64_t get_target_word(){
    return TARGET_WORD_BASE;
}

uint64_t get_signed_target_word(){
    return TARGET_WORD_SIGNED_BASE;
}


void set_max_float_type(uint64_t val){
    if(val != BASE_F64 && val != BASE_F32){
        TARGET_MAX_FLOAT_TYPE = 0;
        TARGET_DISABLE_FLOAT = 1;
    } else {
        TARGET_MAX_FLOAT_TYPE = val;
        TARGET_DISABLE_FLOAT = 0;
    }
}

uint64_t get_target_max_float_type(){
    return TARGET_MAX_FLOAT_TYPE;
}


/*
    Has a compatible streq function been implemented for the target?
*/
static inline int impl_streq_exists(){
    for(unsigned long i = 0; i < nsymbols; i++){
        if(streq("impl_streq", symbol_table[i]->name)){
            if(symbol_table[i]->t.is_function == 0) return 0;
            if(symbol_table[i]->is_codegen > 0) return 0;
            if(symbol_table[i]->is_pure == 0) return 0;
            if(symbol_table[i]->t.basetype != TARGET_WORD_SIGNED_BASE) return 0;
            if(symbol_table[i]->t.pointerlevel != 0) return 0;
            if(symbol_table[i]->nargs != 2) return 0;
            if(symbol_table[i]->fargs[0]->basetype != BASE_U8) return 0;
            if(symbol_table[i]->fargs[0]->pointerlevel != 1) 	return 0;
            if(symbol_table[i]->fargs[1]->basetype != BASE_U8) return 0;
            if(symbol_table[i]->fargs[1]->pointerlevel != 1) 	return 0;
            return 1;
        }
    }
    return 0;
}

static inline void validator_exit_err(){
    char buf[80];
    if(curr_stmt){
        mutoa(buf, curr_stmt->linenum);
        if(curr_stmt->filename){
            fputs("File:Line:Col\n",stdout);
            fputs(curr_stmt->filename,stdout);
            fputs(":",stdout);
            mutoa(buf,curr_stmt->linenum);
            fputs(buf,stdout);
            fputs(":",stdout);

            mutoa(buf,curr_stmt->colnum);
            fputs(buf,stdout);
            fputs("\n",stdout);
            
            puts("~~~~\nNote that the line number and column number");
            puts("are where the validator invoked validator_exit_err.");
            puts("\n\n(the actual error may be slightly before,\nor on the previous line)");
            puts("I recommend looking near the location provided,\nnot just that exact spot!");
        }
    }
    exit(1);
}


static int this_specific_scope_has_label(char* c, scope* s){
    uint64_t i;
    stmt* stmtlist = s->stmts;
    for(i = 0; i < s->nstmts; i++)
        if(stmtlist[i].kind == STMT_LABEL)
            if(streq(c, stmtlist[i].referenced_label_name)) /*this is the label we're looking for!*/
                return 1;
    return 0;
}

static int64_t this_specific_scope_get_label_index(char* c, scope* s){
    uint64_t i;
    stmt* stmtlist = s->stmts;
    for(i = 0; i < s->nstmts; i++)
        if(stmtlist[i].kind == STMT_LABEL)
            if(streq(c, stmtlist[i].referenced_label_name)) /*this is the label we're looking for!*/
                return i;
    return -1;
}

static void checkswitch(stmt* sw){
    unsigned long i;
    if(sw->kind != STMT_SWITCH){
        puts("<VALIDATOR ERROR> checkswitch erroneously passed non-switch statement");
        validator_exit_err();
    }
    /*switch never has a scopediff or vardiff because the labels must be in the same scope. Thus, no variables ever have to be popped.*/
    sw->goto_scopediff = 0;
    sw->goto_vardiff = 0;
    for(i = 0; i < sw->switch_nlabels; i++){
        int64_t index = this_specific_scope_get_label_index(sw->switch_label_list[i],sw->whereami);
        if(index == -1)
            {
                puts("Switch statement in function uses label not in its home scope.");
                puts("A switch statement is not allowed to jump out of its own scope, either up or down!");
                puts("Function name:");
                puts(symbol_table[active_function]->name);
                puts("Label in particular:");
                puts(sw->switch_label_list[i]);
                validator_exit_err();
            }
        sw->switch_label_indices[i] = index;
        /*
        if(  ((stmt*)sw->whereami->stmts)[sw->switch_label_indices[i]].kind != STMT_LABEL){
            puts("Internal Validator Error");
            puts("this_specific_scope_get_label_index returned the index of a non-label.");
            validator_exit_err();
        }
        if(
            !streq(
                ((stmt*)sw->whereami->stmts)
                    [sw->switch_label_indices[i]]
                        .referenced_label_name
                ,
                sw->switch_label_list[i]
            )
        ){
            puts("Internal Validator Error");
            puts("this_specific_scope_get_label_index returned the index of wrong label!");
            validator_exit_err();
        }
        */
    }
    return;
}


static void check_label_declarations(scope* lvl){
    unsigned long i;
    unsigned long j;
    stmt* stmtlist = lvl->stmts;
    for(i = 0; i < lvl->nstmts; i++)
    {
        curr_stmt = stmtlist + i;
        if(stmtlist[i].myscope){
            check_label_declarations(stmtlist[i].myscope);
        }else if(stmtlist[i].kind == STMT_LABEL){
            for(j = 0; j < n_discovered_labels;j++){
                if(streq(stmtlist[i].referenced_label_name, discovered_labels[j]))
                {
                    puts("Duplicate label in function:");
                    puts(symbol_table[active_function]->name);
                    puts("Label is:");
                    puts(stmtlist[i].referenced_label_name);
                    validator_exit_err();
                }
            }
            /*add it to the list of discovered labels*/
            n_discovered_labels++;
            if(n_discovered_labels >= discovered_labels_capacity){
                discovered_labels = realloc(discovered_labels, (n_discovered_labels) * sizeof(char*));
            }
            /*
            if(0)
            if(discovered_labels == NULL){
                puts("failed realloc");
                validator_exit_err();
            }
            */
            discovered_labels[n_discovered_labels-1] = stmtlist[i].referenced_label_name;
        }
    }
    return;
}

static void check_switches(scope* lvl){
    unsigned long i;
    stmt* stmtlist = lvl->stmts;
    for(i = 0; i < lvl->nstmts; i++)
    {
        curr_stmt = stmtlist + i;
        if(stmtlist[i].myscope){
            check_switches(stmtlist[i].myscope);
        }else if(stmtlist[i].kind == STMT_SWITCH){
            /**/
            checkswitch(stmtlist+i);
        }
    }
    return;
}



static void assign_lsym_gsym(expr_node* ee){
    int64_t i;
    int64_t j;
    for(i = 0; i < MAX_FARGS; i++){
        if(ee->subnodes[i])
            assign_lsym_gsym(ee->subnodes[i]);
    }
    int64_t depth = 0;
    /*Now, do our assignment.*/
    if(ee->kind == EXPR_SYM)
        for(i = (nscopes - 1); i >= 0; i--){
            for(j = scopestack[i]->nsyms - 1; j >= 0 ; j--){
                if(streq(ee->symname, scopestack[i]->syms[j].name)){
                    ee->kind = EXPR_LSYM;
                    ee->t = scopestack[i]->syms[j].t;
                    ee->symid = depth;
                    return;
                }
                depth++;
            }
        }

    /*Must search active_function's fargs...*/
    if(ee->kind == EXPR_SYM)
        for(i = 0; i < (int64_t)symbol_table[active_function]->nargs; i++){
            if(
                streq(
                    ee->symname, 
                    symbol_table[active_function]->fargs[i]->membername
                )
            ){
                ee->kind = EXPR_LSYM;
                ee->t = *symbol_table[active_function]->fargs[i];
                ee->symid = depth;
                //ee->t.membername;
                return;
            } else {
                depth++; //function arguments are on the stack in reverse order. so we search linearly.
            }
        }
    
    if(ee->kind == EXPR_SYM)
        for(i = 0; i < (int64_t)nsymbols; i++)
            if(streq(ee->symname, symbol_table[i]->name)){
                /*Do some validation. It can't be a function...*/
                if(
                    symbol_table[i]->t.is_function && 
                    symbol_table[i]->nargs == 0
                ){
                    //transform into an EXPR_FCALL....
                    ee->kind = EXPR_FCALL;
                    ee->is_function = 1;
                    ee->symid = i;
                    ee->t = symbol_table[i]->t;
                    ee->t.is_function = 0;
                    return;
                    
                } else if(symbol_table[i]->t.is_function)
                {
                    //
                    puts("<INTERNAL ERROR> Uncaught usage of function (with >0 args) name as identifier");
                    puts("Active Function:");
                    puts(symbol_table[active_function]->name);
                    puts("Identifier:");
                    if(ee->symname)
                        puts(ee->symname);
                    validator_exit_err();
                }
                if(symbol_table[active_function]->is_pure > 0){
                    puts("VALIDATOR ERROR!");
                    puts("You may not use global variables in pure functions.");
                    puts("This is the variable you are not allowed to access:");
                    puts(ee->symname);
                    puts("This is the function you tried to access it from:");
                    puts(symbol_table[active_function]->name);
                    validator_exit_err();
                }
                ee->kind = EXPR_GSYM;
                ee->symid = i;
                //this function is impure because it uses global variables.
                symbol_table[active_function]->is_impure_globals_or_asm = 1;
                symbol_table[active_function]->is_impure = 1;
                return;
            }
    if(ee->kind == EXPR_SYM){
        puts("<VALIDATION ERROR> Unknown identifier...");
        puts("Active Function:");
        puts(symbol_table[active_function]->name);
        puts("Identifier:");
        if(ee->symname)
            puts(ee->symname);
        validator_exit_err();
    }
    return;
}



static void throw_type_error(char* msg){
    puts("TYPING ERROR!");
    puts(msg);
    puts("Function:");
    puts(symbol_table[active_function]->name);
    validator_exit_err();
}


static void throw_type_error_with_expression_enums(char* msg, unsigned a, unsigned b){

    unsigned c;
    puts("TYPING ERROR!");
    puts(msg);
    puts("Function:");
    puts(symbol_table[active_function]->name);
    c = a;
    puts("a=");
    if(c == EXPR_SIZEOF) puts("EXPR_SIZEOF");
    if(c == EXPR_INTLIT) puts("EXPR_INTLIT");
    if(c == EXPR_FLOATLIT) puts("EXPR_FLOATLIT");
    if(c == EXPR_STRINGLIT) puts("EXPR_STRINGLIT");
    if(c == EXPR_LSYM) puts("EXPR_LSYM");
    if(c == EXPR_GSYM) puts("EXPR_GSYM");
    if(c == EXPR_SYM) puts("EXPR_SYM");
    if(c == EXPR_POST_INCR) puts("EXPR_POST_INCR");
    if(c == EXPR_POST_DECR) puts("EXPR_POST_DECR");
    if(c == EXPR_PRE_DECR) puts("EXPR_PRE_DECR");
    if(c == EXPR_PRE_INCR) puts("EXPR_PRE_INCR");
    if(c == EXPR_INDEX) puts("EXPR_INDEX");
    if(c == EXPR_MEMBER) puts("EXPR_MEMBER");
    if(c == EXPR_MEMBERPTR) puts("EXPR_MEMBERPTR");
    if(c == EXPR_METHOD) puts("EXPR_METHOD");
    if(c == EXPR_CAST) puts("EXPR_CAST");
    if(c == EXPR_NEG) puts("EXPR_NEG");
    if(c == EXPR_NOT) puts("EXPR_NOT");
    if(c == EXPR_COMPL) puts("EXPR_COMPL");
    if(c == EXPR_MUL) puts("EXPR_MUL");
    if(c == EXPR_DIV) puts("EXPR_DIV");
    if(c == EXPR_MOD) puts("EXPR_MOD");
    if(c == EXPR_ADD) puts("EXPR_ADD");
    if(c == EXPR_SUB) puts("EXPR_SUB");
    if(c == EXPR_BITOR) puts("EXPR_BITOR");
    if(c == EXPR_BITAND) puts("EXPR_BITAND");
    if(c == EXPR_BITXOR) puts("EXPR_BITXOR");
    if(c == EXPR_LSH) puts("EXPR_LSH");
    if(c == EXPR_RSH) puts("EXPR_RSH");
    if(c == EXPR_LOGOR) puts("EXPR_LOGOR");
    if(c == EXPR_LOGAND) puts("EXPR_LOGAND");
    if(c == EXPR_LT) puts("EXPR_LT");
    if(c == EXPR_LTE) puts("EXPR_LTE");
    if(c == EXPR_GT) puts("EXPR_GT");
    if(c == EXPR_GTE) puts("EXPR_GTE");
    if(c == EXPR_EQ) puts("EXPR_EQ");
    if(c == EXPR_STREQ) puts("EXPR_STREQ");
    if(c == EXPR_STRNEQ) puts("EXPR_STRNEQ");
    if(c == EXPR_NEQ) puts("EXPR_NEQ");
    if(c == EXPR_ASSIGN) puts("EXPR_ASSIGN");
    if(c == EXPR_MOVE) puts("EXPR_MOVE");
    if(c == EXPR_FCALL) puts("EXPR_FCALL");
    if(c == EXPR_GETFNPTR) puts("EXPR_GETFNPTR");
    if(c == EXPR_CALLFNPTR) puts("EXPR_CALLFNPTR");
    if(c == EXPR_BUILTIN_CALL) puts("EXPR_BUILTIN_CALL");
    if(c == EXPR_CONSTEXPR_FLOAT) puts("EXPR_CONSTEXPR_FLOAT");
    if(c == EXPR_CONSTEXPR_INT) puts("EXPR_CONSTEXPR_INT");

    if(b == EXPR_BAD) validator_exit_err();
    puts("b=");
    c = b;
    if(c == EXPR_SIZEOF) puts("EXPR_SIZEOF");
    if(c == EXPR_INTLIT) puts("EXPR_INTLIT");
    if(c == EXPR_FLOATLIT) puts("EXPR_FLOATLIT");
    if(c == EXPR_STRINGLIT) puts("EXPR_STRINGLIT");
    if(c == EXPR_LSYM) puts("EXPR_LSYM");
    if(c == EXPR_GSYM) puts("EXPR_GSYM");
    if(c == EXPR_SYM) puts("EXPR_SYM");
    if(c == EXPR_POST_INCR) puts("EXPR_POST_INCR");
    if(c == EXPR_POST_DECR) puts("EXPR_POST_DECR");
    if(c == EXPR_PRE_DECR) puts("EXPR_PRE_DECR");
    if(c == EXPR_PRE_INCR) puts("EXPR_PRE_INCR");
    if(c == EXPR_INDEX) puts("EXPR_INDEX");
    if(c == EXPR_MEMBER) puts("EXPR_MEMBER");
    if(c == EXPR_MEMBERPTR) puts("EXPR_MEMBERPTR");
    if(c == EXPR_METHOD) puts("EXPR_METHOD");
    if(c == EXPR_CAST) puts("EXPR_CAST");
    if(c == EXPR_NEG) puts("EXPR_NEG");
    if(c == EXPR_NOT) puts("EXPR_NOT");
    if(c == EXPR_COMPL) puts("EXPR_COMPL");
    if(c == EXPR_MUL) puts("EXPR_MUL");
    if(c == EXPR_DIV) puts("EXPR_DIV");
    if(c == EXPR_MOD) puts("EXPR_MOD");
    if(c == EXPR_ADD) puts("EXPR_ADD");
    if(c == EXPR_SUB) puts("EXPR_SUB");
    if(c == EXPR_BITOR) puts("EXPR_BITOR");
    if(c == EXPR_BITAND) puts("EXPR_BITAND");
    if(c == EXPR_BITXOR) puts("EXPR_BITXOR");
    if(c == EXPR_LSH) puts("EXPR_LSH");
    if(c == EXPR_RSH) puts("EXPR_RSH");
    if(c == EXPR_LOGOR) puts("EXPR_LOGOR");
    if(c == EXPR_LOGAND) puts("EXPR_LOGAND");
    if(c == EXPR_LT) puts("EXPR_LT");
    if(c == EXPR_LTE) puts("EXPR_LTE");
    if(c == EXPR_GT) puts("EXPR_GT");
    if(c == EXPR_GTE) puts("EXPR_GTE");
    if(c == EXPR_EQ) puts("EXPR_EQ");
    if(c == EXPR_STREQ) puts("EXPR_STREQ");
    if(c == EXPR_STRNEQ) puts("EXPR_STRNEQ");
    if(c == EXPR_NEQ) puts("EXPR_NEQ");
    if(c == EXPR_ASSIGN) puts("EXPR_ASSIGN");
    if(c == EXPR_MOVE) puts("EXPR_MOVE");
    if(c == EXPR_FCALL) puts("EXPR_FCALL");
    if(c == EXPR_GETFNPTR) puts("EXPR_GETFNPTR");
    if(c == EXPR_CALLFNPTR) puts("EXPR_CALLFNPTR");
    if(c == EXPR_BUILTIN_CALL) puts("EXPR_BUILTIN_CALL");
    if(c == EXPR_CONSTEXPR_FLOAT) puts("EXPR_CONSTEXPR_FLOAT");
    if(c == EXPR_CONSTEXPR_INT) puts("EXPR_CONSTEXPR_INT");

    
    validator_exit_err();
}

static void propagate_types(expr_node* ee){
    unsigned long i;
    unsigned long j;
    type t = {0};
    type t2 = {0};
    uint64_t WORD_BASE;
    uint64_t SIGNED_WORD_BASE;
    uint64_t FLOAT_BASE;
    if(symbol_table[active_function]->is_codegen){
        WORD_BASE = BASE_U64;
        SIGNED_WORD_BASE = BASE_I64;
        FLOAT_BASE = BASE_F64;
    } else {
        WORD_BASE = TARGET_WORD_BASE;
        SIGNED_WORD_BASE = TARGET_WORD_SIGNED_BASE;
        FLOAT_BASE = TARGET_MAX_FLOAT_TYPE;
    }
    
    for(i = 0; i < MAX_FARGS; i++){
        if(ee->subnodes[i])
            propagate_types(ee->subnodes[i]);
        if(!ee->subnodes[i]) continue; //in case we ever implement something that has null members...
    /*
        Do double-checking of the types in the subnodes.

        particularly, we may not have struct rvalues, or lvalue structs.

        must have pointer-to-struct or better.
    */
        if(ee->subnodes[i]->t.basetype == BASE_STRUCT)
            if(ee->subnodes[i]->t.pointerlevel == 0)
            {
                throw_type_error_with_expression_enums(
                    "expression node validated as having BASE_STRUCT without a pointer level..."
                    ,ee->subnodes[i]->kind,
                    EXPR_BAD
                );
            }
        if(ee->subnodes[i]->t.arraylen > 0){
            throw_type_error_with_expression_enums(
                "Expression node validated as having an array length.",
                ee->subnodes[i]->kind,
                EXPR_BAD
            );
        }
        if(ee->subnodes[i]->t.is_function){
            throw_type_error_with_expression_enums(
                "Expression node validated as being a function. What?",
                ee->subnodes[i]->kind,
                EXPR_BAD
            );
        }
    }


/*
        TYPE GUARD

        prevent most illegal type operations.
*/
    back_to_the_top:;

    if(ee->kind != EXPR_FCALL)/*Disallow void from this point onward.*/
    if(ee->kind != EXPR_BUILTIN_CALL)/*Disallow void from this point onward.*/
    if(ee->kind != EXPR_CALLFNPTR)/*Disallow void from this point onward.*/
    {
        if(ee->subnodes[0])
            if(ee->subnodes[0]->t.basetype == BASE_VOID)
                throw_type_error_with_expression_enums(
                    "Math expressions may never have void in them. a=Parent,b=Child:",
                    ee->kind,
                    ee->subnodes[0]->kind
                );
        if(ee->subnodes[1])
            if(ee->subnodes[1]->t.basetype == BASE_VOID)
                throw_type_error_with_expression_enums(
                    "Math expressions may never have void in them. a=Parent,b=Child:",
                    ee->kind,
                    ee->subnodes[0]->kind
                );
    }


    /*Forbid pointers for arithmetic purposes.*/
    if(ee->kind != EXPR_STREQ)
    if(ee->kind != EXPR_STRNEQ)
    if(ee->kind != EXPR_EQ)
    if(ee->kind != EXPR_NEQ)
    if(ee->kind != EXPR_ADD)
    if(ee->kind != EXPR_SUB)
    if(ee->kind != EXPR_FCALL)
    if(ee->kind != EXPR_BUILTIN_CALL)
    if(ee->kind != EXPR_CALLFNPTR)
    if(ee->kind != EXPR_PRE_INCR)
    if(ee->kind != EXPR_PRE_DECR)
    if(ee->kind != EXPR_POST_INCR)
    if(ee->kind != EXPR_POST_DECR)
    if(ee->kind != EXPR_ASSIGN) /*you can assign pointers...*/
    if(ee->kind != EXPR_MOVE) /*you can assign pointers...*/
    if(ee->kind != EXPR_CAST) /*You can cast pointers...*/
    if(ee->kind != EXPR_INDEX) /*You can index pointers...*/
    if(ee->kind != EXPR_METHOD) /*You invoke methods on or with pointers.*/
    if(ee->kind != EXPR_MEMBER) /*You can get members of structs which are pointed to.*/
    if(ee->kind != EXPR_MEMBERPTR) /*You can get pointers to members of structs which are pointed to.*/
    {
        if(ee->subnodes[0])
            if(ee->subnodes[0]->t.pointerlevel > 0)
                throw_type_error_with_expression_enums(
                    "Pointer math is limited to addition, subtraction, assignment,moving, casting, equality comparison, and indexing.\n"
                    "a=Parent,b=Child",
                    ee->kind,
                    ee->subnodes[0]->kind
                );
        if(ee->subnodes[1])
            if(ee->subnodes[1]->t.pointerlevel > 0)
                throw_type_error_with_expression_enums(
                    "Pointer math is limited to addition, subtraction, assignment,moving, casting, equality comparison, and indexing.\n"
                    "a=Parent,b=Child",
                    ee->kind,
                    ee->subnodes[1]->kind
                );
    }


    
    if(ee->kind == EXPR_INTLIT){
        //t.basetype = BASE_U64;
        t.basetype = WORD_BASE;
        t.is_lvalue = 0;
        t.pointerlevel = 0;
        ee->constint_propagator = 1;
        ee->t = t;
        return;
    }
    if(ee->kind == EXPR_FLOATLIT){
        if(symbol_table[active_function]->is_codegen == 0)
        if(TARGET_DISABLE_FLOAT)
        {
            puts("Validator error:");
            puts("Target has disabled floating point.");
            validator_exit_err();
        }
        //t.basetype = BASE_F64;
        t.basetype = FLOAT_BASE;
        t.is_lvalue = 0;
        t.pointerlevel = 0;
        ee->t = t;
        return;
    }
    if(ee->kind == EXPR_GSYM){
        t = symbol_table[ee->symid]->t;
        if(t.arraylen){
            t.arraylen = 0;
            t.pointerlevel++; //this prevents the next part from triggering...
            t.is_lvalue = 0;
        }
        if(t.pointerlevel == 0){
            if(t.basetype == BASE_STRUCT){
                t.pointerlevel = 1;
                t.is_lvalue = 0;
                ee->was_struct_var = 1;
            }
        }
        ee->t = t;
        return;
    }
    if(ee->kind == EXPR_GETGLOBALPTR){
        t = symbol_table[ee->symid]->t;
        t.is_lvalue = 0;
        if(t.arraylen){ //arrays need arrayness removed.
            t.arraylen = 0;
        }
        if(symbol_table[ee->symid]->is_data == 0){ //pointer level is not increased for data variables. they are already pointers!
            t.pointerlevel++;
        }
        ee->t = t;
        return;
    }
    if(ee->kind == EXPR_LSYM){
        t = ee->t;
        if(t.arraylen){
            t.arraylen = 0;
            t.pointerlevel++; //this prevents the next part from triggering...
            t.is_lvalue = 0;
        }
        if(t.pointerlevel == 0){
            if(t.basetype == BASE_STRUCT){
                t.pointerlevel = 1;
                t.is_lvalue = 0;
                ee->was_struct_var = 1;
            }
        }
        ee->t = t;
        ee->t.membername = NULL; /*If it was a function argument, we dont want the membername thing to be there.*/
        return;
    }
    if(ee->kind == EXPR_SIZEOF){
        t.basetype = WORD_BASE;
        t.is_lvalue = 0;
        t.pointerlevel = 0;
        ee->t = t;
        return;
    }
    if(ee->kind == EXPR_STRINGLIT){
        t.basetype = BASE_U8;
        t.pointerlevel = 1;
        t.is_lvalue = 0;
        ee->t = t;
        return;
    }
    if(ee->kind == EXPR_BUILTIN_CALL){
        uint64_t q = get_builtin_retval(ee->symname);
        t.is_lvalue = 0;
        if(symbol_table[active_function]->is_pure > 0){
            puts("Validator error:");
            puts("Attempted to use a builtin call in a pure function.");
            validator_exit_err();
        }
        if(symbol_table[active_function]->is_codegen == 0){
            puts("Cannot use builtin calls in non-codegen function!");
            validator_exit_err();
        }
        if(q == BUILTIN_PROTO_VOID){
            t.basetype = BASE_VOID;
            t.arraylen = 0;
            t.pointerlevel = 0;
            ee->t = t;
            return;
        }
        if(q == BUILTIN_PROTO_U8_PTR){
            t.basetype = BASE_U8;
            t.arraylen = 0;
            t.pointerlevel = 1;
            ee->t = t;
            return;
        }
        if(q == BUILTIN_PROTO_U8_PTR2){
            t.basetype = BASE_U8;
            t.arraylen = 0;
            t.pointerlevel = 2;
            ee->t = t;
            return;
        }
        if(q == BUILTIN_PROTO_U64_PTR){
            t.basetype = BASE_U64;
            t.arraylen = 0;
            t.pointerlevel = 1;
            ee->t = t;
            return;
        }
        if(q == BUILTIN_PROTO_U64){
            t.basetype = BASE_U64;
            t.arraylen = 0;
            t.pointerlevel = 0;
            ee->t = t;
            return;
        }
        if(q == BUILTIN_PROTO_I64){
            t.basetype = BASE_I64;
            t.arraylen = 0;
            t.pointerlevel = 0;
            ee->t = t;
            return;
        }
        if(q == BUILTIN_PROTO_DOUBLE){
            t.basetype = BASE_F64;
            t.arraylen = 0;
            t.pointerlevel = 0;
            ee->t = t;
            return;
        }
        if(q == BUILTIN_PROTO_I32){
            t.basetype = BASE_I32;
            t.arraylen = 0;
            t.pointerlevel = 0;
            ee->t = t;
            return;
        }
        puts("Internal Error: Unhandled __builtin function return value in type propagator.");
        puts("The unhandled one is:");
        puts(ee->symname);
        validator_exit_err();
    }
    if(ee->kind == EXPR_CONSTEXPR_FLOAT){
        //ee->t.basetype = BASE_F64;
        ee->t.basetype = FLOAT_BASE;
        t.is_lvalue = 0;
        t.pointerlevel = 0;
        return;
    }
    if(ee->kind == EXPR_CONSTEXPR_INT){
        ee->t.basetype = SIGNED_WORD_BASE;
        t.is_lvalue = 0;
        t.pointerlevel = 0;
        ee->constint_propagator = 1;
        return;
    }
    if(ee->kind == EXPR_POST_INCR || 
        ee->kind == EXPR_PRE_INCR ||
        ee->kind == EXPR_POST_DECR ||
        ee->kind == EXPR_PRE_DECR
    ){
        t = ee->subnodes[0]->t;
        if(t.pointerlevel == 0){
            if(t.basetype == BASE_VOID){
                throw_type_error("Can't increment/decrement void!");
            }
            if(t.basetype == BASE_STRUCT){
                throw_type_error("Can't increment/decrement struct");
            }
            if(t.is_lvalue == 0){
                throw_type_error("Outside of a constant expression, it is invalid to increment or decrement a non-lvalue.");
            }
        }
        t.is_lvalue = 0; /*no longer an lvalue.*/
        ee->t = t;
        return;
    }	
    if(ee->kind == EXPR_INDEX){
        t = ee->subnodes[0]->t;
        if(t.pointerlevel == 0){
            throw_type_error("Can't index non-pointer!");
        }
        t.pointerlevel--;
        t.is_lvalue = 1; /*if it wasn't an lvalue before, it is now!*/
        if(t.pointerlevel == 0){
            if(t.basetype == BASE_STRUCT)
            {
                ee->kind = EXPR_ADD;
                t.pointerlevel++;
                t.is_lvalue = 0;
                ee->t = t;

                goto back_to_the_top;
            }
            if(t.basetype == BASE_VOID)
                throw_type_error("Can't deref pointer to void.");
        }
        ee->t = t;
        if(
            ee->subnodes[1]->t.pointerlevel > 0 ||
            ee->subnodes[1]->t.basetype == BASE_F32 ||
            ee->subnodes[1]->t.basetype == BASE_F64 ||
            ee->subnodes[1]->t.basetype == BASE_VOID ||
            ee->subnodes[1]->t.basetype == BASE_STRUCT ||
            ee->subnodes[1]->t.basetype > BASE_STRUCT
        ){
            throw_type_error("Indexing requires an integer.");
        }
        return;
    }
    if(ee->kind == EXPR_MEMBER){
        //int found = 0;
        t = ee->subnodes[0]->t;
        if(t.basetype != BASE_STRUCT)
            throw_type_error_with_expression_enums(
                "Can't access member of non-struct! a=me",
                ee->kind,
                ee->subnodes[0]->kind
            );
        if(t.structid >= ntypedecls)
            throw_type_error("Internal error, invalid struct ID for EXPR_MEMBER");
        if(ee->symname == NULL)
            throw_type_error("Internal error, EXPR_MEMBER had null symname...");
        for(j = 0; j < type_table[t.structid].nmembers; j++){
            if(
                streq(
                    type_table[t.structid].members[j].membername,
                    ee->symname
                )
            ){
            //	found = 1;
                ee->t = type_table[t.structid].members[j];
                ee->idata = vm_get_offsetof(type_table + t.structid, ee->symname);
                ee->t.is_lvalue = 1;
                //handle: struct member is array.
                if(ee->t.arraylen){
                    ee->t.is_lvalue = 0;
                    ee->t.arraylen = 0;
                    ee->t.pointerlevel++;
                }
                //handle: struct member is also a struct, but not pointer to struct.
                //note that for an array of structs, pointerlevel was already set,
                //so we don't have to worry about that here.
                if(ee->t.basetype == BASE_STRUCT)
                if(ee->t.pointerlevel == 0){
                    ee->t.is_lvalue = 0;
                    ee->t.arraylen = 0;
                    ee->t.pointerlevel = 1;
                }
                //handle:struct member is function
                if(ee->t.is_function){
                    puts("Error: Struct member is function. How did that happen?");
                    throw_type_error("Struct member is function.");
                }
                ee->t.membername = NULL; /*We don't want it!*/
                return;
            }
        }
        
        {
            puts("Struct:");
            puts(type_table[t.structid].name);
            puts("Does not have member:");
            puts(ee->symname);
            throw_type_error_with_expression_enums(
                "Struct lacking member. a=me",
                ee->kind,
                EXPR_BAD
            );
            validator_exit_err();
        }
        return;
    }

    if(ee->kind == EXPR_MEMBERPTR){
        //int found = 0;
        t = ee->subnodes[0]->t;
        if(t.basetype != BASE_STRUCT)
            throw_type_error_with_expression_enums(
                "Can't access memberptr of non-struct! a=me",
                ee->kind,
                ee->subnodes[0]->kind
            );
        if(t.structid >= ntypedecls)
            throw_type_error("Internal error, invalid struct ID for EXPR_MEMBERPTR");
        if(ee->symname == NULL)
            throw_type_error("Internal error, EXPR_MEMBERPTR had null symname...");
        for(j = 0; j < type_table[t.structid].nmembers; j++){
            if(
                streq(
                    type_table[t.structid].members[j].membername,
                    ee->symname
                )
            ){
            //	found = 1;
                ee->t = type_table[t.structid].members[j];
                ee->t.is_lvalue = 0;
                ee->idata = vm_get_offsetof(type_table + t.structid, ee->symname);
                //handle: struct member is array.
                if(ee->t.arraylen > 0){
                    ee->t.arraylen = 0;
                }
                ee->t.pointerlevel++; //for arrays, this is what we would do if we were decaying an array, anyway!
                //handle:struct member is function
                if(ee->t.is_function){
                    puts("Error: Struct member is function. How did that happen?");
                    throw_type_error("Struct member is function.");
                }
                ee->t.membername = NULL; /*We don't want it!*/
                return;
            }
        }
        {
            puts("Struct:");
            puts(type_table[t.structid].name);
            puts("Does not have member, therefore cannot retrieve pointer:");
            puts(ee->symname);
            throw_type_error_with_expression_enums(
                "Struct lacking member. a=me",
                ee->kind,
                EXPR_BAD
            );
            validator_exit_err();
        }
        return;
    }
    if(ee->kind == EXPR_METHOD){
        char* c;
        t = ee->subnodes[0]->t;
        if(t.basetype != BASE_STRUCT)
            throw_type_error("Can't call method on non-struct!");
        if(t.structid >= ntypedecls)
            throw_type_error("Internal error, invalid struct ID for EXPR_METHOD");
        if(ee->method_name == NULL)
            throw_type_error("Internal error, EXPR_METHOD had null methodname...");
        /*
            do name mangling.
        */
        if(ee->symname == 0){
            c = strcata("__method_", type_table[t.structid].name);
            c = strcataf1(c, "_____");
            c = strcataf1(c, ee->method_name);
            ee->symname = c;
        } else {
            c = ee->symname;
        }
        for(j = 0; j < nsymbols; j++)
            if(streq(c, symbol_table[j]->name)){
                if(symbol_table[j]->t.is_function == 0){
                    puts("Weird things have been happening with the method table.");
                    puts("It appears you've been using reserved (__) names...");
                    puts("Don't do that!");
                    puts("The method I was looking for is:");
                    puts(c);
                    puts("And I found a symbol by that name, but it is not a function.");
                    throw_type_error("Method table messed up");
                }
                if(symbol_table[j]->nargs == 0){
                    puts("This method:");
                    puts(c);
                    puts("Takes no arguments? Huh?");
                    throw_type_error("Method table messed up");
                }
                if(symbol_table[j]->fargs[0]->basetype != BASE_STRUCT){
                    puts("This method:");
                    puts(c);
                    puts("Does not take a struct pointer as its first argument. Huh?");
                    throw_type_error("Method table messed up");
                }
                if(symbol_table[j]->fargs[0]->structid != t.structid){
                    puts("This method:");
                    puts(c);
                    puts("Was apparently somehow called, as a method, on a non-matching struct.");
                    throw_type_error("Method shenanigans");
                }
                if(symbol_table[active_function]->is_pure)
                    if(symbol_table[j]->is_pure == 0){
                        puts("<VALIDATOR ERROR>");
                        puts("You tried to invoke this method:");
                        puts(ee->method_name);
                        puts("On a struct of this type:");
                        puts(type_table[t.structid].name);
                        puts("But that method is not declared 'pure', and you are trying to invoke it in a pure function.");
                        puts("The pure function's name is:");
                        puts(symbol_table[active_function]->name);
                        throw_type_error("Purity check failure.");
                    }
                if(!!symbol_table[active_function]->is_codegen != !!symbol_table[j]->is_codegen){
                    puts("Validator Error");
                    puts("You tried to invoke this method:");
                    puts(ee->method_name);
                    puts("On a struct of this type:");
                    puts(type_table[t.structid].name);
                    if(!!symbol_table[j]->is_codegen)
                        puts("But that method was declared codegen,");
                    else
                        puts("But that method was not declared codegen,");
                    puts("And the active function:");
                    puts(symbol_table[active_function]->name);
                    if(!!symbol_table[active_function]->is_codegen)
                        puts("is declared codegen.");
                    else
                        puts("is not declared codegen.");
                    throw_type_error("Method codegen check failure.");
                }
                //count how many subnodes we have.
                for(i = 0; ee->subnodes[i] != NULL; i++);
                //this is not meant to be inside the for...
                if(i != symbol_table[j]->nargs){
                    puts("This method:");
                    puts(c);
                    puts("Was called with the wrong number of arguments.");
                    throw_type_error_with_expression_enums(
                        "Method invoked with wrong number of arguments! a=me",
                        ee->kind,
                        EXPR_BAD
                    );
                }
                t2 = symbol_table[j]->t;
                t2.is_function = 0;
                t2.funcid = 0;
                t2.is_lvalue = 0;
                t2.membername = NULL;
                ee->t = t2;
                ee->symid = j;
                return;
            }
        puts("This method:");
        puts(c);
        puts("Does not exist!");
        validator_exit_err();
    }
    if(ee->kind == EXPR_FCALL){
        if(ee->symid >= nsymbols){
            throw_type_error("INTERNAL ERROR: EXPR_FCALL erroneously has bad symbol ID. This should have been resolved in parser.c");
        }
        if(symbol_table[active_function]->is_pure > 0)
            if(symbol_table[ee->symid]->is_pure == 0){
                puts("<VALIDATOR ERROR>");
                puts("You tried to invoke this function:");
                puts(symbol_table[ee->symid]->name);
                puts("But that function is not declared 'pure', and you are trying to invoke it in a pure function.");
                puts("The pure function's name is:");
                puts(symbol_table[active_function]->name);
                throw_type_error("Purity check failure.");
            }
        for(i = 0; ee->subnodes[i] != NULL; i++);
        if(i != symbol_table[ee->symid]->nargs){
            puts("This function:");
            puts(ee->symname);
            puts("Was called with the wrong number of arguments.");
            throw_type_error_with_expression_enums(
                "function invoked with wrong number of arguments! a=me",
                ee->kind,
                EXPR_BAD
            );
        }
        
        ee->t = symbol_table[ee->symid]->t;
        ee->t.is_function = 0;
        ee->t.funcid = 0;
        ee->t.is_lvalue = 0; /*You can't assign to the output of a function*/
        return;
    }
    if(ee->kind == EXPR_CALLFNPTR){
        if(ee->symid >= nsymbols){
            throw_type_error("INTERNAL ERROR: EXPR_CALLFNPTR erroneously has bad symbol ID. This should have been resolved in parser.c");
        }
        if(symbol_table[active_function]->is_pure > 0){
            puts("<VALIDATOR ERROR>");
            puts("You tried to invoke a function pointer in a pure function. That is not allowed!");
            validator_exit_err();
        }
        for(i = 0; ee->subnodes[i] != NULL; i++);
        if(i != symbol_table[ee->symid]->nargs+1){
            puts("Function pointer, with the given prototype:");
            puts(ee->symname);
            puts("Was called with the wrong number of arguments.");
            throw_type_error_with_expression_enums(
                "function pointer invoked with wrong number of arguments! a=me",
                ee->kind,
                EXPR_BAD
            );
        }
        ee->t = symbol_table[ee->symid]->t;
        ee->t.is_function = 0;
        ee->t.funcid = 0;
        ee->t.is_lvalue = 0; /*You can't assign to the output of a function*/
        return;
    }
    /*always yields a char* */
    if(ee->kind == EXPR_GETFNPTR){
        ee->t = type_init();
        ee->t.basetype = BASE_U8;
        ee->t.pointerlevel = 1;
        ee->t.is_lvalue = 0;
        return;
    }

    if(ee->kind == EXPR_CAST){
        t = ee->type_to_get_size_of;
        t2 = ee->subnodes[0]->t;
        if(t.basetype == BASE_STRUCT)
            if(t.pointerlevel == 0)
                throw_type_error_with_expression_enums(
                    "Cannot cast to struct. a=Parent, b=Child",
                    ee->kind,
                    ee->subnodes[0]->kind
                );
        if(symbol_table[active_function]->is_codegen == 0){
            if(TARGET_DISABLE_FLOAT)
                if(t.basetype == BASE_F32 || t.basetype == BASE_F64)
                    throw_type_error("Floating point is disabled on the target and it is invalid to cast to floating point types on it.");
            if(FLOAT_BASE == BASE_F32 && t.basetype == BASE_F64){
                throw_type_error("The target does not support 64 bit floats! You may not use them!");
            }
        }
        if(t.pointerlevel == 0)
            if(t.basetype == BASE_VOID)
                throw_type_error("Cannot cast to void.");
        /*Allow arbitrary pointer-to-pointer casts.*/
        if(t.pointerlevel > 0)
        if(t2.pointerlevel > 0)
        {
            ee->t = t;
            return;
        }
        if(t.pointerlevel > 0){
            if(t2.basetype == BASE_F64 ||
                t2.basetype == BASE_F32
            )
            throw_type_error("You cannot cast floating point types to a pointer.");
        }
        if(t2.pointerlevel > 0){
            if(t.basetype == BASE_F64 ||
                t.basetype == BASE_F32
            )
            throw_type_error("You cannot cast pointer types to a floating point number.");
        }
        if(t2.pointerlevel == 0)
            if(t2.basetype == BASE_VOID)
                throw_type_error("You Cannot cast void to anything, no matter how hard you try!");
        ee->t = t;
        ee->t.is_lvalue = 0; /*Casting _always_ removes lvalue*/
        return;
    }
    
    if(ee->kind == EXPR_STREQ){
        t = ee->subnodes[0]->t;
        t2 = ee->subnodes[1]->t;
        if(t.pointerlevel != 1 ||
            t2.pointerlevel != 1 ||
            t.basetype != BASE_U8 ||
            t2.basetype != BASE_U8
        ){
                throw_type_error("EXPR_STREQ requires two char pointers!");
            }
        if(symbol_table[active_function]->is_codegen == 0)
        {
            if(!impl_streq_exists()){
                puts("Validator error:");
                puts("usage of streq and strneq require this function to be defined with this exact prototype:");
                if(TARGET_WORD_BASE == BASE_U64) puts("fn pure impl_streq(u8* a, u8* b)->i64;");
                if(TARGET_WORD_BASE == BASE_U32) puts("fn pure impl_streq(u8* a, u8* b)->i32;");
                if(TARGET_WORD_BASE == BASE_U16) puts("fn pure impl_streq(u8* a, u8* b)->i16;");
                if(TARGET_WORD_BASE == BASE_U8) puts("fn pure impl_streq(u8* a, u8* b)->i8;");
                validator_exit_err();
            }
        }
        ee->t = type_init();
        //ee->t.basetype = BASE_I64;
        ee->t.basetype = SIGNED_WORD_BASE;
        ee->t.is_lvalue = 0;
        return;
    }
    if(ee->kind == EXPR_STRNEQ){
        t = ee->subnodes[0]->t;
        t2 = ee->subnodes[1]->t;
        if(t.pointerlevel != 1 ||
            t2.pointerlevel != 1 ||
            t.basetype != BASE_U8 ||
            t2.basetype != BASE_U8){
                throw_type_error("EXPR_STRNEQ requires two char pointers!");
            }
        if(symbol_table[active_function]->is_codegen == 0)
        {
            if(!impl_streq_exists()){
                puts("Validator error:");
                puts("usage of streq and strneq require this function to be defined with this exact prototype:");
                puts("fn pure impl_streq(u8* a, u8* b)->i64");
                puts("Note this declaration is allowed to be public or static.");
                puts("You may also change the names of the variables.");
                validator_exit_err();
            }
        }
        ee->t = type_init();
        //ee->t.basetype = BASE_I64;
        ee->t.basetype = SIGNED_WORD_BASE;
        ee->t.is_lvalue = 0;
        return;
    }

    if(ee->kind == EXPR_EQ ||
        ee->kind == EXPR_NEQ ||
        ee->kind == EXPR_LT ||
        ee->kind == EXPR_LTE ||
        ee->kind == EXPR_GT ||
        ee->kind == EXPR_GTE
    ){
        unsigned special_exception_pointer_int_compare = 0;
        t = ee->subnodes[0]->t;
        t2 = ee->subnodes[1]->t;
        //check specifically for the case if EQ/NEQ where we are comparing
        //a pointer with an integer constant...
        //TODO: debug why an alternate version of this function seemingly caused segfaults...
        if(ee->kind == EXPR_EQ || ee->kind == EXPR_NEQ){
            int propagator = 0;
            //One of these has a pointer level not like the other...
            if(t.pointerlevel != t2.pointerlevel){
                //determine which of them is NOT the pointer...
                if(t.pointerlevel == 0) {
                    propagator = ee->subnodes[0]->constint_propagator;
                } else if(t2.pointerlevel == 0) {
                    propagator = ee->subnodes[1]->constint_propagator;
                }
            }
            if(propagator){
                special_exception_pointer_int_compare = 1;
                /*
                    Perform a conversion...
                */
            }
        }
        if(!special_exception_pointer_int_compare){
            if(
                ( (t.pointerlevel!=0) != (t2.pointerlevel!=0) )
            ){
                throw_type_error_with_expression_enums("Comparison between incompatible types. Operands:",
                    ee->subnodes[0]->kind,
                    ee->subnodes[1]->kind
                );
            }
        }
        ee->t = type_init();
        //ee->t.basetype = BASE_I64;
        ee->t.basetype = SIGNED_WORD_BASE;
        ee->t.is_lvalue = 0;
        return;
    }

    /*
        TODO: implement property propagation
    */
    if(ee->kind == EXPR_ASSIGN){
        if(
            (ee->subnodes[0]->t.pointerlevel != ee->subnodes[1]->t.pointerlevel)
        ){
            /*
                Explicitly check for the case that we are assigning an
                integer constant to a pointer.
            */
            if(ee->subnodes[1]->constint_propagator == 0){
                throw_type_error_with_expression_enums(
                    "Assignment between incompatible types (pointerlevel) Operands:",
                    ee->subnodes[0]->kind,
                    ee->subnodes[1]->kind
                );
            }
        }
        if(ee->subnodes[0]->t.pointerlevel > 0){
            if(ee->subnodes[1]->constint_propagator == 0)
                if(
                    ee->subnodes[0]->t.basetype != 
                    ee->subnodes[1]->t.basetype
                )
                    throw_type_error_with_expression_enums(
                        "Assignment between incompatible pointer types (basetype) Operands:",
                        ee->subnodes[0]->kind,
                        ee->subnodes[1]->kind
                    );
        }
        if(ee->subnodes[0]->t.basetype == BASE_STRUCT){
            if(ee->subnodes[1]->constint_propagator == 0)
                if(ee->subnodes[0]->t.structid !=
                    ee->subnodes[1]->t.structid)
                    throw_type_error_with_expression_enums(
                        "Assignment between incompatible types- Pointers to different structs. Operands:",
                        ee->subnodes[0]->kind,
                        ee->subnodes[1]->kind
                    );
        }
        if(ee->subnodes[0]->t.is_lvalue == 0){
            throw_type_error_with_expression_enums("Cannot assign to non lvalue. Operands:",
                ee->subnodes[0]->kind,
                ee->subnodes[1]->kind
            );
        }
        ee->t = type_init();
        return;
    }
    if(ee->kind == EXPR_MOVE){
        if(
            (ee->subnodes[0]->t.pointerlevel != ee->subnodes[1]->t.pointerlevel)
        ){
            throw_type_error_with_expression_enums(
                "Move between incompatible types (pointerlevel) Operands:",
                ee->subnodes[0]->kind,
                ee->subnodes[1]->kind
            );
        }
        if(
            ee->subnodes[0]->t.basetype != 
            ee->subnodes[1]->t.basetype
        )
            throw_type_error_with_expression_enums(
                "Move between incompatible pointer types (basetype) Operands:",
                ee->subnodes[0]->kind,
                ee->subnodes[1]->kind
            );
        if(ee->subnodes[0]->t.pointerlevel == 0){
            throw_type_error_with_expression_enums(
                "Move with non-pointers?:",
                ee->subnodes[0]->kind,
                ee->subnodes[1]->kind
            );
        }
        if(ee->subnodes[0]->t.basetype == BASE_STRUCT)
        if(ee->subnodes[0]->t.structid != ee->subnodes[1]->t.structid){
            throw_type_error_with_expression_enums(
                "Move between incompatible struct types:",
                ee->subnodes[0]->kind,
                ee->subnodes[1]->kind
            );
        }
        ee->t = ee->subnodes[0]->t;
        ee->t.is_lvalue = 0;
        return;
    }
    
    if(ee->kind == EXPR_NEG){
        t = ee->subnodes[0]->t;
        if(t.pointerlevel > 0)
            throw_type_error("Cannot negate pointer.");
        if(t.basetype == BASE_U8) t.basetype = BASE_I8;
        if(t.basetype == BASE_U16) t.basetype = BASE_I16;
        if(t.basetype == BASE_U32) t.basetype = BASE_I32;
        if(t.basetype == BASE_U64) t.basetype = BASE_I64;
        ee->t = t;
        ee->t.is_lvalue = 0;
        return;
    }
    if(ee->kind == EXPR_COMPL || 
        ee->kind == EXPR_NOT ||
        ee->kind == EXPR_LOGOR ||
        ee->kind == EXPR_LOGAND ||
        ee->kind == EXPR_BITOR ||
        ee->kind == EXPR_BITAND ||
        ee->kind == EXPR_BITXOR
    ){
        t = ee->subnodes[0]->t;
        //t.basetype = BASE_I64;
        if(ee->kind == EXPR_COMPL || ee->kind == EXPR_NOT){
            if(ee->subnodes[0]->constint_propagator){
                ee->constint_propagator = 1;
            }
        } else if(ee->kind == EXPR_LOGOR ||
                ee->kind == EXPR_LOGAND ||
                ee->kind == EXPR_BITOR ||
                ee->kind == EXPR_BITAND ||
                ee->kind == EXPR_BITXOR
        ){
            if(
                ee->subnodes[0]->constint_propagator &&
                ee->subnodes[1]->constint_propagator
            ){
                ee->constint_propagator = 1;
            }
        }
        t.basetype = WORD_BASE;
        ee->t = t;
        ee->t.is_lvalue = 0;
        return;
    }
    if(
        ee->kind == EXPR_LSH ||
        ee->kind == EXPR_RSH
    ){
        t = ee->subnodes[0]->t;
        if(
            ee->subnodes[0]->constint_propagator &&
            ee->subnodes[1]->constint_propagator
        ){
            ee->constint_propagator = 1;
        }
        //t.basetype = BASE_I64;
        t.basetype = WORD_BASE;
        ee->t = t;
        ee->t.is_lvalue = 0;
        return;
    }
    if(ee->kind == EXPR_MUL ||
        ee->kind == EXPR_DIV
    ){
        t = ee->subnodes[0]->t;
        t2 = ee->subnodes[1]->t;
        if(
            ee->subnodes[0]->constint_propagator &&
            ee->subnodes[1]->constint_propagator
        ){
            ee->constint_propagator = 1;
        }
        
        if(t.pointerlevel > 0 ||
            t2.pointerlevel > 0
        )throw_type_error_with_expression_enums(
                "Cannot do multiplication or division on a pointer.",
                ee->subnodes[0]->kind,
                ee->subnodes[1]->kind
        );

        
        ee->t.basetype = type_promote(t.basetype, t2.basetype);
        ee->t.is_lvalue = 0;
        return;
    }
    if(ee->kind == EXPR_MOD){
        t = ee->subnodes[0]->t;
        t2 = ee->subnodes[1]->t;
        if(
            ee->subnodes[0]->constint_propagator &&
            ee->subnodes[1]->constint_propagator
        ){
            ee->constint_propagator = 1;
        }
        if(t.pointerlevel > 0 ||
            t2.pointerlevel > 0)
            throw_type_error_with_expression_enums(
                "Cannot do multiplication, division, or modulo on a pointer.",
                ee->subnodes[0]->kind,
                ee->subnodes[1]->kind
            );
        if(
            t.basetype == BASE_F32 ||
            t.basetype == BASE_F64 ||
            t2.basetype == BASE_F32 ||
            t2.basetype == BASE_F64
        )
            throw_type_error_with_expression_enums(
                "You can't modulo floats. Not allowed!",
                ee->subnodes[0]->kind,
                ee->subnodes[1]->kind
            );
        ee->t.basetype = type_promote(t.basetype, t2.basetype);
        ee->t.is_lvalue = 0;
        return;
    }
    if(ee->kind == EXPR_ADD){
        ee->t = type_init();
        t = ee->subnodes[0]->t;
        t2 = ee->subnodes[1]->t;
        if(
            ee->subnodes[0]->constint_propagator &&
            ee->subnodes[1]->constint_propagator
        ){
            ee->constint_propagator = 1;
        }
        if(
            t.pointerlevel > 0 &&
            t2.pointerlevel > 0
        )
        throw_type_error_with_expression_enums(
            "Cannot add two pointers.",
            ee->subnodes[0]->kind,
            ee->subnodes[1]->kind
        );

        if(t.pointerlevel > 0 && t2.pointerlevel == 0){
            if(t2.basetype == BASE_F32 || t2.basetype == BASE_F64)
                throw_type_error("Cannot add float to pointer.");
            ee->t = t;
            ee->t.is_lvalue = 0;
            return;
        }
        if(t2.pointerlevel > 0 && t.pointerlevel == 0){
            if(t.basetype == BASE_F32 || t.basetype == BASE_F64)
                throw_type_error("Cannot add float to pointer.");
            ee->t = t2;
            ee->t.is_lvalue = 0;
            return;
        }
        ee->t.basetype = type_promote(t.basetype, t2.basetype);
        ee->t.is_lvalue = 0;
        return;
    }

    
    if(ee->kind == EXPR_SUB){
        ee->t = type_init();
        t = ee->subnodes[0]->t;
        t2 = ee->subnodes[1]->t;
        if(
            ee->subnodes[0]->constint_propagator &&
            ee->subnodes[1]->constint_propagator
        ){
            ee->constint_propagator = 1;
        }
        if(t2.pointerlevel > 0)
            throw_type_error_with_expression_enums(
                "Cannot subtract with second value being pointer. Operands:",
                ee->subnodes[0]->kind,
                ee->subnodes[1]->kind
            );
        /*Neither are pointers? Do type promotion.*/
        if(t.pointerlevel == 0){
            ee->t.basetype = type_promote(t.basetype, t2.basetype);
            return;
        }
        ee->t = t;
        ee->t.is_lvalue = 0;
        return;
    }

    throw_type_error_with_expression_enums(
        "Internal error, ee->kind type is unhandled or fell through.",
        ee->kind,
        EXPR_BAD
    );
    
    /**/
}



static void throw_if_types_incompatible(
    type a,
    type b,
    char* msg,
    int basetypes_must_be_identical
){
    if(
        (a.pointerlevel != b.pointerlevel)
    ){
        puts(msg);
        puts("incompatible types, due to pointerlevel");
        validator_exit_err();
    }
    if(a.pointerlevel > 0){
        if(
            a.basetype != 
            b.basetype
        )
        {
            puts(msg);
            puts("incompatible basetypes");
            validator_exit_err();
        }
    }
    if(a.basetype == BASE_STRUCT)
        if(a.structid != b.structid){
            puts(msg);
            puts("incompatible structs");
            validator_exit_err();
        }
    if(basetypes_must_be_identical)
        if(a.basetype != b.basetype){
            puts(msg);
            validator_exit_err();
        }
}

static char buf_err[1024 * 64];
static void validate_function_argument_passing(expr_node* ee){
    unsigned long i;
    uint64_t nargs;
    uint64_t got_builtin_arg1_type;
    uint64_t got_builtin_arg2_type;
    uint64_t got_builtin_arg3_type;
    type t_target = {0};
    for(i = 0; i < MAX_FARGS; i++){
        if(ee->subnodes[i])
            validate_function_argument_passing(ee->subnodes[i]);
    }


    /*
        TODO
    */
    if(ee->kind == EXPR_BUILTIN_CALL){
        nargs = get_builtin_nargs(ee->symname);
        t_target = type_init();
        /*The hardest one!!*/
        if(nargs == 0) return; /*EZ!*/
         got_builtin_arg1_type = get_builtin_arg1_type(ee->symname);
         if(nargs > 1)
            got_builtin_arg2_type = get_builtin_arg2_type(ee->symname);
        if(nargs > 2)
            got_builtin_arg3_type = get_builtin_arg3_type(ee->symname);
        /*Check argument 1.*/
        if(got_builtin_arg1_type == BUILTIN_PROTO_VOID){
            puts("Internal error: Argument 1 is BUILTIN_PROTO_VOID for EXPR_BUILITIN_CALL. Update metaprogramming.");
            throw_type_error("Internal error: BUILTIN_PROTO_VOID for EXPR_BUILTIN_CALL");
        }
        if(got_builtin_arg1_type == BUILTIN_PROTO_U8_PTR){
            t_target.basetype = BASE_U8;
            t_target.pointerlevel = 1;
        }
        if(got_builtin_arg1_type == BUILTIN_PROTO_U8_PTR2){
            t_target.basetype = BASE_U8;
            t_target.pointerlevel = 2;
        }
        if(got_builtin_arg1_type == BUILTIN_PROTO_U8_PTR2){
            t_target.basetype = BASE_U8;
            t_target.pointerlevel = 2;
        }
        if(got_builtin_arg1_type == BUILTIN_PROTO_U64_PTR){
            t_target.basetype = BASE_U64;
            t_target.pointerlevel = 1;
        }		
        if(got_builtin_arg1_type == BUILTIN_PROTO_U64){
            t_target.basetype = BASE_U64;
            t_target.pointerlevel = 0;
        }
        if(got_builtin_arg1_type == BUILTIN_PROTO_I64){
            t_target.basetype = BASE_I64;
            t_target.pointerlevel = 0;
        }
        if(got_builtin_arg1_type == BUILTIN_PROTO_DOUBLE){
            t_target.basetype = BASE_F64;
            t_target.pointerlevel = 0;
        }
        if(got_builtin_arg1_type == BUILTIN_PROTO_I32){
            t_target.basetype = BASE_I32;
            t_target.pointerlevel = 0;
        }
        throw_if_types_incompatible(
            t_target, 
            ee->subnodes[0]->t, 
            "First argument passed to builtin function is wrong!",
            0
        );
        if(nargs < 2) return;
        t_target = type_init();
        if(got_builtin_arg2_type == BUILTIN_PROTO_VOID){
            puts("Internal error: Argument 2 is BUILTIN_PROTO_VOID for EXPR_BUILITIN_CALL. Update metaprogramming.");
            throw_type_error("Internal error: BUILTIN_PROTO_VOID for EXPR_BUILTIN_CALL");
        }
        if(got_builtin_arg2_type == BUILTIN_PROTO_U8_PTR){
            t_target.basetype = BASE_U8;
            t_target.pointerlevel = 1;
        }
        if(got_builtin_arg2_type == BUILTIN_PROTO_U8_PTR2){
            t_target.basetype = BASE_U8;
            t_target.pointerlevel = 2;
        }
        if(got_builtin_arg2_type == BUILTIN_PROTO_U8_PTR2){
            t_target.basetype = BASE_U8;
            t_target.pointerlevel = 2;
        }
        if(got_builtin_arg2_type == BUILTIN_PROTO_U64_PTR){
            t_target.basetype = BASE_U64;
            t_target.pointerlevel = 1;
        }		
        if(got_builtin_arg2_type == BUILTIN_PROTO_U64){
            t_target.basetype = BASE_U64;
            t_target.pointerlevel = 0;
        }
        if(got_builtin_arg2_type == BUILTIN_PROTO_I64){
            t_target.basetype = BASE_I64;
            t_target.pointerlevel = 0;
        }
        if(got_builtin_arg2_type == BUILTIN_PROTO_DOUBLE){
            t_target.basetype = BASE_F64;
            t_target.pointerlevel = 0;
        }
        if(got_builtin_arg2_type == BUILTIN_PROTO_I32){
            t_target.basetype = BASE_I32;
            t_target.pointerlevel = 0;
        }
        throw_if_types_incompatible(
            t_target, 
            ee->subnodes[1]->t, 
            "Second argument passed to builtin function is wrong!",
            0
        );
        if(nargs < 3) return;
        t_target = type_init();
        if(got_builtin_arg3_type == BUILTIN_PROTO_VOID){
            puts("Internal error: Argument 3 is BUILTIN_PROTO_VOID for EXPR_BUILITIN_CALL. Update metaprogramming.");
            throw_type_error("Internal error: BUILTIN_PROTO_VOID for EXPR_BUILTIN_CALL");
        }
        if(got_builtin_arg3_type == BUILTIN_PROTO_U8_PTR){
            t_target.basetype = BASE_U8;
            t_target.pointerlevel = 1;
        }
        if(got_builtin_arg3_type == BUILTIN_PROTO_U8_PTR2){
            t_target.basetype = BASE_U8;
            t_target.pointerlevel = 2;
        }
        if(got_builtin_arg3_type == BUILTIN_PROTO_U8_PTR2){
            t_target.basetype = BASE_U8;
            t_target.pointerlevel = 2;
        }
        if(got_builtin_arg3_type == BUILTIN_PROTO_U64_PTR){
            t_target.basetype = BASE_U64;
            t_target.pointerlevel = 1;
        }		
        if(got_builtin_arg3_type == BUILTIN_PROTO_U64){
            t_target.basetype = BASE_U64;
            t_target.pointerlevel = 0;
        }
        if(got_builtin_arg3_type == BUILTIN_PROTO_I64){
            t_target.basetype = BASE_I64;
            t_target.pointerlevel = 0;
        }
        if(got_builtin_arg3_type == BUILTIN_PROTO_DOUBLE){
            t_target.basetype = BASE_F64;
            t_target.pointerlevel = 0;
        }
        if(got_builtin_arg3_type == BUILTIN_PROTO_I32){
            t_target.basetype = BASE_I32;
            t_target.pointerlevel = 0;
        }
        throw_if_types_incompatible(
            t_target, 
            ee->subnodes[2]->t, 
            "Third argument passed to builtin function is wrong!",
            0
        );
    }

    if(ee->kind == EXPR_FCALL || ee->kind == EXPR_METHOD || ee->kind == EXPR_CALLFNPTR){
        
        buf_err[0] = 0;
        strcpy(buf_err, "Function Argument Number ");

        
        nargs = symbol_table[ee->symid]->nargs;
        for(i = 0; i < nargs; i++){
            if(ee->kind == EXPR_FCALL)
                strcpy(buf_err, "fn arg ");
            if(ee->kind == EXPR_METHOD)
                strcpy(buf_err, "method arg ");
            if(ee->kind == EXPR_CALLFNPTR)
                strcpy(buf_err, "func pointer arg ");
            mutoa(buf_err + strlen(buf_err), i+1);
            strcat(buf_err, " has the wrong type.\n");
            strcat(buf_err, " function's name is... ");
            strcat(buf_err, symbol_table[ee->symid]->name);
            strcat(buf_err, "\nCurrent function is... ");
            strcat(buf_err, symbol_table[active_function]->name);
            strcat(buf_err, "\n");
            if(ee->kind == EXPR_METHOD)
                strcat(
                    buf_err,
                    "Note that argument number includes 'this' as the invisible first argument."
                );
            if(ee->kind != EXPR_CALLFNPTR){
                throw_if_types_incompatible(
                    symbol_table[ee->symid]->fargs[i][0], 
                    ee->subnodes[i]->t,
                    buf_err,
                    0
                );
            } else{
                throw_if_types_incompatible(
                    symbol_table[ee->symid]->fargs[i][0], 
                    ee->subnodes[i+1]->t,
                    buf_err,
                    0
                );
            }
        }
        return;
    }

}


static void validate_codegen_safety(expr_node* ee){
    unsigned long i;
    for(i = 0; i < MAX_FARGS; i++){
        if(ee->subnodes[i])
            validate_codegen_safety(ee->subnodes[i]);
    }

    /*
        codegen functions may not access non-codegen variables,
        or call non-codegen code.
        Enforce that!
    */
    if(
        ee->kind == EXPR_GSYM||
        ee->kind == EXPR_FCALL || 
        ee->kind == EXPR_METHOD ||
        ee->kind == EXPR_CALLFNPTR ||
        ee->kind == EXPR_GETFNPTR ||
        ee->kind == EXPR_GETGLOBALPTR
    )
    if(
        symbol_table[active_function]->is_codegen !=
        symbol_table[ee->symid]->is_codegen
    ){
        puts("Codegen safety check failed!");
        puts("This function:");
        puts(symbol_table[active_function]->name);
        puts("May not at any time use this symbol:");
        puts(symbol_table[ee->symid]->name);
        puts("Because one of them is declared 'codegen'");
        puts("and the other is not!");
        validator_exit_err();
    }
    if(symbol_table[active_function]->is_codegen == 0)
        if(ee->kind == EXPR_BUILTIN_CALL){
            puts("Codegen safety check failed!");
            puts("This non-codegen function:");
            puts(symbol_table[active_function]->name);
            puts("May not invoke builtin functions. Including this one:");
            puts(ee->symname);
            puts("Because it is not declared 'codegen'");
            validator_exit_err();
        }
}

static void insert_implied_type_conversion_ptr_assign_int_override(
    expr_node** e_ptr, 
    type t
){
    expr_node cast = {0};
    expr_node* allocated = NULL;

    t.is_lvalue = 0; /*No longer an lvalue after conversion.*/
    cast.kind = EXPR_CAST;
    cast.type_to_get_size_of = t;
    cast.t = t;
    cast.subnodes[0] = *e_ptr;
    cast.is_implied = 1;
    
    allocated = calloc(sizeof(expr_node),1);
    allocated[0] = cast;
    e_ptr[0] = allocated;
}

static void insert_implied_type_conversion_ptr_compare_int_ptr_override(
    expr_node** e_ptr, 
    type t
){
    expr_node cast = {0};
    expr_node* allocated = NULL;

    t.is_lvalue = 0; /*No longer an lvalue after conversion.*/
    cast.kind = EXPR_CAST;
    cast.type_to_get_size_of = t;
    cast.t = t;
    cast.subnodes[0] = *e_ptr;
    cast.is_implied = 1;
    
    allocated = calloc(sizeof(expr_node),1);
    allocated[0] = cast;
    e_ptr[0] = allocated;
}

static void insert_implied_type_conversion(expr_node** e_ptr, type t){
    expr_node cast = {0};
    expr_node* allocated = NULL;

    t.is_lvalue = 0; /*No longer an lvalue after conversion.*/
    cast.kind = EXPR_CAST;
    cast.type_to_get_size_of = t;
    cast.t = t;
    cast.subnodes[0] = *e_ptr;
    cast.is_implied = 1;

    if(t.pointerlevel != e_ptr[0][0].t.pointerlevel){
        throw_type_error("Cannot insert implied cast where pointerlevel is not equal!");
    }
    if(t.pointerlevel > 0){
        if(t.basetype != e_ptr[0][0].t.basetype){
            throw_type_error("Cannot insert implied cast between invalid pointers!");
        }
        if(t.basetype == BASE_STRUCT)
        if(e_ptr[0][0].t.structid != t.structid)
            throw_type_error("Cannot insert implied cast between invalid pointers (to structs)!");
    }
    
    if(t.basetype == BASE_STRUCT)
    if(e_ptr[0][0].t.basetype != BASE_STRUCT)
        throw_type_error("Cannot insert implied cast from non struct to struct!");

    if(e_ptr[0][0].t.basetype == BASE_STRUCT)
    if(t.basetype != BASE_STRUCT)
        throw_type_error("Cannot insert implied cast from struct to non-struct!");

    if(t.basetype == BASE_STRUCT)
    if(e_ptr[0][0].t.structid != t.structid)
        throw_type_error("Cannot insert implied cast between invalid structs!");
        


    /*Identical basetype, pointerlevel, and is_lvalue?*/
    if(t.is_lvalue == e_ptr[0][0].t.is_lvalue)
        if(t.basetype == e_ptr[0][0].t.basetype)
            if(t.pointerlevel == e_ptr[0][0].t.pointerlevel)
                return;
    /*
        A type conversion is necessary.
        Even if it means that we're just converting a pointer to an integer.
    */
    allocated = calloc(sizeof(expr_node),1);
    allocated[0] = cast;
    e_ptr[0] = allocated;
}

static void propagate_implied_type_conversions(expr_node* ee){
    unsigned long i;
    uint64_t nargs;
    type t_target = {0};
    uint64_t WORD_BASE;
    uint64_t SIGNED_WORD_BASE;
    uint64_t FLOAT_BASE;
    (void)FLOAT_BASE;
    if(symbol_table[active_function]->is_codegen){
        WORD_BASE = BASE_U64;
        SIGNED_WORD_BASE = BASE_I64;
        FLOAT_BASE = BASE_F64;
    } else {
        WORD_BASE = TARGET_WORD_BASE;
        SIGNED_WORD_BASE = TARGET_WORD_SIGNED_BASE;
        FLOAT_BASE = TARGET_MAX_FLOAT_TYPE;
    }
    (void)WORD_BASE;


    for(i = 0; i < MAX_FARGS; i++){
        if(ee->subnodes[i])
            propagate_implied_type_conversions(ee->subnodes[i]);
    }


    /*The ones we don't do anything for.*/
    if(
        ee->kind == EXPR_SIZEOF ||
        ee->kind == EXPR_INTLIT ||
        ee->kind == EXPR_FLOATLIT ||
        ee->kind == EXPR_STRINGLIT ||
        ee->kind == EXPR_LSYM ||
        ee->kind == EXPR_GSYM ||
        ee->kind == EXPR_POST_INCR ||
        ee->kind == EXPR_PRE_INCR ||
        ee->kind == EXPR_POST_DECR ||
        ee->kind == EXPR_PRE_DECR ||
        //ee->kind == EXPR_MEMBER ||
        ee->kind == EXPR_CAST ||
        ee->kind == EXPR_CONSTEXPR_FLOAT ||
        ee->kind == EXPR_CONSTEXPR_INT
    ) return;

    if(ee->kind == EXPR_MEMBER){
        t_target = ee->subnodes[0]->t;
        t_target.is_lvalue = 0; //must be: not an lvalue.
        insert_implied_type_conversion(
            ee->subnodes+0, 
            t_target
        );
        return;
    }
    if(ee->kind == EXPR_MEMBERPTR){
        t_target = ee->subnodes[0]->t;
        t_target.is_lvalue = 0; //must be: not an lvalue.
        insert_implied_type_conversion(
            ee->subnodes+0, 
            t_target
        );
        return;
    }
    
    if(ee->kind == EXPR_INDEX){
        //t_target.basetype = BASE_I64;
        t_target.basetype = SIGNED_WORD_BASE;
        t_target.is_lvalue = 0;
        insert_implied_type_conversion(
            ee->subnodes+1, 
            t_target
        );
        t_target = ee->subnodes[0]->t;
        t_target.is_lvalue = 0;
        insert_implied_type_conversion(
            ee->subnodes+0, 
            t_target
        );
        return;
    }
    if(ee->kind == EXPR_NEG){
        t_target = ee->t;
        t_target.is_lvalue = 0;
        if(t_target.pointerlevel > 0)
            throw_type_error("Cannot negate pointer.");
        if(t_target.basetype == BASE_U8 ||
            t_target.basetype == BASE_U16 ||
            t_target.basetype == BASE_U32 ||
            t_target.basetype == BASE_U64
        ) t_target.basetype++;

        if(t_target.basetype < BASE_U8 && t_target.basetype > BASE_F64)
            throw_type_error("Cannot negate non-numeric type.");
        insert_implied_type_conversion(
            ee->subnodes + 0,
            t_target
        );
        return;
    }

    if(
        ee->kind == EXPR_COMPL ||
        ee->kind == EXPR_NOT
    ){
        t_target = type_init();
        //t_target.basetype = BASE_I64;

        t_target.basetype = SIGNED_WORD_BASE;
        insert_implied_type_conversion(
            ee->subnodes + 0,
            t_target
        );
        return;
    }

    /*
        TODO: do type promotion instead of total conversion to SIGNED_WORD_BASE
    */
    if(
        ee->kind == EXPR_BITOR ||
        ee->kind == EXPR_BITAND ||
        ee->kind == EXPR_BITXOR ||
        ee->kind == EXPR_LOGOR ||
        ee->kind == EXPR_LOGAND
    ){
        //t_target.basetype = BASE_I64;
        t_target.basetype = WORD_BASE;
        insert_implied_type_conversion(
            ee->subnodes + 0,
            t_target
        );
        insert_implied_type_conversion(
            ee->subnodes + 1,
            t_target
        );
        return;
    }
    if(
        ee->kind == EXPR_LSH ||
        ee->kind == EXPR_RSH
    ){
        //t_target.basetype = BASE_I64;
        t_target.basetype = WORD_BASE;
        insert_implied_type_conversion(
            ee->subnodes + 0,
            t_target
        );
        insert_implied_type_conversion(
            ee->subnodes + 1,
            t_target
        );
        return;
    }

    /*most of the binops...*/
    if(ee->kind == EXPR_MUL ||
        ee->kind == EXPR_DIV ||
        ee->kind == EXPR_MOD ||
        ee->kind == EXPR_LTE ||
        ee->kind == EXPR_LT ||
        ee->kind == EXPR_GT ||
        ee->kind == EXPR_GTE 
    ){
        t_target.basetype = 
        type_promote(
            ee->subnodes[0]->t.basetype, 
            ee->subnodes[1]->t.basetype
        );
        
        insert_implied_type_conversion(
            ee->subnodes + 0,
            t_target
        );
        insert_implied_type_conversion(
            ee->subnodes + 1,
            t_target
        );
        return;
    }

    /*Special cases of eq and neq, add and sub where neither are pointers*/
    if(ee->kind == EXPR_ADD ||
        ee->kind == EXPR_SUB ||
        ee->kind == EXPR_EQ ||
        ee->kind == EXPR_NEQ
    ){
        if(ee->subnodes[0]->t.pointerlevel == 0){
        if(ee->subnodes[1]->t.pointerlevel == 0)
        {
            t_target = type_init();
            t_target.basetype = 
            type_promote(
                ee->subnodes[0]->t.basetype, 
                ee->subnodes[1]->t.basetype
            );
            insert_implied_type_conversion(
                ee->subnodes + 0,
                t_target
            );
            insert_implied_type_conversion(
                ee->subnodes + 1,
                t_target
            );
            return;
        }}
        /*For the specific and unusual case...*/
        if(		ee->kind == EXPR_EQ ||
        ee->kind == EXPR_NEQ)
        if(
            (ee->subnodes[0]->t.pointerlevel != ee->subnodes[1]->t.pointerlevel) &&
            (
                (ee->subnodes[0]->t.pointerlevel == 0) ||
                (ee->subnodes[1]->t.pointerlevel == 0)
            )
        ){
            /*We must insert a cast...*/
            t_target = type_init();
            if(ee->subnodes[0]->t.pointerlevel){
                //cast the integer to the pointer type...
                t_target = ee->subnodes[0]->t;
                insert_implied_type_conversion_ptr_compare_int_ptr_override(
                    ee->subnodes + 1,
                    t_target
                );
            }else{
                //cast the integer to the pointer type...
                t_target = ee->subnodes[1]->t;
                insert_implied_type_conversion_ptr_compare_int_ptr_override(
                    ee->subnodes + 0,
                    t_target
                );
            }
            /*insert the cast...*/
        
        }
    }
    /*streq always takes two pointers.*/
    if(ee->kind == EXPR_STREQ){
        t_target = type_init();
        t_target.basetype = BASE_U8;
        t_target.pointerlevel = 1;
        insert_implied_type_conversion(
            ee->subnodes + 0,
            t_target
        );
        insert_implied_type_conversion(
            ee->subnodes + 1,
            t_target
        );
        return;
    }
    if(ee->kind == EXPR_STRNEQ){
        t_target = type_init();
        t_target.basetype = BASE_U8;
        t_target.pointerlevel = 1;
        insert_implied_type_conversion(
            ee->subnodes + 0,
            t_target
        );
        insert_implied_type_conversion(
            ee->subnodes + 1,
            t_target
        );
        return;
    }

    /*Add Where the first one is a pointer...*/
    if(ee->kind == EXPR_ADD)
        if(ee->subnodes[0]->t.pointerlevel > 0)
        {
            //t_target.basetype = BASE_I64;
            t_target = type_init();
            t_target.basetype = SIGNED_WORD_BASE;
            t_target.pointerlevel = 0;
            insert_implied_type_conversion(
                ee->subnodes + 1,
                t_target
            );
            t_target = ee->t;
            t_target.is_lvalue = 0;
            insert_implied_type_conversion(
                ee->subnodes + 0,
                t_target
            );
            return;
        }
    /*The second...*/	
    if(ee->kind == EXPR_ADD)
        if(ee->subnodes[1]->t.pointerlevel > 0)
        {
            //t_target.basetype = BASE_I64;
            t_target = type_init();
            t_target.basetype = SIGNED_WORD_BASE;
            t_target.pointerlevel = 0;
            insert_implied_type_conversion(
                ee->subnodes + 0,
                t_target
            );
            t_target = ee->t;
            t_target.is_lvalue = 0;
            insert_implied_type_conversion(
                ee->subnodes + 1,
                t_target
            );
            return;
        }
    /*Subtracting from a pointer.*/
    if(ee->kind == EXPR_SUB)
    if(ee->subnodes[0]->t.pointerlevel > 0)
    {
        //t_target.basetype = BASE_I64;
        t_target.basetype = SIGNED_WORD_BASE;
        t_target.pointerlevel = 0;
        insert_implied_type_conversion(
            ee->subnodes + 1,
            t_target
        );
        t_target = ee->t;
        t_target.is_lvalue = 0;
        insert_implied_type_conversion(
            ee->subnodes + 0,
            t_target
        );
        return;
    }
    /*Pointer-pointer eq/neq.*/
    if(ee->kind == EXPR_EQ || ee->kind == EXPR_NEQ){
        if(ee->subnodes[0]->t.pointerlevel > 0)
        if(ee->subnodes[1]->t.pointerlevel > 0)
        {
            /*
               allow comparisons between arbitrary pointers.
               
               NOTE: We do conversions to our own type to remove lvalue
            */
            t_target = ee->subnodes[1]->t;
            t_target.is_lvalue = 0;
            insert_implied_type_conversion(
                ee->subnodes + 1,
                t_target
            );
            
            t_target = ee->subnodes[0]->t;
            t_target.is_lvalue = 0;
            insert_implied_type_conversion(
                ee->subnodes + 0,
                t_target
            );
            return;
        }
        if(
            (ee->subnodes[0]->t.pointerlevel > 0) ||
            (ee->subnodes[1]->t.pointerlevel > 0)
        ){ //our special little exception for comparing raw integers to pointers.
            if(ee->subnodes[0]->constint_propagator){

            } else if(ee->subnodes[1]->constint_propagator){

            } else {
                throw_type_error(
                    "Internal Error- The special exception for comparing integer literals to pointers has triggered improperly!"
                );
            }
             t_target = ee->subnodes[1]->t;
            t_target.is_lvalue = 0;
            insert_implied_type_conversion(
                ee->subnodes + 1,
                t_target
            );
            
            t_target = ee->subnodes[0]->t;
            t_target.is_lvalue = 0;
            insert_implied_type_conversion(
                ee->subnodes + 0,
                t_target
            );
            return;
            
        }
    }
    /*Assignment*/
    if(ee->kind == EXPR_ASSIGN){
        t_target = ee->subnodes[0]->t;
        if(t_target.is_lvalue == 0){
            puts(
                "<INTERNAL ERROR> Non-lvalue reached the lhs of assignment\n"
                "in the implied type conversion propagator."
            );
            throw_type_error("Internal error- check implied conversion propagator.");
        }
        t_target.is_lvalue = 0;
        //here! We want to check here!
        if(
            //we are assigning something that isn't an int constant
            (ee->subnodes[1]->constint_propagator == 0) 
            ||//OR
            //we are assigning to something that isn't a pointer.
            (ee->subnodes[0]->t.pointerlevel == 0)
        ){
            insert_implied_type_conversion(
                ee->subnodes + 1,
                t_target
            );
        } else {
            //version with an override specifically
            //for assigning an integer constant
            //to a pointer.
            insert_implied_type_conversion_ptr_assign_int_override(
                 ee->subnodes + 1,
                 t_target
             );
        }
        return;
    }
    if(ee->kind == EXPR_MOVE){
        t_target = ee->subnodes[0]->t;
        t_target.is_lvalue = 0;
        insert_implied_type_conversion(
            ee->subnodes + 1,
            t_target
        );
        insert_implied_type_conversion(
            ee->subnodes + 0,
            t_target
        );
        return;
    }
    
    if(ee->kind == EXPR_FCALL || ee->kind == EXPR_METHOD){
        for(i = 0; i < symbol_table[ee->symid]->nargs; i++){
            type qqq;
            qqq = symbol_table[ee->symid]->fargs[i][0];
            qqq.is_lvalue = 0;
            qqq.membername = NULL;
            throw_if_types_incompatible(
                ee->subnodes[i]->t,
                qqq,
                "function argument is wrong.",
                0
            );
            insert_implied_type_conversion(
                ee->subnodes+i, 
                qqq
            );
        }
        return;
    }
    if(ee->kind == EXPR_GETFNPTR){
        return;
    }
    if(ee->kind == EXPR_GETGLOBALPTR){
        return;
    }
    if(ee->kind == EXPR_CALLFNPTR){
        {
            type qqq;
            qqq = type_init();
            qqq.is_lvalue = 0;
            qqq.basetype = BASE_U8;
            qqq.pointerlevel = 1;
            insert_implied_type_conversion(
                ee->subnodes+0, 
                qqq
            );
        }
        if(symbol_table[ee->symid]->nargs)
            for(i = 0; i < symbol_table[ee->symid]->nargs; i++){
                type qqq;
                qqq = symbol_table[ee->symid]->fargs[i][0];
                qqq.is_lvalue = 0;
                qqq.membername = NULL;
                throw_if_types_incompatible(
                    ee->subnodes[1+i]->t,
                    qqq,
                    "fnptr argument is wrong.",
                    0
                );
                insert_implied_type_conversion(
                    ee->subnodes+1+i, 
                    qqq
                );
            }
        return;
    }
    if(ee->kind == EXPR_BUILTIN_CALL){
        uint64_t got_builtin_arg1_type;
        uint64_t got_builtin_arg2_type;
        uint64_t got_builtin_arg3_type;
        nargs = get_builtin_nargs(ee->symname);
        t_target = type_init();
        /*The hardest one!!*/
        if(nargs == 0) return; /*EZ!*/
             got_builtin_arg1_type = get_builtin_arg1_type(ee->symname);
         if(nargs > 1)
            got_builtin_arg2_type = get_builtin_arg2_type(ee->symname);
        if(nargs > 2)
            got_builtin_arg3_type = get_builtin_arg3_type(ee->symname);
        /*Check argument 1.*/
        if(got_builtin_arg1_type == BUILTIN_PROTO_U8_PTR){
            t_target.basetype = BASE_U8;
            t_target.pointerlevel = 1;
        }
        if(got_builtin_arg1_type == BUILTIN_PROTO_U8_PTR2){
            t_target.basetype = BASE_U8;
            t_target.pointerlevel = 2;
        }
        if(got_builtin_arg1_type == BUILTIN_PROTO_U8_PTR2){
            t_target.basetype = BASE_U8;
            t_target.pointerlevel = 2;
        }
        if(got_builtin_arg1_type == BUILTIN_PROTO_U64_PTR){
            t_target.basetype = BASE_U64;
            t_target.pointerlevel = 1;
        }		
        if(got_builtin_arg1_type == BUILTIN_PROTO_U64){
            t_target.basetype = BASE_U64;
            t_target.pointerlevel = 0;
        }
        if(got_builtin_arg1_type == BUILTIN_PROTO_I64){
            t_target.basetype = BASE_I64;
            t_target.pointerlevel = 0;
        }
        if(got_builtin_arg1_type == BUILTIN_PROTO_DOUBLE){
            t_target.basetype = BASE_F64;
            t_target.pointerlevel = 0;
        }
        if(got_builtin_arg1_type == BUILTIN_PROTO_I32){
            t_target.basetype = BASE_I32;
            t_target.pointerlevel = 0;
        }
        insert_implied_type_conversion(
            ee->subnodes+0,
            t_target
        );
        if(nargs < 2) return;
        t_target = type_init();

        if(got_builtin_arg2_type == BUILTIN_PROTO_U8_PTR){
            t_target.basetype = BASE_U8;
            t_target.pointerlevel = 1;
        }
        if(got_builtin_arg2_type == BUILTIN_PROTO_U8_PTR2){
            t_target.basetype = BASE_U8;
            t_target.pointerlevel = 2;
        }
        if(got_builtin_arg2_type == BUILTIN_PROTO_U8_PTR2){
            t_target.basetype = BASE_U8;
            t_target.pointerlevel = 2;
        }
        if(got_builtin_arg2_type == BUILTIN_PROTO_U64_PTR){
            t_target.basetype = BASE_U64;
            t_target.pointerlevel = 1;
        }		
        if(got_builtin_arg2_type == BUILTIN_PROTO_U64){
            t_target.basetype = BASE_U64;
            t_target.pointerlevel = 0;
        }
        if(got_builtin_arg2_type == BUILTIN_PROTO_I64){
            t_target.basetype = BASE_I64;
            t_target.pointerlevel = 0;
        }
        if(got_builtin_arg2_type == BUILTIN_PROTO_DOUBLE){
            t_target.basetype = BASE_F64;
            t_target.pointerlevel = 0;
        }
        if(got_builtin_arg2_type == BUILTIN_PROTO_I32){
            t_target.basetype = BASE_I32;
            t_target.pointerlevel = 0;
        }
        insert_implied_type_conversion(
            ee->subnodes+1,
            t_target
        );
        if(nargs < 3) return;
        t_target = type_init();

        if(got_builtin_arg3_type == BUILTIN_PROTO_U8_PTR){
            t_target.basetype = BASE_U8;
            t_target.pointerlevel = 1;
        }
        if(got_builtin_arg3_type == BUILTIN_PROTO_U8_PTR2){
            t_target.basetype = BASE_U8;
            t_target.pointerlevel = 2;
        }
        if(got_builtin_arg3_type == BUILTIN_PROTO_U8_PTR2){
            t_target.basetype = BASE_U8;
            t_target.pointerlevel = 2;
        }
        if(got_builtin_arg3_type == BUILTIN_PROTO_U64_PTR){
            t_target.basetype = BASE_U64;
            t_target.pointerlevel = 1;
        }		
        if(got_builtin_arg3_type == BUILTIN_PROTO_U64){
            t_target.basetype = BASE_U64;
            t_target.pointerlevel = 0;
        }
        if(got_builtin_arg3_type == BUILTIN_PROTO_I64){
            t_target.basetype = BASE_I64;
            t_target.pointerlevel = 0;
        }
        if(got_builtin_arg3_type == BUILTIN_PROTO_DOUBLE){
            t_target.basetype = BASE_F64;
            t_target.pointerlevel = 0;
        }
        if(got_builtin_arg3_type == BUILTIN_PROTO_I32){
            t_target.basetype = BASE_I32;
            t_target.pointerlevel = 0;
        }
        insert_implied_type_conversion(
            ee->subnodes+2,
            t_target
        );
        return;
    }
    throw_type_error_with_expression_enums(
        "Implied type conversion propagator fell through! Expression kind:",
        ee->kind,EXPR_BAD
    );
}

static void validate_goto_target(stmt* me, char* name){
    int64_t i;
    uint64_t j;
    uint64_t scopediff_sofar = 0;
    uint64_t vardiff_sofar = 0;
    for(i=nscopes-1;i>=0;i--){
        stmt* stmtlist;
        stmtlist = scopestack[i]->stmts;
        for(j=0;j<scopestack[i]->nstmts;j++){
            if(stmtlist[j].kind == STMT_LABEL)
                if(streq(stmtlist[j].referenced_label_name,name))
                {
                    //assign it!
                    me->referenced_loop = ((stmt*)stmtlist) + j;
                    //assign scopediff
                    me->goto_scopediff = scopediff_sofar;
                    me->goto_vardiff = vardiff_sofar;
                    me->goto_where_in_scope = j;
                    return;
                }
        }
        scopediff_sofar++;
        vardiff_sofar += scopestack[i]->nsyms;
    }
    puts("~~~~~~~VALIDATOR ERROR~~~~~~~~~~~~~~~\nGoto uses out-of-sequence or non-existent jump target:");
    puts(name);
    puts("Note that Seabass does NOT allow you to jump into a scope. Goto may only be within the same scope,");
    puts("or 'breaking out' of one or more scopes into a higher one.");
    validator_exit_err();
}

static void assign_scopediff_vardiff(stmt* me, scope* jtarget_scope){
    int64_t i;
    uint64_t scopediff_sofar = 0;
    uint64_t vardiff_sofar = 0;
    for(i=nscopes-1;i>=0;i--){
        if(jtarget_scope == scopestack[i]){
            //assign scopediff
            me->goto_scopediff = scopediff_sofar;
            me->goto_vardiff = vardiff_sofar;
            return;
        }
        scopediff_sofar++;
        vardiff_sofar += scopestack[i]->nsyms;
    }
    // if(is_return){
    //     me->goto_scopediff = scopediff_sofar;
    //     me->goto_vardiff = vardiff_sofar;
    //     return;
    // }
    puts("<INTERNAL ERROR> Could not assign scopediff/vardiff for jumping statement.");
    validator_exit_err();
}
static void assign_scopediff_vardiff_return(stmt* me){
    int64_t i;
    uint64_t scopediff_sofar = 0;
    uint64_t vardiff_sofar = 0;
    for(i=nscopes-1;i>=0;i--){
        scopediff_sofar++;
        vardiff_sofar += scopestack[i]->nsyms;
    }
    me->goto_scopediff = scopediff_sofar;
    me->goto_vardiff = vardiff_sofar;
    return;
}

//Now it's recursive, so is "walk" really a good name for this?
static void walk_assign_lsym_gsym(scope* current_scope, int phase){
    int64_t i;
    int64_t j;
    //PHASE 0- Do everything
    //PHASE 1- Only do stuff which touches 
    //current_scope = symbol_table[active_function]->fbody;
    stmt* stmtlist;
    //current_scope->walker_point = 0;
    i = 0;
    uint64_t WORD_BASE;
    uint64_t SIGNED_WORD_BASE;
    uint64_t FLOAT_BASE;
    (void)FLOAT_BASE;
    if(symbol_table[active_function]->is_codegen){
        WORD_BASE = BASE_U64;
        SIGNED_WORD_BASE = BASE_I64;
        FLOAT_BASE = BASE_F64;
    } else {
        WORD_BASE = TARGET_WORD_BASE;
        SIGNED_WORD_BASE = TARGET_WORD_SIGNED_BASE;
        FLOAT_BASE = TARGET_MAX_FLOAT_TYPE;
    }
    (void)WORD_BASE;
    //
    for(i = 0; i <(int64_t)current_scope->nstmts; i++) {
        stmtlist = current_scope->stmts;
        curr_stmt = stmtlist + i;

        //on both phases, we must perform validation of goto...
        if(stmtlist[i].kind == STMT_GOTO){

            scopestack_push(current_scope);
                validate_goto_target(stmtlist + i, stmtlist[i].referenced_label_name);
            scopestack_pop();
        }
        //PHASE 1 EXCLUSIVES~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
        //(That's the SECOND pass....)
        if(phase == 1){
            //in phase 1, we perform safety checks on expression statements...
            if(stmtlist[i].kind == STMT_EXPR){
                j=0;
                //this function needs to "see" the current scope...
                scopestack_push(current_scope);
                    assign_lsym_gsym(stmtlist[i].expressions[j]);
                    propagate_types(stmtlist[i].expressions[j]);
                    validate_function_argument_passing(stmtlist[i].expressions[j]);
                    validate_codegen_safety(stmtlist[i].expressions[j]);
                    propagate_implied_type_conversions(stmtlist[i].expressions[j]);

                    //Repeat for absolute safety
                    //propagate_types(stmtlist[i].expressions[j]);
                    validate_function_argument_passing(stmtlist[i].expressions[j]);
                scopestack_pop();
            }
            else if(
                stmtlist[i].kind == STMT_IF     ||
                stmtlist[i].kind == STMT_ELIF
            )
            {
                int64_t else_chain_follower = i + 1;
                for(;else_chain_follower < (int64_t)current_scope->nstmts; else_chain_follower++){
                    /*terminating condition 1: is not part of the else chain.*/
                    if(
                        stmtlist[else_chain_follower].kind != STMT_ELIF &&
                        stmtlist[else_chain_follower].kind != STMT_ELSE
                    ) break;
                    /*terminating condition 2: is else. else is always the last one in an else chain.*/
                    if(stmtlist[else_chain_follower].kind == STMT_ELSE){
                        else_chain_follower++;  /*the next statement, if it exists, is the one we should be executing.*/
                        break;
                    }
                }
                /*Note that this may actually be past the end*/
                stmtlist[i].goto_where_in_scope = else_chain_follower;
            }
        }
        //PHASE 0~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
        if(phase == 0){
            //Don't validate STMT_EXPR on the first pass- we pick it up on the second.
            if(
                stmtlist[i].nexpressions > 0 &&
                stmtlist[i].kind != STMT_EXPR
            )
            {
                /*assign lsym and gsym right here.*/
                for(j = 0; j < (int64_t)stmtlist[i].nexpressions; j++){
                    //this function needs to "see" the current scope...
                    scopestack_push(current_scope);
                        assign_lsym_gsym(stmtlist[i].expressions[j]);
                        propagate_types(stmtlist[i].expressions[j]);
                        validate_function_argument_passing(stmtlist[i].expressions[j]);
                        validate_codegen_safety(stmtlist[i].expressions[j]);
                        propagate_implied_type_conversions(stmtlist[i].expressions[j]);

                        //Repeat for absolute safety
                        //propagate_types(stmtlist[i].expressions[j]);
                        validate_function_argument_passing(stmtlist[i].expressions[j]);
                    scopestack_pop();
                }
            }
            //DO NOT put else here...
            if(
                stmtlist[i].kind == STMT_SWITCH
            )
            {
                type qq = ((expr_node*)stmtlist[i].expressions[0])->t;
                if(
                    (qq.pointerlevel > 0) ||
                    (qq.basetype == BASE_STRUCT) ||
                    (qq.basetype == BASE_VOID) ||
                    (qq.basetype == BASE_F64) ||
                    (qq.basetype == BASE_F32) 
                )
                throw_type_error("Switch statement has non-integer expression.");
                qq = type_init();
                //Notice- We do nothing here to validate where the switch
                //is going. We're only dealing with its expression....
                qq.basetype = SIGNED_WORD_BASE;
                insert_implied_type_conversion((expr_node**)stmtlist[i].expressions, qq);
            }
            else if(stmtlist[i].kind == STMT_ASM){
                symbol_table[active_function]->is_impure_globals_or_asm = 1;
                symbol_table[active_function]->is_impure = 1;
                if(symbol_table[active_function]->is_codegen != 0){
                    puts("VALIDATION ERROR!");
                    puts("asm blocks may not exist in codegen functions.");
                    puts("This function:");
                    puts(symbol_table[active_function]->name);
                    puts("Was declared 'codegen' so you cannot use 'asm' blocks in it.");
                    validator_exit_err();
                }
                if(symbol_table[active_function]->is_pure > 0){
                    puts("VALIDATION ERROR!");
                    puts("asm blocks may not exist in pure functions.");
                    puts("This function:");
                    puts(symbol_table[active_function]->name);
                    puts("Was declared 'pure' so you cannot use 'asm' blocks in it.");
                    validator_exit_err();
                }
            }
            else if(stmtlist[i].kind == STMT_IF){
                type qq = ((expr_node*)stmtlist[i].expressions[0])->t;
                if(
                    (qq.pointerlevel > 0) ||
                    (qq.basetype == BASE_STRUCT) ||
                    (qq.basetype == BASE_VOID) ||
                    (qq.basetype == BASE_F64) ||
                    (qq.basetype == BASE_F32) 
                )
                throw_type_error("If statement has non-integer conditional expression..");
                qq = type_init();
                //qq.basetype = BASE_I64;
                qq.basetype = SIGNED_WORD_BASE;
                insert_implied_type_conversion((expr_node**)stmtlist[i].expressions, qq);
            }
            else if(
                stmtlist[i].kind == STMT_ELSE ||
                stmtlist[i].kind == STMT_ELIF
            ){
                if(i == 0){
                    puts("Else/Elif at the beginning of a scope? (No preceding if?)");
                    validator_exit_err();
                }
                if(stmtlist[i-1].kind != STMT_ELIF &&
                    stmtlist[i-1].kind != STMT_IF){
                    puts("Else/Elif without a preceding if/else?");
                    validator_exit_err();
                }
                if(stmtlist[i].kind == STMT_ELIF){
                    type qq = ((expr_node*)stmtlist[i].expressions[0])->t;
                    if(qq.pointerlevel > 0)
                        throw_type_error("Cannot branch on pointer in elif stmt.");
                    if(qq.basetype == BASE_VOID)
                        throw_type_error("Cannot branch on void in elif stmt.");
                    if(
                        (qq.pointerlevel > 0) ||
                        (qq.basetype == BASE_STRUCT) ||
                        (qq.basetype == BASE_VOID) ||
                        (qq.basetype == BASE_F64) ||
                        (qq.basetype == BASE_F32) 
                    )
                        throw_type_error("elif statement has non-integer conditional expression..");
                    qq = type_init();
                    qq.basetype = SIGNED_WORD_BASE;
                    insert_implied_type_conversion((expr_node**)stmtlist[i].expressions, qq);
                }
            }
            else if(stmtlist[i].kind == STMT_RETURN)
            {
                if((expr_node*)stmtlist[i].expressions[0]){
                    type qq = ((expr_node*)stmtlist[i].expressions[0])->t;
                    type pp = symbol_table[active_function]->t;
                    pp.is_function = 0;
                    pp.funcid = 0;
                    throw_if_types_incompatible(pp,qq,"Return statement must have compatible type.",0);
                    insert_implied_type_conversion((expr_node**)stmtlist[i].expressions, pp);
                }
                scopestack_push(current_scope);
                    assign_scopediff_vardiff_return(stmtlist+i);
                scopestack_pop();
            }
            else if(stmtlist[i].kind == STMT_TAIL)
            {
                if(symbol_table[active_function]->is_pure){
                    if(symbol_table[stmtlist[i].symid]->is_pure == 0){
                        puts("Validator Error!");
                        puts("Tail statement in function:");
                        puts(symbol_table[active_function]->name);
                        puts("Tails-off to a function not explicitly defined as pure.");
                        validator_exit_err();
                    }
                }
                /*compare function arguments...*/
                for(j = 0; j < (int64_t)symbol_table[active_function]->nargs; j++)
                    throw_if_types_incompatible(
                        symbol_table[active_function]->fargs[j][0],
                        symbol_table[stmtlist[i].symid]->fargs[j][0],
                        "Tail to function with non-matching function prototype.",
                        1 //basetypes must be identical
                    );
                scopestack_push(current_scope);
                    assign_scopediff_vardiff_return(stmtlist+i);
                scopestack_pop();
            }

        } //EOF PHASE 0 EXCLUSIVES
        //These must be done in both phases...
        if(stmtlist[i].kind == STMT_FOR){
            //this portion is Phase 0 only- dealing with the implied type conversion...
            if(phase == 0){
                type qq = ((expr_node*)stmtlist[i].expressions[1])->t;
                if(
                    (qq.pointerlevel > 0) ||
                    (qq.basetype == BASE_STRUCT) ||
                    (qq.basetype == BASE_VOID) ||
                    (qq.basetype == BASE_F64) ||
                    (qq.basetype == BASE_F32) 
                )
                throw_type_error("For statement has non-integer conditional expression..");
                qq = type_init();
                qq.basetype = SIGNED_WORD_BASE;
                    insert_implied_type_conversion((expr_node**)stmtlist[i].expressions+1, qq);
            }
            //...but we always push onto the loopstack...
            loopstack_push(stmtlist + i);
        }else if(stmtlist[i].kind == STMT_WHILE){
            //this portion is Phase 0 only- dealing with the implied type conversion...
            if(phase == 0){
                type qq = ((expr_node*)stmtlist[i].expressions[0])->t;
                if(
                    (qq.pointerlevel > 0) ||
                    (qq.basetype == BASE_STRUCT) ||
                    (qq.basetype == BASE_VOID) ||
                    (qq.basetype == BASE_F64) ||
                    (qq.basetype == BASE_F32) 
                )
                throw_type_error("While statement has non-integer conditional expression..");
                qq = type_init();
                qq.basetype = SIGNED_WORD_BASE;
                insert_implied_type_conversion((expr_node**)stmtlist[i].expressions, qq);
            }
            //...but we always push onto the loopstack...
            loopstack_push(stmtlist + i);
        }else if(
            //these have to be calculated twice because the stmt references may be invalidated by ctor/dtor insertion.....
            stmtlist[i].kind == STMT_CONTINUE ||
            stmtlist[i].kind == STMT_BREAK
        ){
            stmtlist[i].referenced_loop = loopstack[nloops-1];
            scopestack_push(current_scope);
                assign_scopediff_vardiff(
                    stmtlist+i,
                    stmtlist[i].referenced_loop->whereami
                );
            scopestack_pop();
        }


        if(stmtlist[i].myscope){
            scopestack_push(current_scope);
                walk_assign_lsym_gsym(stmtlist[i].myscope, phase);
            scopestack_pop(current_scope);
            if(
                stmtlist[i].kind == STMT_FOR ||
                stmtlist[i].kind == STMT_WHILE
            ){
                loopstack_pop();
            }
        }
    }
}


extern stmt* parser_push_statement_nop();
void scope_insert_nops(scope* me, unsigned pos, unsigned n);
//All this has to do is insert the required destructors 

typedef struct {
    int64_t ctors_needed;
    int64_t dtors_needed;
    int64_t stmts_needed;
    char* constructible_arr;
    char* destructible_arr;
}cache_ctor_dtor_data;

static cache_ctor_dtor_data ctor_dtor_stack[0x10000];
uint64_t ctor_dtor_stackptr = 0;

static inline void ctor_dtor_stack_push(){
    ctor_dtor_stack[ctor_dtor_stackptr].ctors_needed = 0;
    ctor_dtor_stack[ctor_dtor_stackptr].dtors_needed = 0;
    ctor_dtor_stack[ctor_dtor_stackptr].stmts_needed = 0;
    ctor_dtor_stack[ctor_dtor_stackptr].constructible_arr = 0;
    ctor_dtor_stack[ctor_dtor_stackptr].destructible_arr = 0;
    ctor_dtor_stackptr++;
}
static inline void ctor_dtor_stack_pop(){
    if(ctor_dtor_stack[ctor_dtor_stackptr-1].constructible_arr)
        free(ctor_dtor_stack[ctor_dtor_stackptr-1].constructible_arr);
    ctor_dtor_stack[ctor_dtor_stackptr-1].constructible_arr = 0;
    
    if(ctor_dtor_stack[ctor_dtor_stackptr-1].destructible_arr)
       free(ctor_dtor_stack[ctor_dtor_stackptr-1].destructible_arr); 
    
    ctor_dtor_stack[ctor_dtor_stackptr-1].destructible_arr = 0;
    
    ctor_dtor_stackptr--;
}

static void recurse_insert_dtors_before_return(
    scope* scope_to_work_on,
    symdecl* syms,
    uint64_t nsyms,
    char* destructible_arr,
    int64_t dtors_needed
){
    int64_t i;
    int64_t j;
    int64_t iter;
    if(dtors_needed == 0) return;
    stmt* stmtlist = scope_to_work_on->stmts;
    for(i = 0; i < (int64_t)scope_to_work_on->nstmts;i++){
        //find a return statement...
        stmt* cur_stmt = stmtlist + i;
        if(cur_stmt->kind == STMT_RETURN || cur_stmt->kind == STMT_TAIL){
            scope_insert_nops(scope_to_work_on, i, dtors_needed);
            stmtlist = scope_to_work_on->stmts;
            /*Write our destructors here!*/
            iter = 0;
            for(j = nsyms-1; j >= 0; j--){
                //TODO
                if(destructible_arr[j] == 0) continue;
                stmt* me = stmtlist + i + iter;
                iter++;
                *me =  stmt_init();
                me->whereami = scope_to_work_on; /*The scope to look in!*/
                me->filename = "OBJECT_ORIENTED_PROGRAMMING_INTERNAL_AUTOGENERATED";
                me->linenum = 0;
                me->colnum = 0;
                me->kind = STMT_EXPR;
                me->nexpressions = 1;
                expr_node* parent_method_expr = c_allocX(sizeof(expr_node));
                expr_node* child_node_self_sym = c_allocX(sizeof(expr_node));
                child_node_self_sym->symname = strdup(syms[j].name);
                child_node_self_sym->kind = EXPR_SYM;
                
                me->expressions[0] = parent_method_expr;
                parent_method_expr->kind = EXPR_METHOD;
                parent_method_expr->subnodes[0] = child_node_self_sym;
                //before_f->subnodes[0] = f; /*A method is (secretly) passing "this" as the first argument.*/
                 parent_method_expr->method_name = strdup("dtor");
                parent_method_expr->symname = NULL;
                 //f->is_function = 1; //HUH?!?! Regardless, we replicate it...
                 child_node_self_sym->is_function = 1;
            }
            i += dtors_needed;

        } else if(cur_stmt->myscope){
            recurse_insert_dtors_before_return(
                cur_stmt->myscope,
                syms, nsyms, 
                destructible_arr,
                dtors_needed
            );
        }
    }
}

static void recurse_insert_dtors_before_gotos(
    scope* scope_to_work_on,
    symdecl* syms,
    uint64_t nsyms,
    char* destructible_arr,
    int64_t dtors_needed,
    int64_t scopediff //minimum scope diff....
){
    int64_t i;
    int64_t j;
    int64_t iter;
    if(dtors_needed == 0) return;
    stmt* stmtlist = scope_to_work_on->stmts;
    for(i = 0; i < (int64_t)scope_to_work_on->nstmts;i++){
        //find a return statement...
        stmt* cur_stmt = stmtlist + i;
        if(
            cur_stmt->kind == STMT_GOTO || 
            cur_stmt->kind == STMT_CONTINUE ||
            cur_stmt->kind == STMT_BREAK
        ){
            if(
                cur_stmt->goto_scopediff >= scopediff //has the minimum scopediff...
            )
            {
                scope_insert_nops(scope_to_work_on, i, dtors_needed);
                
                stmtlist = scope_to_work_on->stmts;
                /*Write our destructors here!*/
                iter = 0;
                for(j = nsyms-1; j >= 0; j--){
                    //TODO
                    if(destructible_arr[j] == 0) continue;
                    stmt* me = stmtlist + i + iter;
                    iter++;
                    *me =  stmt_init();
                    me->whereami = scope_to_work_on; /*The scope to look in!*/
                    me->filename = "OBJECT_ORIENTED_PROGRAMMING_INTERNAL_AUTOGENERATED";
                    me->linenum = 0;
                    me->colnum = 0;
                    me->kind = STMT_EXPR;
                    me->nexpressions = 1;
                    expr_node* parent_method_expr = c_allocX(sizeof(expr_node));
                    expr_node* child_node_self_sym = c_allocX(sizeof(expr_node));
                    child_node_self_sym->symname = strdup(syms[j].name);
                    child_node_self_sym->kind = EXPR_SYM;
                    
                    me->expressions[0] = parent_method_expr;
                    parent_method_expr->kind = EXPR_METHOD;
                    parent_method_expr->subnodes[0] = child_node_self_sym;
                    //before_f->subnodes[0] = f; /*A method is (secretly) passing "this" as the first argument.*/
                     parent_method_expr->method_name = strdup("dtor");
                    parent_method_expr->symname = NULL;
                     //f->is_function = 1; //HUH?!?! Regardless, we replicate it...
                     child_node_self_sym->is_function = 1;
                }
                i += dtors_needed;
            }
        }
        else if(cur_stmt->myscope){
            recurse_insert_dtors_before_gotos(
                cur_stmt->myscope,
                syms, nsyms, 
                destructible_arr,
                dtors_needed,
                scopediff+1
            );
        }
    }
}

static void walk_insert_ctor_dtor(scope* current_scope){
    int64_t i;

    //current_scope = symbol_table[active_function]->fbody;
    stmt* stmtlist;
    //current_scope->walker_point = 0;
    i = 0;
    //
    stmtlist = current_scope->stmts;
    symdecl* syms = current_scope->syms;
    uint64_t nsyms = current_scope->nsyms;
    {
        char* constructible_arr = malloc(nsyms);
        char* destructible_arr = malloc(nsyms);
        ctor_dtor_stack_push();
        ctor_dtor_stack[ctor_dtor_stackptr-1].constructible_arr = constructible_arr;
        ctor_dtor_stack[ctor_dtor_stackptr-1].destructible_arr = destructible_arr;
        memset(constructible_arr, 0, nsyms);
        memset(destructible_arr, 0, nsyms);
        int64_t j;
        int64_t stmts_needed = 0;
        int64_t ctors_needed = 0;
        int64_t dtors_needed = 0;
        int64_t nstmts_before = 0;
        int64_t beginning_of_dtors = 0;
        for(j = 0; j < (int64_t)nsyms; j++){
            if(syms[j].t.basetype != BASE_STRUCT) continue;
            if(syms[j].t.pointerlevel != 0) continue; //we don't bother for pointers to structs.
            if(syms[j].t.arraylen != 0) continue; //we don't bother for arrays of them, either.
            update_metadata(syms[j].t.structid); //we update the metadata. Then we check...
            constructible_arr[j] = (oop_metadata[syms[j].t.structid].ctor_id != -1);
            destructible_arr[j] = (oop_metadata[syms[j].t.structid].dtor_id != -1);
            
            if(constructible_arr[j]) {stmts_needed++;ctors_needed++;}
            if(destructible_arr[j]) {stmts_needed++;dtors_needed++;}
        }
        ctor_dtor_stack[ctor_dtor_stackptr-1].ctors_needed = ctors_needed;
        ctor_dtor_stack[ctor_dtor_stackptr-1].dtors_needed = dtors_needed;
        ctor_dtor_stack[ctor_dtor_stackptr-1].stmts_needed = stmts_needed;
        if(stmts_needed == 0) goto just_skip_the_part_where_we_do_the_constructor_destructor_insertion;
        nstmts_before = current_scope->nstmts;
        scopestack_push(current_scope);
            for(j = 0; j < stmts_needed; j++){
                parser_push_statement_nop();
            }
        scopestack_pop();
        stmtlist = current_scope->stmts;
        //according to the C documentation, this is fine...
        memmove(stmtlist + ctors_needed, stmtlist, sizeof(stmt) * nstmts_before);
        beginning_of_dtors = ctors_needed + nstmts_before; //skip those other statements!
        int64_t iter = 0;
        for(j = 0; j < (int64_t)nsyms; j++){
            //TODO
            if(constructible_arr[j] == 0) continue;
            stmt* me = stmtlist + iter;
            iter++;
            *me =  stmt_init();
            me->whereami = current_scope; /*The scope to look in!*/
            me->filename = "OBJECT_ORIENTED_PROGRAMMING_INTERNAL_AUTOGENERATED";
            me->linenum = 0;
            me->colnum = 0;
            me->kind = STMT_EXPR;
            me->nexpressions = 1;
            expr_node* parent_method_expr = c_allocX(sizeof(expr_node));
            expr_node* child_node_self_sym = c_allocX(sizeof(expr_node));
            child_node_self_sym->symname = strdup(syms[j].name);
            child_node_self_sym->kind = EXPR_SYM;
            
            me->expressions[0] = parent_method_expr;
            parent_method_expr->kind = EXPR_METHOD;
            parent_method_expr->subnodes[0] = child_node_self_sym;
            //before_f->subnodes[0] = f; /*A method is (secretly) passing "this" as the first argument.*/
             parent_method_expr->method_name = strdup("ctor");
            parent_method_expr->symname = NULL;
            //f->is_function = 1; //HUH?!?! Regardless, we replicate it...
             child_node_self_sym->is_function = 1;
        }

        iter = 0;
        for(j = nsyms-1; j >= 0; j--){
            //TODO
            if(destructible_arr[j] == 0) continue;
            stmt* me = stmtlist + iter + beginning_of_dtors;
            iter++;
            *me =  stmt_init();
            me->whereami = current_scope; /*The scope to look in!*/
            me->filename = "OBJECT_ORIENTED_PROGRAMMING_INTERNAL_AUTOGENERATED";
            me->linenum = 0;
            me->colnum = 0;
            me->kind = STMT_EXPR;
            me->nexpressions = 1;
            expr_node* parent_method_expr = c_allocX(sizeof(expr_node));
            expr_node* child_node_self_sym = c_allocX(sizeof(expr_node));
            child_node_self_sym->symname = strdup(syms[j].name);
            child_node_self_sym->kind = EXPR_SYM;
            
            me->expressions[0] = parent_method_expr;
            parent_method_expr->kind = EXPR_METHOD;
            parent_method_expr->subnodes[0] = child_node_self_sym;
            //before_f->subnodes[0] = f; /*A method is (secretly) passing "this" as the first argument.*/
             parent_method_expr->method_name = strdup("dtor");
            parent_method_expr->symname = NULL;
             //f->is_function = 1; //HUH?!?! Regardless, we replicate it...
             child_node_self_sym->is_function = 1;
        }
        just_skip_the_part_where_we_do_the_constructor_destructor_insertion:;
    }
    for(i = 0; i < (int64_t)current_scope->nstmts; i++){
        if(stmtlist[i].myscope){
            /*ctx switch.*/
            scopestack_push(current_scope);
                walk_insert_ctor_dtor(stmtlist[i].myscope);
            scopestack_pop();
            continue;
        }
    }
    if(ctor_dtor_stack[ctor_dtor_stackptr-1].dtors_needed){
        recurse_insert_dtors_before_return(
            current_scope,
            current_scope->syms,
            current_scope->nsyms,
            ctor_dtor_stack[ctor_dtor_stackptr-1].destructible_arr,
            ctor_dtor_stack[ctor_dtor_stackptr-1].dtors_needed
        );
    }
    ctor_dtor_stack_pop();

    return;
}

static void walk_insert_ctor_dtor_pt2(scope* current_scope){
    //TODO
    int64_t i;

    //current_scope = symbol_table[active_function]->fbody;
    stmt* stmtlist;
    i = 0;
    stmtlist = current_scope->stmts;
    symdecl* syms = current_scope->syms;
    uint64_t nsyms = current_scope->nsyms;
    {
        
        char* constructible_arr = malloc(nsyms);
        char* destructible_arr = malloc(nsyms);
        ctor_dtor_stack_push();
        ctor_dtor_stack[ctor_dtor_stackptr-1].constructible_arr = constructible_arr;
        ctor_dtor_stack[ctor_dtor_stackptr-1].destructible_arr = destructible_arr;
        memset(constructible_arr, 0, nsyms);
        memset(destructible_arr, 0, nsyms);
        int64_t j;
        int64_t stmts_needed = 0;
        int64_t ctors_needed = 0;
        int64_t dtors_needed = 0;
        //int64_t nstmts_before = 0;
        //int64_t beginning_of_dtors = 0;
        for(j = 0; j < (int64_t)nsyms; j++){
            if(syms[j].t.basetype != BASE_STRUCT) continue;
            if(syms[j].t.pointerlevel != 0) continue; //we don't bother for pointers to structs.
            if(syms[j].t.arraylen != 0) continue; //we don't bother for arrays of them, either.
            update_metadata(syms[j].t.structid); //we update the metadata. Then we check...
            constructible_arr[j] = (oop_metadata[syms[j].t.structid].ctor_id != -1);
            destructible_arr[j] = (oop_metadata[syms[j].t.structid].dtor_id != -1);
            
            if(constructible_arr[j]) {stmts_needed++;ctors_needed++;}
            if(destructible_arr[j]) {stmts_needed++;dtors_needed++;}
        }
        ctor_dtor_stack[ctor_dtor_stackptr-1].ctors_needed = ctors_needed;
        ctor_dtor_stack[ctor_dtor_stackptr-1].dtors_needed = dtors_needed;
        ctor_dtor_stack[ctor_dtor_stackptr-1].stmts_needed = stmts_needed;
        if(stmts_needed == 0) goto just_skip_the_part_where_we_do_the_constructor_destructor_insertion;
        //nstmts_before = current_scope->nstmts;
        just_skip_the_part_where_we_do_the_constructor_destructor_insertion:;
    }
    //
    for(i = 0; i < (int64_t)current_scope->nstmts; i++){
        if(stmtlist[i].myscope){
            /*ctx switch.*/
            scopestack_push(current_scope);
                walk_insert_ctor_dtor_pt2(stmtlist[i].myscope);
            scopestack_pop();
        }
    }
    {
        if(ctor_dtor_stack[ctor_dtor_stackptr-1].dtors_needed){
            recurse_insert_dtors_before_gotos(
                current_scope,
                current_scope->syms,
                current_scope->nsyms,
                ctor_dtor_stack[ctor_dtor_stackptr-1].destructible_arr,
                ctor_dtor_stack[ctor_dtor_stackptr-1].dtors_needed,
                1 //if it traverses above us even one level, this requires our attention...
                //we use >= for the test, so it's nothing special....
            );
        }
        //pop what we pushed...
        ctor_dtor_stack_pop();
        i++;
        return;
    }
}



void validate_function(symdecl* funk){
    long i;

    if(funk->t.is_function == 0)
    {
        puts("INTERNAL VALIDATOR ERROR: Passed non-function.");
        validator_exit_err();
    }
    if(nscopes > 0 || nloops > 0){
        puts("INTERNAL VALIDATOR ERROR: Bad scopestack or loopstack.");
        validator_exit_err();
    }
    if(nsymbols == 0){
        puts("INTERNAL VALIDATOR ERROR: There are no symbols, therefore, this function cannot be validated!");
        exit(1);
    }
    for(i = nsymbols-1; 1; i--){
        if(i == -1){
            puts("INTERNAL VALIDATOR ERROR: The function is not in the symbol list!");
            exit(1);
        }
        if((symbol_table+i)[0] == funk){
            active_function = i; break;
        }
    }
    if(nsymbols == 0){
        puts("INTERNAL VALIDATOR ERROR: The function is not in the symbol list!");
        exit(1);
    }

    n_discovered_labels = 0;
    /*
        pre-allocate discovered labels space...
    */
    if(discovered_labels == NULL){
        discovered_labels_capacity = 1024 * 1024;
        discovered_labels = malloc(1024 * 1024 * sizeof(char*));
    }
    
    
    walk_insert_ctor_dtor(funk->fbody);
    
    check_label_declarations(funk->fbody);
    walk_assign_lsym_gsym(funk->fbody,0);
    /*
        Based on scopediffs, we need to insert more complex destructor calls...
    */
    walk_insert_ctor_dtor_pt2(funk->fbody);
    check_switches(funk->fbody);
    //This must be repeated, but in the second phase...
    walk_assign_lsym_gsym(funk->fbody,1);
    //
    
    optimize_fn(funk);
    /*
        TODO: 
        Fix big: You can shadow variables by accident...
    */
    
    if(nscopes > 0){
        puts("INTERNAL VALIDATOR ERROR: Bad scopestack after walk_assign_lsym_gsym");
        validator_exit_err();
    }	
    if(nloops > 0){
        puts("INTERNAL VALIDATOR ERROR: Bad loopstack after walk_assign_lsym_gsym");
        validator_exit_err();
    }
    /*DONE: Assign loop pointers to continue and break statements. It also does goto.*/


    n_discovered_labels = 0;
}

