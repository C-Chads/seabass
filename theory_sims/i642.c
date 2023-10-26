#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

int main(int argc, char** argv){
    int64_t ii;
    ii = strtoll(argv[1], 0, 0);
    ii |= ((int64_t)0xf<<(64-4));
    printf("%zx",ii);

}
