
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
//our big buffer!
static void* bbuf = 0;
static const size_t ALGN = 8; //memory alignment...
typedef struct{
    void* p;
    size_t t;
} ptr_sizet_pair;
static ptr_sizet_pair* ptr_cache = 0;
static size_t npairs = 0;

//register a pointer to be allocated somewhere at/ahead of the
//current position. It may also point to the same place...
static void push_pointer_ahead(void* p, size_t depth){
    ptr_sizet_pair pr;
    pr.p = p;
    pr.t = memneeded + depth;
    npairs++;
    ptr_cache = realloc(ptr_cache, sizeof(ptr_sizet_pair) * (npairs));
    ptr_cache[npairs-1] = pr;
}

//register a pointer in the system. It will
//be allocated at the current position....
static void push_pointer(void* p){
    push_pointer_ahead(p,0);
}


//plan a single chunk of memory...
//you may not use this to allocate array elements!
static void plan_memory(size_t amt){
    amt = (amt + (ALGN-1)) & ~(size_t)(ALGN-1);
    memneeded = memneeded + amt;
}

static void push_plan(void* p, size_t amt){
    push_pointer(p);
    plan_memory(amt);
}

//plan an array of things...
static void plan_array(void* p, size_t elemsz, size_t n){
    for(unsigned long i = 0; i < n; i++)
    {
        push_pointer_ahead(p + elemsz * i, elemsz * i);
    }
    plan_memory(elemsz * n);
}
//plan an array of things that will have two elemsizes...
static void plan_array_new(void* p, size_t elemsz,size_t elemsz_new, size_t n){
    for(unsigned long i = 0; i < n; i++)
    {   
        //we must register a pointer for every element,
        //and also say that it points to something inside the new thing...
        push_pointer_ahead(p + elemsz * i, elemsz_new * i);
    }
    plan_memory(elemsz_new * n);
}


static size_t lookup_pointer(void* p){
    for(unsigned long i = 0; i < npairs; i++){
        if(p == ptr_cache[i].p)
            return ptr_cache[i].t;
    }
    return ~(size_t)0;
}

//decode a pointer from the original AST into the new, compacted one...
static void* pdecode(void* p){
    size_t q = lookup_pointer(p);
    if(q == ~(size_t)0) return NULL;
    return bbuf + q;
}

#define REP(A) (A) = pdecode(A);

#ifdef COMPILER_CLEANS_UP

#define FREP(A) free(A); (A) = pdecode(A);

#else

#define FREP(A) (A) = pdecode(A);

#endif

static void plan_string(char* s){
    push_plan(s, strlen(s)+1);
}
static void write_string(char* s){
    memcpy(pdecode(s),s,strlen(s)+1);
}
static void plan_lvars(symdecl* s, unsigned long n){
    plan_array(s, sizeof(symdecl_astexec), n);
}
static void write_lvars(symdecl* s, unsigned long n){
    void* q = pdecode(s);
    memcpy(q,s,n * sizeof(symdecl_astexec));
    for(unsigned long i = 0; i < n; i++){
#ifdef COMPILER_CLEANS_UP
        free(s[i].name); 
#endif
        memcpy((q + i * sizeof(symdecl_astexec)),s + i, sizeof(symdecl_astexec));
        
    }
}

//Does not do any allocation of itself, because
//expr_node appears both in an array of pointers,
//and an array of objects (inside itself, actually...)
static void plan_expr_node(expr_node* s);
static void write_expr_node(expr_node* s);

static void plan_expr_node(expr_node* s){
    //plan ourselves..
    push_plan(s, sizeof(expr_node_astexec));
    if(s->symname)
        plan_string(s->symname);
    if(s->method_name)
        plan_string(s->method_name);
    for(unsigned long i = 0; i < MAX_FARGS; i++)
        if(s->subnodes[i])
            plan_expr_node(s->subnodes[i]);
}
static void write_expr_node(expr_node* s_old){
    expr_node* s = pdecode(s_old);
    memcpy(s,s_old, sizeof(expr_node_astexec));
    if(s->symname){
        write_string(s->symname);
        FREP(s->symname);
    }
    if(s->method_name){
        write_string(s->method_name);
        FREP(s->method_name);
    }
    for(unsigned long i = 0; i < MAX_FARGS; i++)
        if(s->subnodes[i]){
            write_expr_node(s->subnodes[i]);
            FREP(s->subnodes[i]);
        }
}
static void plan_scope(scope* s);
static void write_scope(scope* s);

static void plan_stmts(stmt* s, unsigned long n){
    //plan_array(s, sizeof(stmt_astexec), n);
    plan_array_new(s, sizeof(stmt), sizeof(stmt_astexec), n);
    for(unsigned long i = 0; i < n; i++){
        if(s[i].myscope){
            plan_scope(s[i].myscope);
        }
        for(unsigned long j = 0; j < s[i].nexpressions; j++)
        {
            if(s[i].nexpressions == 0) continue;
            plan_expr_node(s[i].expressions[j]);
        }
        /*
        if(s[i].referenced_label_name){
            plan_string(s[i].referenced_label_name);
        }*/
        if(s[i].switch_nlabels){
            push_plan(s[i].switch_label_indices, sizeof(uint64_t) * s[i].switch_nlabels);
        }
    }
}
static void write_stmts(stmt* s_old, unsigned long n){
    stmt_astexec* s = pdecode(s_old);
    for(unsigned long i = 0; i < n; i++){
        memcpy(s + i ,s_old + i,sizeof(stmt_astexec));
        REP(s[i].whereami)
        if(s[i].referenced_loop){
            void* pp = pdecode(s[i].referenced_loop);
            if(pp == 0){
                puts("<OPTIMIZER ERROR>");
                puts("referenced_loop was not registered!");
            }
            REP(s[i].referenced_loop);
        }
        if(s[i].myscope){
            write_scope((scope*)s[i].myscope);
            FREP(s[i].myscope);
        }
        for(unsigned long j = 0; j < s[i].nexpressions; j++)
        {
            if(s[i].nexpressions == 0) continue;
            write_expr_node(s[i].expressions[j]);
            FREP(s[i].expressions[j])
        }
        /*
        if(s[i].referenced_label_name){
            write_string(s[i].referenced_label_name);
            FREP(s[i].referenced_label_name)
        }*/
        if(s[i].switch_nlabels){
            memcpy(
                pdecode(s[i].switch_label_indices), 
                s[i].switch_label_indices, 
                s[i].switch_nlabels * sizeof(uint64_t)
            );
            FREP(s[i].switch_label_indices);
        }
    }
}



static void plan_scope(scope* s){
    push_plan(s, sizeof(scope_astexec));
    if(s->nsyms)
        plan_lvars(s->syms, s->nsyms);
    if(s->nstmts)
        plan_stmts(s->stmts, s->nstmts);
}
static void write_scope(scope* s_old){
    scope_astexec* s = pdecode(s_old);
    memcpy(s,s_old, sizeof(scope_astexec));
    if(s->nsyms){
        write_lvars((symdecl*)s->syms, s->nsyms);
        FREP(s->syms)
    }
    if(s->nstmts){
        write_stmts(s->stmts, s->nstmts);
        FREP(s->stmts)
    }
}



void optimize_fn(symdecl* ss){

    memneeded = 0;
    bbuf = 0;
    npairs = 0;
    if(ss->t.is_function == 0 || ss->is_codegen == 0) return;
    //printf("Optimizing function %s\n", ss->name);
    plan_scope(ss->fbody);
    for(unsigned long i = 0; i < ss->nargs; i++){
        push_plan(ss->fargs[i], sizeof(type));
    }
    //Malloc must be aligned to ALGN here...
    bbuf = malloc(memneeded);
    memneeded = 0;

    
    //WALK THE TREE, BUT THIS TIME, DECODING...
    write_scope(ss->fbody);
    FREP(ss->fbody);
    for(unsigned long i = 0; i < ss->nargs; i++){
        memcpy(pdecode(ss->fargs[i]), ss->fargs[i], sizeof(type));
        FREP(ss->fargs[i])
    }
    //FUNCTION IS OPTIMIZED...
}

