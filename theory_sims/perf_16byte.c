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
    if(argc == 1)
    for(int i= 0; i < 100000; i++){
    
        mystruct pp;
        pp.p = rand();
        pp.p2 = rand();
        pp = byvalue(pp);
    }
    else{
        for(int i= 0; i < 100000; i++){
        
            mystruct pp;
            pp.p = rand();
            pp.p2 = rand();
            bypointer(&pp);
        }
    }

}


