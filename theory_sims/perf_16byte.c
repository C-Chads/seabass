#include <stdio.h>
#include <stdlib.h>
#include <time.h>

//16 byte struct...
typedef struct{
    unsigned long long p;
    unsigned long long p2;
    //unsigned long long p3;
}mystruct;


__attribute__ ((noinline)) void bypointer(mystruct* pp){
    pp->p++;
    pp->p2++;
}

__attribute__ ((noinline)) mystruct byvalue(mystruct pp){
    pp.p++;
    pp.p2++;
    return pp;
}


int main(int argc, char** argv){
    srand(time(0));
    mystruct pp;
    pp.p = atoi(argv[1]) + rand();
    pp.p2 = atoi(argv[1]) + rand();
    
    if(argc == 2)
        puts("By value!");
    else puts("By pointer!");
    
    if(argc == 2){
        for(unsigned long long i= 0; i < (1<<25); i++){
            pp = byvalue(pp);
        }
    }else{
        for(unsigned long long i= 0; i < (1<<25); i++){
            bypointer(&pp);
        }
    }
    printf("%lld\n%lld", pp.p, pp.p2);

}


