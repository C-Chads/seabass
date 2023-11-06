#include "stdio.h"
int main(){
    printf("size of long double is... %zu", sizeof(long double));
    printf("alignof of long double is... %zu", _Alignof(long double));
    printf("alignof of double is... %zu", _Alignof(double));
}
