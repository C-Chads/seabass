
#include <iostream>
#include <cstdlib>
#include <cstdio>

typedef unsigned char byte;

typedef byte (*bytefunction)(byte);

byte incr_byte(byte i){return i + 1;}

class byte_state_decision_node{
    public:
        byte_state_decision_node* left;
        byte_state_decision_node* right;
        byte value;
    void encode(bytefunction f){
        byte memo[256];
        byte i = 0;
        while(1){
            memo[i] = f(i);
            i++;
            if(i==0)break;
        }
        //TODO: find correlations
    }
};


int main(){

}
