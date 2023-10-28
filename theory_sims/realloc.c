#include <stdlib.h>

#include <stdio.h>
#include <string.h>

char* b = 0;


int main(int argc, char** argv){

    char* prev_b = b;
    for(int i = 0; i < 1000000; i++){
        b = realloc(b, i+1);
        if(b != prev_b){
            printf("\nPointer change at %d",i);
        }
        prev_b = b;
        b[i] = rand();
    }
}
