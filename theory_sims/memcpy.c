#include <stdio.h>




int main(){

    int a = 4;
    int b = 12;
    memcpy(&a, &b, sizeof(int));
    printf("%d", a);
}
