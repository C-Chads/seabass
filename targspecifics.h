

#ifndef TARG_SPECIFICS_H
#define TARG_SPECIFICS_H
#include <stdint.h>
#include <stdlib.h>
#define LIBMIN_INT int64_t
#define LIBMIN_UINT uint64_t
#define LIBMIN_FLOAT double
#define POINTER_SIZE 8
#define MAX_FARGS 64

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
    /*
    if(p == NULL){
        puts("malloc failed");
        exit(1);
    }
    */
    return p;
}
static inline void* c_allocX(uint64_t sz){
    void* p = calloc(sz, 1);
    /*
    if(p == NULL){
        puts("calloc failed");
        exit(1);
    }*/
    return p;
}


static inline void* re_allocX(void* p, uint64_t sz){
    p = realloc(p, sz);

    return p;
}


#endif
