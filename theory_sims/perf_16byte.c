#include <stdio.h>
#include <stdlib.h>
#include <time.h>

//16 byte struct...
typedef struct{
    unsigned long long p;
    unsigned long long p2;
}mystruct;


void bypointer(mystruct* pp){
    pp->p++;
    pp->p2++;
}

mystruct byvalue(mystruct pp){
    pp.p++;
    pp.p2++;
    return pp;
}


int main(int argc, char** argv){
    srand(time(0));
    mystruct pp;
    pp.p = atoi(argv[1]);
    pp.p2 = atoi(argv[1]);
    
    
    if(argc == 2){
        for(unsigned long long i= 0; i < (1<<25); i++){
            pp = byvalue(pp);
        }
    }else{
        for(unsigned long long i= 0; i < (1<<25); i++){
            bypointer(&pp);
        }
    }

}


