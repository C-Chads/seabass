

#ifndef TARG_SPECIFICS_H
#define TARG_SPECIFICS_H
#include <stdint.h>
#include <stdlib.h>
#define LIBMIN_INT int64_t
#define LIBMIN_UINT uint64_t
#define LIBMIN_FLOAT double
#define MAX_FARGS 64
#define SEABASS_CODEGEN_64
/*#define SEABASS_CODEGEN_32*/

#ifdef SEABASS_CODEGEN_64
#define POINTER_SIZE 8
#else
#ifdef SEABASS_CODEGEN_32
#define POINTER_SIZE 4
#else
#error "Unsupported SEABASS_CODEGEN (either not defined, or wrong value....)"
#endif
#endif

#ifdef SEABASS_CODEGEN_64
#ifdef SEABASS_CODEGEN_32
#error "Only one SEABASS_CODEGEN_XX define can be used at a time..."
#endif
#endif

/*
    Notice that "unsigned long" is used all over the place here.

    to port this stuff to a 64 bit platform, change it all to "unsigned long long" and "long long" or 
    size_t and ssize_t
*/


/*
    Commandline argument handler. The code generator may need to know things about
    the kind of code to generate.

*/
unsigned handle_cli_args(int argc, char** argv);



static inline void* m_allocX(uint64_t sz){
    void* p = malloc(sz);
    return p;
}
static inline void* c_allocX(uint64_t sz){
    void* p = calloc(1, sz);
    return p;
}


static inline void* re_allocX(void* p, uint64_t sz){
    p = realloc(p, sz);

    return p;
}


#endif
