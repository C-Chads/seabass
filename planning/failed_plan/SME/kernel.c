

#include <SDL2/SDL.h>

#include <SDL2/SDL_mixer.h>
#include <stddef.h>
#include "stringutil.h"
#include "stdio.h"
#include "libmin.h"

enum{
    SME_TYPE_ERR = 0,
    SME_TYPE_R8 = 1,
    SME_TYPE_R16 = 2,
    SME_TYPE_R32 = 3,
    SME_TYPE_R64 = 4,
    SME_TYPE_R128 = 5,
    SME_TYPE_R256 = 6,
    SME_TYPE_RMAX = 7,
    SME_TYPE_COMPOUND = 8,
    SME_TYPE_NTYPES
};
enum {
    SME_REALTYPE_NA = 0,
    SME_REALTYPE_I = 1,
    SME_REALTYPE_F = 2,
    SME_REALTYPE_NTYPES
};

//TODO: serialize types, load and save them, etc.

typedef struct sme_type{
    struct sme_type* members; //for SME_TYPE_COMPOUND
    size_t type; //from the SME_TYPE_ enum.
    size_t rtype; //for MASM
    size_t ptrlevel;
    size_t arraysz; //for arrays. This is only used to declare variables.
    size_t nmembers;
    size_t is_literal; //is this a literal type?
    char* name;
} sme_type;

typedef sme_type sme_typedecl;

typedef struct {
    sme_type t;
    size_t off;
} sme_gvar;

sme_type type_clone(sme_type in){
    sme_type ret;
    ret = in;
    ret.name = strdup(in.name);
    if(ret.nmembers){
        ret.members = malloc(sizeof(sme_type) * ret.nmembers);
    }
    for(unsigned i = 0; i < in.nmembers; i++){
        ret.members[i] = type_clone(in.members[i]);
    }
    return ret;
}

sme_type type_init(){sme_type t = {0};return t;}

void type_destroy(sme_type* in, int root){
    for(size_t i = 0; i < in->nmembers; i++)
        type_destroy(in->members+i, 0);
    if(in->members) free(in->members);
    if(in->name) free(in->name);
    if(!root) free(in);
}

size_t type_eq(sme_type a, sme_type b){
    if(a.type != b.type) return 0;
    if(a.rtype != b.rtype) return 0;
    if(a.ptrlevel != b.ptrlevel) return 0;
    if(a.arraysz != b.arraysz) return 0;
    if(a.type != SME_TYPE_COMPOUND) return 1;
    //this is a compound type...
    if(a.nmembers != b.nmembers) return 0;
    if(a.nmembers == 0) return 1; //a zero 
    for(size_t i = 0; i < a.nmembers; i++){
        //
        if(!type_eq(a.members[i], b.members[i])){
            return 0;
        }
    }
    return 1;
}

size_t type_getsz_native(sme_type in){
    if(in.arraysz == 0){ //not an array
        if(in.ptrlevel) //is a pointer!
            return sizeof(char*);
        if(in.type == SME_TYPE_COMPOUND){
            size_t t;
            for(size_t i = 0; i < in.nmembers; i++)
                t+= type_getsz_native(in.members[i]);
            return t;
        }
        if(in.type == SME_TYPE_R8) return 1;
        if(in.type == SME_TYPE_R16) return 2;
        if(in.type == SME_TYPE_R32) return 4;
        if(in.type == SME_TYPE_R64) return 8;
        if(in.type == SME_TYPE_R128) return 16;
        if(in.type == SME_TYPE_R256) return 32;
        if(in.type == SME_TYPE_RMAX) return 8;
    } else {
        sme_type q;
        q = in;
        q.arraysz = 0;
        return in.arraysz * type_getsz_native(q);
    }
}

typedef struct{
    char* name; //actual name of the function.
    char* unmangled_name; //unmangled, i.e. before method name mangling.
    size_t offset; //offset into the module's bytecode.
    //Per my decisions, functions now take and return their argument list- 
    //"Shared Variable Syntax"
    sme_type argcombo;
    //we can duck-type this during compilation...
    sme_type method_on_this_type;
    size_t callsite; //where in the module do we go to make a call?
    size_t is_method;
} sme_fn;

/*
    Use global buffer objects to store global data.
    
    We use syscalls.
    
    global variables will be allowed in the editor too and will be serialized as their own
    buffers.
*/



typedef struct{
    char* softpane; //software rendering pane.    
    char has_pane; //if it has a pane
    char paneshown; //is the pane shown or hidden?
    //pane data
    unsigned w;
    unsigned h;
    unsigned x;
    unsigned y;
    
    //OpenGL rendering props.
    unsigned framebuffer;
    unsigned rendered_texture;
    unsigned depthbuffer;
    //callsites for things the active SME system uses.
    //TODO keep track of callsites for api callbacks, modinit, modpaneshown, etc
    
    
    char using_opengl; //0 for software, 1 for 
    char is_gui;    //does this module actively use the gui?
                    //if a module does not actively use the gui,
                    //it does not need to be drawn, ticked, resized, or
                    //given a pane.
                    //This can be determined from the source code of the
                    //module, as it will say!
    char activity_disabled; //if the module has its activity disabled, it will
                            //no longer be drawn or rendered.
} sme_mod_gui_props;



typedef struct{
    char* name; //module name.
    char* orig_source; //original source code as a C string.
    char* new_source; //When a module is being edited, we keep new source code.
                      //the new source is not yet built
    char* bytecode;
    char* gdata;
    size_t gdata_sz;
    sme_gvar* gvars;
    sme_fn* funcs;
    sme_typedecl* typedecls;
    size_t nfuncs;
    size_t ngvars;
    size_t ntypedecls;
    size_t bytecode_sz;
    size_t id; //module id stored in module.
    sme_mod_gui_props guiprops;
} sme_module;

typedef struct{
    char* name;
    size_t creator_modid; //module that created me.
    char* buf;
    size_t buf_sz;
} sme_gbo;

sme_gbo* gbos = 0;
size_t ngbos = 0;
sme_module* mods = 0;
size_t nmods = 0;

/*
    THE AST for SME-MASM
    
    
*/

typedef sme_type sme_ast_vardecl;

/*

    TODO:
    allow for argument mapping.
    
    fcall cos2 [a b] ([c d])
    
    does a cosine on two floats (c and d) passed in as a combo,
    returning a combo, which is then mapped to the variables (a and b)
*/




typedef struct sme_ast_combo_node{
    sme_type t; //type of the node- evaluated return type.
                //if this is a terminal, we also put the name here.
    char is_terminal;
    struct sme_ast_expr_node* members;
    size_t nmembers;
} sme_ast_combo_node;

typedef struct sme_ast_insn{
    sme_ast_combo_node arg;
    char* opname;
    void* child_scope;
    char* labelname; //for goto, we need to have a label to go to.
} sme_ast_insn;

typedef struct{
    sme_ast_insn* insns;
    sme_ast_vardecl* vars;
    size_t nvars;
    size_t ninsns;
} sme_ast_scope;

typedef struct{
    sme_type argcombo;
    sme_ast_scope myScope;
    char* name;
    char is_method;
    sme_type method_on_this_type; //used for generating the fn name.
    
} sme_ast_fn;

typedef struct{
    char* name;
    sme_ast_fn* fns;
    sme_ast_vardecl* gvars;
    size_t is_codegen;
    size_t nfns;
    size_t ngvars;
    //where the resulting code is stored.
    sme_module* output_mod;
} sme_ast_mod_root;

/*LOW LEVEL TYPES*/

#define i8 signed char
#define u8 unsigned char
#define i16 signed short
#define u16 unsigned short
#define i32 int
#define u32 unsigned int
#define i64 signed long long
#define u64 unsigned long long


void syscall(u64* regs){
    //TODO.
    (void)regs;
    return;
}

#include "be_encoder.h"
#include "pcode.h"
#include "lex.h"
#include "parse.h"


/*
    SYNTAX NOTES:
    
    we have to delimit line endings because...
    addi a b b
    [r64[r64 r32]] myVar
    is ambiguous when reaching the first open square bracket.
*/


int main(int argc, char** argv){
    pcode_stack = malloc(PCODE_STACK_SZ);
    //TODO

}
