    /*C program*/
    #include <stdio.h>
    #include <stdlib.h>
    
    void mutoa(char* dest, unsigned int value);
    unsigned matou(char* in);
    
    static inline unsigned int fib(unsigned n){
        if(n < 2)
            return 1;
        unsigned int a = 1;
        unsigned int b = 1;
        unsigned int c = 1;
        n = n - 2;
        while(n){
            c = a + b;
            a = b;
            b = c;
            n--;
        }
        return c;
    }
    
    int main(int argc, char** argv){
        if(argc < 2){
            puts("Usage: fib 13");
            exit(1);
        }
        puts("Welcome to the fibonacci number calculator!");
        char buf[50];
        unsigned qq = matou(argv[1]);
        puts("You asked for the fibonacci number...");
        mutoa(buf, qq);
        puts(buf);
        puts("That fibonacci number is:");
        mutoa(buf, fib(qq));
        puts(buf);
        return 0;
    }
    
    unsigned matou(char* in){
        unsigned retval = 0;
        while(
            (*in >= '0') &&
            (*in <= '9')
        ){
            retval *= 10;
            retval += *in - '0';
            in++;
        }
        return retval;
    }
    
    void mutoa(char* dest, unsigned value){
        if(value == 0){
            dest[0] = '0';
            dest[1] = 0;
            return;
        }
        {
            unsigned pow;
            pow = 1;
            while(value/pow >= 10){
                pow = pow * 10;
            }
            while(pow){
                unsigned temp;
                temp = value / pow;
                *dest = (temp + ('0')); dest++;
                value = value - temp * pow;
                pow = pow / 10;
            }
        }
        ending:;
        *dest = 0;
    }
