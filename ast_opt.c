
#include "stringutil.h"
#include "targspecifics.h"

#include "libmin.h"
#include "parser.h"
#include "data.h"
#include "metaprogramming.h"
#include "astexec.h"
#include "ast_opt.h"

//
static size_t memneeded = 0;
static char* bbuf = 0;
static const size_t ALGN = 32;
static void plan_memory(size_t amt){
    amt = (amt + (ALGN-1)) % ALGN;
    memneeded = memneeded + amt;
}
static void plan_string(char* s){
    plan_memory(strlen(s)+1);
}

static void plan_lvar(symdecl* s){
    //TODO
    plan_memory(sizeof(symdecl));
    plan_memory(strlen(s->name)+1);//the name of the variable will be stored...
}

static void plan_expr_node(expr_node* e){
    plan_memory(sizeof(expr_node));
    if(e->kind == EXPR_STRINGLIT || e->kind == EXPR_BUILTIN_CALL)
        plan_string(e->symname);
    for(unsigned long i = 0; i < MAX_FARGS; i++)
        if(e->subnodes[i])
            plan_expr_node(e->subnodes[i]);
    //TODO
}
static void plan_scope(scope* s);
static void plan_stmt(stmt* s){
    plan_memory(sizeof(stmt));
    //TODO
    for(unsigned long i = 0; i < s->nexpressions; i++)
        plan_expr_node(s->expressions[i]);
    if(s->myscope)
        plan_scope(s->myscope);
    
}

static void plan_scope(scope* s){
    plan_memory(sizeof(scope));
    for(unsigned long i = 0; i < s->nsyms; i++){
        plan_lvar(s->syms + i);
    }
    for(unsigned long i = 0; i < s->nstmts; i++){
        plan_stmt(s->stmts + i);
    }
}

void optimize_fn(symdecl* ss){
    memneeded = 0;
    bbuf = 0;
    //we don't eat its name...
    if(ss->t.is_function == 0 || ss->is_codegen == 0) return; //this is not a codegen function...
    for(unsigned long i = 0; i < ss->nargs; i++)
        plan_memory(sizeof(type));
    plan_scope(ss->fbody);
    //TODO: 
}

