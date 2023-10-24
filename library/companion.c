
/*
    SEABASS STANDARD LIBRARY IMPLEMENTATION FOR C PROGRAMMING.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <limits.h>
#include <errno.h>
#define uc unsigned char

size_t openfile(uc* fname, uc* mode){
    return (size_t)fopen((char*)fname, (char*)mode);
}

int closefile(size_t p){
    return fclose((FILE*)p);
}

size_t get_stdout_handle(){
    return (size_t)stdout;
}

size_t get_stdin_handle(){
    return (size_t)stdin;
}

size_t get_stderr_handle(){
    return (size_t)stderr;
}

size_t write_bytes(size_t fhandle, uc* buf, size_t nbytes){
    return fwrite(buf, 1, nbytes,(FILE*)fhandle);
}

size_t read_bytes(size_t fhandle, uc* buf, size_t nbytes){
    return fread(buf, 1, nbytes,(FILE*)fhandle);
}

size_t get_utime(){
    return time(0);
}

uc* get_real_path(uc* path){
    return (uc*)realpath((char*)path,NULL);
}

void set_errno(int new_errno){
    errno = new_errno;
}

int get_errno(){
    return errno;
}

//this is included in case we are not able to optimize...

void mcpy(unsigned char* dst, unsigned char* src, size_t sz){
    memcpy(dst, src, sz);
}

//these are provided for similar reasons...
unsigned char* mem_alloc(size_t amt){
    return malloc(amt);
}

void mem_free(unsigned char* p){
    free(p);
}

unsigned char* mem_realloc(unsigned char* b, size_t amt){
    return realloc(b, amt);
}

void println(char* s){
    puts(s);
}




