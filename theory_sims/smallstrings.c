
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define N 50000000
#define STRLN 10


char* alloc_pascal_string(char* src_buf, unsigned len){
    char* retval = malloc(4+len);
    memcpy(retval+4, src_buf, len);
    memcpy(retval, &len, 4); //our length!
    return retval;
}

char** all;



int main(){
    all = malloc(sizeof(char*) * N);
    unsigned int c = 0;
    char buf[STRLN+1];
    buf[STRLN] = 0;
    srand(time(0));
    //allocate pascal strings
    printf("ALLOCATING PASCAL STRINGS!\n");
    printf("TIME START: %zu\n", time(0));
    for(long long i =0; i < N; i++){
        unsigned long j;
        for(j = 0; j < STRLN; j++){
            buf[j] = rand();
            if(rand()%7 == 1) break;
        }
        if(j<STRLN)j++;
        all[i] = alloc_pascal_string(buf, j);
    }
    printf("TRAVERSING PASCAL STRINGS!\n");
    printf("TIME: %zu\n", time(0));
    for(long long i =0; i < N; i++){
        unsigned l;
        memcpy(&l, all[i], 4); //get length of pascal string
        fflush(stdout);
        for(unsigned long j = 0; j < l; j++){
            if(all[i][4+j] == 'q') c++;
        }
    }
    printf("Qs: %u\nTIME STOP: %zu\n", c, time(0));
    for(long long i =0; i < N; i++){
        free(all[i]);
    }
        c = 0;

    //do the same thing, but for C strings....
    printf("ALLOCATING C-STRINGS!\n");
    printf("TIME START: %zu\n", time(0));
    for(long long i =0; i < N; i++){
        for(unsigned long j = 0; j < STRLN; j++){
            buf[j] = rand();
            if(rand()%7 == 1) break;
        }
        all[i] = strdup(buf);
    }
    printf("TRAVERSING C-STRINGS!\n");
    printf("TIME: %zu\n", time(0));

    for(long long i =0; i < N; i++){
        for(unsigned long j = 0; all[i][j]; j++){
            if(all[i][j] == 'q') c++;
        }
    }
    printf("Qs: %u\nTIME STOP: %zu\n", c, time(0));

}

