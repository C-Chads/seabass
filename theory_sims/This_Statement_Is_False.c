#include <stdio.h>
#include <stdlib.h>
int returns_true(void*p);
int g();
int h();
int G_TRUTHINESS = 1;
int H_TRUTHINESS = 1;

int returns_true(void* p){
    if(p == g) return G_TRUTHINESS;
    if(p == h) return H_TRUTHINESS;
    puts("Undefined behavior"); exit(1);
}
int g(){
    return !returns_true(g);
}
int h(){
    return returns_true(h);
}
int main(){
    H_TRUTHINESS = 0;
    printf("RETURNS_TRUE(h)=%d, h()=%d\n", returns_true(h), h());
    H_TRUTHINESS = 1;
    printf("RETURNS_TRUE(h)=%d, h()=%d\n", returns_true(h), h());
    G_TRUTHINESS = 0;
    printf("RETURNS_TRUE(g)=%d, g()=%d\n", returns_true(g), g());
    G_TRUTHINESS = 1;
    printf("RETURNS_TRUE(g)=%d, g()=%d\n", returns_true(g), g());
}
