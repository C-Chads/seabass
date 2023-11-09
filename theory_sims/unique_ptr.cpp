#include <memory>
#include <cstdio>
#include <cstdlib>
using namespace std;
struct mybuf{
    char d[500];
};
template <typename T>
struct optr{
    T* d;
    optr(T* a){
        d = a;
    }
    T* operator->(){
        return d;
    }
    ~optr(){
        delete d;
    }
};
template <typename T>
struct optr2{
    T* d;
    optr2(T* a){
        d = a;
    }
    T* operator->(){
        return d;
    }
    ~optr2(){
        free(d);
    }
};
void myfunc1(){
    unique_ptr<mybuf> p(new mybuf);
    scanf("%s",p->d);
    puts(p->d);
}

void myfunc2(){
    mybuf* p = new mybuf;
    scanf("%s",p->d);
    puts(p->d);
    delete p;
}
static void myfunc3(){
    optr p(new mybuf);
    scanf("%s",p->d);
    puts(p->d);
}
static void myfunc4(){
    optr2 p((mybuf*)malloc(sizeof(mybuf)));
    scanf("%s",p->d);
    puts(p->d);
}
static void myfunc5(){
    mybuf* p = (mybuf*)malloc(sizeof(mybuf));
    scanf("%s",p->d);
    puts(p->d);
    free(p);
}
