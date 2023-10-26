


/*
EXPORTED CODE FROM SEABASS METAPROGRAMMING LANGUAGE
Code Generator written by DAVID MHS WEBSTER / KEGGEK
Let all your things be done with charity
*/
/*
TYPEDECLS
*/
typedef union bbb{
    float  b;
    int  a;
    union bbb*  beebeebee;
} bbb;
typedef struct membuf{
    unsigned char*  d;
} membuf;
typedef struct this_type_has_a_destructor{
    int  a;
    int  b;
    unsigned char*  qq;
} this_type_has_a_destructor;
/*
GLOBAL VARIABLES
*/
static bbb my_bbbs[60];
static int myintegers[7] = {
3, 4, 5, 7, 9, 10, 1
};
static float myfloats[3] = {
3.0, 3.1415901184082031250000000000000000000000000000000000000000000000, -14.6999998092651367187500000000000000000000000000000000000000000000
};
int myint = 3;
static int myint2 = 3;
unsigned char mystring[59] = {
65, 65, 65, 65, 84, 104, 105, 115, 
32, 105, 115, 32, 97, 32, 115, 116, 
114, 105, 110, 103, 33, 32, 73, 32, 
119, 111, 110, 100, 101, 114, 32, 119, 
104, 97, 116, 32, 116, 104, 101, 32, 
108, 105, 115, 116, 32, 99, 104, 97, 
114, 97, 99, 116, 101, 114, 32, 105, 
115, 63, 0
};
static _Atomic int my_integer = 37;
/*
FUNCTION PROTOTYPES
*/
extern unsigned openfile(unsigned char* __cbas_local_variable_mangled_name_fname,unsigned char* __cbas_local_variable_mangled_name_mode);
extern int closefile(unsigned __cbas_local_variable_mangled_name_p);
extern unsigned get_stdout_handle();
extern unsigned get_stdin_handle();
extern unsigned get_stderr_handle();
extern unsigned write_bytes(unsigned __cbas_local_variable_mangled_name_fhandle,unsigned char* __cbas_local_variable_mangled_name_buf,unsigned __cbas_local_variable_mangled_name_nbytes);
extern unsigned read_bytes(unsigned __cbas_local_variable_mangled_name_fhandle,unsigned char* __cbas_local_variable_mangled_name_buf,unsigned __cbas_local_variable_mangled_name_nbytes);
extern unsigned get_utime();
extern unsigned char* get_real_path(unsigned char* __cbas_local_variable_mangled_name_path);
extern void set_errno(int __cbas_local_variable_mangled_name_new_errno);
extern int get_errno();
extern void mcpy(unsigned char* __cbas_local_variable_mangled_name_dst,unsigned char* __cbas_local_variable_mangled_name_src,unsigned __cbas_local_variable_mangled_name_sz);
extern void println(unsigned char* __cbas_local_variable_mangled_name_s);
extern unsigned char* mem_alloc(unsigned __cbas_local_variable_mangled_name_amt);
extern void mem_free(unsigned char* __cbas_local_variable_mangled_name_p);
extern unsigned char* mem_realloc(unsigned char* __cbas_local_variable_mangled_name_p,unsigned __cbas_local_variable_mangled_name_amt);
extern void sys_exit(int __cbas_local_variable_mangled_name_a);
static void __method_membuf_____ctor(membuf* __cbas_local_variable_mangled_name_this);
static void __method_membuf_____dtor(membuf* __cbas_local_variable_mangled_name_this);
static void __method_membuf_____req(membuf* __cbas_local_variable_mangled_name_this,unsigned __cbas_local_variable_mangled_name_sz);
static inline int __method_bbb_____do_stuff(bbb* __cbas_local_variable_mangled_name_this,int __cbas_local_variable_mangled_name_incr);
static int myFunction(int __cbas_local_variable_mangled_name_arg1,int __cbas_local_variable_mangled_name_arg2,int __cbas_local_variable_mangled_name_arg3);
static inline int __method_bbb_____do_stuff2(bbb* __cbas_local_variable_mangled_name_this,int __cbas_local_variable_mangled_name_incr,int __cbas_local_variable_mangled_name_incr2,int __cbas_local_variable_mangled_name_incr3);
static void __method_bbb_____init(bbb* __cbas_local_variable_mangled_name_this);
static inline void mutoa(unsigned char* __cbas_local_variable_mangled_name_dest,unsigned __cbas_local_variable_mangled_name_value);
static inline void __method_this_type_has_a_destructor_____dtor(this_type_has_a_destructor* __cbas_local_variable_mangled_name_this);
static inline void __method_this_type_has_a_destructor_____ctor(this_type_has_a_destructor* __cbas_local_variable_mangled_name_this);
static inline int impl_streq(unsigned char* __cbas_local_variable_mangled_name_a,unsigned char* __cbas_local_variable_mangled_name_b);
static void whosa_yousa(int __cbas_local_variable_mangled_name_a);
static void wadda(int __cbas_local_variable_mangled_name_iters);
void inf_loop();
int inf_loop2();
static int inf_loop3(int __cbas_local_variable_mangled_name_a);
int main();
/*
FUNCTION BODIES
*/
extern unsigned openfile(unsigned char* __cbas_local_variable_mangled_name_fname,unsigned char* __cbas_local_variable_mangled_name_mode);
extern int closefile(unsigned __cbas_local_variable_mangled_name_p);
extern unsigned get_stdout_handle();
extern unsigned get_stdin_handle();
extern unsigned get_stderr_handle();
extern unsigned write_bytes(unsigned __cbas_local_variable_mangled_name_fhandle,unsigned char* __cbas_local_variable_mangled_name_buf,unsigned __cbas_local_variable_mangled_name_nbytes);
extern unsigned read_bytes(unsigned __cbas_local_variable_mangled_name_fhandle,unsigned char* __cbas_local_variable_mangled_name_buf,unsigned __cbas_local_variable_mangled_name_nbytes);
extern unsigned get_utime();
extern unsigned char* get_real_path(unsigned char* __cbas_local_variable_mangled_name_path);
extern void set_errno(int __cbas_local_variable_mangled_name_new_errno);
extern int get_errno();
extern void mcpy(unsigned char* __cbas_local_variable_mangled_name_dst,unsigned char* __cbas_local_variable_mangled_name_src,unsigned __cbas_local_variable_mangled_name_sz);
extern void println(unsigned char* __cbas_local_variable_mangled_name_s);
extern unsigned char* mem_alloc(unsigned __cbas_local_variable_mangled_name_amt);
extern void mem_free(unsigned char* __cbas_local_variable_mangled_name_p);
extern unsigned char* mem_realloc(unsigned char* __cbas_local_variable_mangled_name_p,unsigned __cbas_local_variable_mangled_name_amt);
extern void sys_exit(int __cbas_local_variable_mangled_name_a);
static void __method_membuf_____ctor(membuf* __cbas_local_variable_mangled_name_this)
{
    (((membuf*)(__cbas_local_variable_mangled_name_this))->d) = ((unsigned char*)(0));
    
}
static void __method_membuf_____dtor(membuf* __cbas_local_variable_mangled_name_this)
{
    if(((unsigned char*)(((membuf*)(__cbas_local_variable_mangled_name_this))->d)) != (0)){

    mem_free(((unsigned char*)(((membuf*)(__cbas_local_variable_mangled_name_this))->d)));
    }

    
}
static void __method_membuf_____req(membuf* __cbas_local_variable_mangled_name_this,unsigned __cbas_local_variable_mangled_name_sz)
{
    (((membuf*)(__cbas_local_variable_mangled_name_this))->d) = (mem_alloc(((unsigned)(__cbas_local_variable_mangled_name_sz))));
    if(((unsigned char*)(((membuf*)(__cbas_local_variable_mangled_name_this))->d)) == (0)){

    println(("mem_alloc failed!"));
    sys_exit(((int)(1)));
    }

    
}
static inline int __method_bbb_____do_stuff(bbb* __cbas_local_variable_mangled_name_this,int __cbas_local_variable_mangled_name_incr)
{
    float* __cbas_local_variable_mangled_name_b_ptr;
    int* __cbas_local_variable_mangled_name_a_ptr;
    (((bbb*)(__cbas_local_variable_mangled_name_this))->a) = (((int)(((bbb*)(__cbas_local_variable_mangled_name_this))->a)) + ((int)(__cbas_local_variable_mangled_name_incr)));
    (__cbas_local_variable_mangled_name_a_ptr) = (&(((bbb*)(__cbas_local_variable_mangled_name_this))->a));
    (__cbas_local_variable_mangled_name_b_ptr) = (&(((bbb*)(__cbas_local_variable_mangled_name_this))->b));
    if(((int*)(__cbas_local_variable_mangled_name_a_ptr)) != ((int*)(__cbas_local_variable_mangled_name_b_ptr))){

    

    }

    else {
    
    return(int)(((bbb*)(__cbas_local_variable_mangled_name_this))->a);
    }

    
    return((int)(((bbb*)(__cbas_local_variable_mangled_name_this))->a)) - ((int)(1));
    
}
static int myFunction(int __cbas_local_variable_mangled_name_arg1,int __cbas_local_variable_mangled_name_arg2,int __cbas_local_variable_mangled_name_arg3)
{
    
    return(((int)(__cbas_local_variable_mangled_name_arg1)) + ((int)(__cbas_local_variable_mangled_name_arg2))) + ((int)(__cbas_local_variable_mangled_name_arg3));
    
}
static inline int __method_bbb_____do_stuff2(bbb* __cbas_local_variable_mangled_name_this,int __cbas_local_variable_mangled_name_incr,int __cbas_local_variable_mangled_name_incr2,int __cbas_local_variable_mangled_name_incr3)
{
    float* __cbas_local_variable_mangled_name_b_ptr;
    int* __cbas_local_variable_mangled_name_a_ptr;
    (((bbb*)(__cbas_local_variable_mangled_name_this))->a) = (((((int)(((bbb*)(__cbas_local_variable_mangled_name_this))->a)) + ((int)(__cbas_local_variable_mangled_name_incr))) + ((int)(__cbas_local_variable_mangled_name_incr2))) + ((int)(__cbas_local_variable_mangled_name_incr3)));
    (__cbas_local_variable_mangled_name_a_ptr) = (&(((bbb*)(__cbas_local_variable_mangled_name_this))->a));
    (__cbas_local_variable_mangled_name_b_ptr) = (&(((bbb*)(__cbas_local_variable_mangled_name_this))->b));
    if(((int*)(__cbas_local_variable_mangled_name_a_ptr)) != ((int*)(__cbas_local_variable_mangled_name_b_ptr))){

    

    }

    else {
    
    return(int)(((bbb*)(__cbas_local_variable_mangled_name_this))->a);
    }

    
    return((int)(((bbb*)(__cbas_local_variable_mangled_name_this))->a)) - ((int)(1));
    
}
static void __method_bbb_____init(bbb* __cbas_local_variable_mangled_name_this)
{
    (((bbb*)(__cbas_local_variable_mangled_name_this))->a) = ((int)(0));
    
}
static inline void mutoa(unsigned char* __cbas_local_variable_mangled_name_dest,unsigned __cbas_local_variable_mangled_name_value)
{
    float* __cbas_local_variable_mangled_name_ppp;
    if(((unsigned)(__cbas_local_variable_mangled_name_value)) == (0)){

    (((unsigned char*)(__cbas_local_variable_mangled_name_dest))[(int)(0)]) = ((unsigned char)(48));
    (__cbas_local_variable_mangled_name_dest)++;
    goto __cbas_mangled_label__ending;
    }

    if((int)(1)){

    unsigned __cbas_local_variable_mangled_name_pow;
    (__cbas_local_variable_mangled_name_pow) = (1);
    while((((unsigned)(__cbas_local_variable_mangled_name_value)) / ((unsigned)(__cbas_local_variable_mangled_name_pow))) > (9)){

    (__cbas_local_variable_mangled_name_pow) = (((unsigned)(__cbas_local_variable_mangled_name_pow)) * (10));
    }

    while((int)(__cbas_local_variable_mangled_name_pow)){

    unsigned __cbas_local_variable_mangled_name_temp;
    (__cbas_local_variable_mangled_name_temp) = (((unsigned)(__cbas_local_variable_mangled_name_value)) / ((unsigned)(__cbas_local_variable_mangled_name_pow)));
    (((unsigned char*)(__cbas_local_variable_mangled_name_dest))[(int)(0)]) = ((unsigned char)(((unsigned)(__cbas_local_variable_mangled_name_temp)) + (48)));
    (__cbas_local_variable_mangled_name_dest)++;
    (__cbas_local_variable_mangled_name_value) = (((unsigned)(__cbas_local_variable_mangled_name_value)) - (((unsigned)(__cbas_local_variable_mangled_name_temp)) * ((unsigned)(__cbas_local_variable_mangled_name_pow))));
    (__cbas_local_variable_mangled_name_pow) = (((unsigned)(__cbas_local_variable_mangled_name_pow)) / (10));
    }

    }

    __cbas_mangled_label__ending:1;/*Statement with no effect generated for label...*/
    (((unsigned char*)(__cbas_local_variable_mangled_name_dest))[(int)(0)]) = ((unsigned char)(0));
    
    return;
    (__cbas_local_variable_mangled_name_ppp) = (myfloats);
    
}
static inline void __method_this_type_has_a_destructor_____dtor(this_type_has_a_destructor* __cbas_local_variable_mangled_name_this)
{
    if((int)(1)){

    unsigned char __cbas_local_variable_mangled_name___pprint_local_buf[250];
    println(("Goodbye mom!"));
    println(("My a was:"));
    mutoa((__cbas_local_variable_mangled_name___pprint_local_buf),((unsigned)(((this_type_has_a_destructor*)(__cbas_local_variable_mangled_name_this))->a)));
    println((__cbas_local_variable_mangled_name___pprint_local_buf));
    }

    if(((unsigned char*)(((this_type_has_a_destructor*)(__cbas_local_variable_mangled_name_this))->qq)) != (0)){

    mem_free(((unsigned char*)(((this_type_has_a_destructor*)(__cbas_local_variable_mangled_name_this))->qq)));
    }

    
}
static inline void __method_this_type_has_a_destructor_____ctor(this_type_has_a_destructor* __cbas_local_variable_mangled_name_this)
{
    println(("Hello Mom!"));
    (((this_type_has_a_destructor*)(__cbas_local_variable_mangled_name_this))->a) = ((int)(5));
    (((this_type_has_a_destructor*)(__cbas_local_variable_mangled_name_this))->b) = ((int)(7));
    (((this_type_has_a_destructor*)(__cbas_local_variable_mangled_name_this))->qq) = (mem_alloc((20)));
    if(((unsigned char*)(((this_type_has_a_destructor*)(__cbas_local_variable_mangled_name_this))->qq)) == (0)){

    println(("mem_alloc failed!"));
    }

    
}
static inline int impl_streq(unsigned char* __cbas_local_variable_mangled_name_a,unsigned char* __cbas_local_variable_mangled_name_b)
{
    while((int)(1)){

    if(((unsigned char)(((unsigned char*)(__cbas_local_variable_mangled_name_a))[(int)(0)])) != ((unsigned char)(((unsigned char*)(__cbas_local_variable_mangled_name_b))[(int)(0)]))){

    
    return(int)(0);
    }

    if(((unsigned)(((unsigned char*)(__cbas_local_variable_mangled_name_a))[(int)(0)])) == (0)){

    
    return(int)(1);
    }

    (__cbas_local_variable_mangled_name_a)++;
    (__cbas_local_variable_mangled_name_b)++;
    }

    
}
static void whosa_yousa(int __cbas_local_variable_mangled_name_a)
{
    println(("Whosa programmer? Yousa programmer!"));
    
}
static void wadda(int __cbas_local_variable_mangled_name_iters)
{
    unsigned char __cbas_local_variable_mangled_name_buf[50];
    int __cbas_local_variable_mangled_name_i;
    int __cbas_local_variable_mangled_name_j;
    for((__cbas_local_variable_mangled_name_i) = ((int)(0));((int)(__cbas_local_variable_mangled_name_i)) <= ((int)(__cbas_local_variable_mangled_name_iters));(__cbas_local_variable_mangled_name_i)++){

    mutoa((__cbas_local_variable_mangled_name_buf),((unsigned)(((int)(__cbas_local_variable_mangled_name_i)) * ((int)(__cbas_local_variable_mangled_name_i)))));
    for((__cbas_local_variable_mangled_name_j) = ((int)(0));((int)(__cbas_local_variable_mangled_name_j)) < ((int)(12));(__cbas_local_variable_mangled_name_j)++){

    println((__cbas_local_variable_mangled_name_buf));
    if(((int)(__cbas_local_variable_mangled_name_j)) == ((int)(3))){

    ((__cbas_local_variable_mangled_name_buf)[(int)(0)]) = ((unsigned char)(113));
    println((__cbas_local_variable_mangled_name_buf));
    break;
    }

    }

    if(((int)(__cbas_local_variable_mangled_name_i)) == ((int)(13))){

    break;
    }

    continue;
    }

    
    whosa_yousa((__cbas_local_variable_mangled_name_iters));return;
    
}
void inf_loop()
{
    
    inf_loop();return;
    
}
int inf_loop2()
{
    
    return inf_loop2();
    
}
static int inf_loop3(int __cbas_local_variable_mangled_name_a)
{
    (__cbas_local_variable_mangled_name_a) = (((int)(__cbas_local_variable_mangled_name_a)) + ((int)(1)));
    if((((int)(__cbas_local_variable_mangled_name_a)) % ((int)(5))) != ((int)(0))){

    
    return inf_loop3((__cbas_local_variable_mangled_name_a));
    }

    
    return(int)(__cbas_local_variable_mangled_name_a);
    
}
int main()
{
    unsigned char* __cbas_local_variable_mangled_name_p;
    bbb __cbas_local_variable_mangled_name_my_bbb;
    bbb __cbas_local_variable_mangled_name_my_bbb_2;
    int __cbas_local_variable_mangled_name_i;
    println(("Hello World, from CBAS!"));
    (__cbas_local_variable_mangled_name_p) = ((unsigned char*)&wadda);
    ((void(*)(int))((unsigned char*)(__cbas_local_variable_mangled_name_p)))(((int)(30)));
    if((int)(0)){

    println(("a"));
    }

    else if(impl_streq(("eq"),("neq"))){

    println(("b"));
    }

    else {
    println(("Quality!"));
    }

    if((int)(1)){

    const void* const __cbas_dispatchtable_switch__0_____[4] = {
    &&__cbas_mangled_label__aa, &&__cbas_mangled_label__bb, &&__cbas_mangled_label__cc, &&__cbas_mangled_label__dd
    };
    goto *__cbas_dispatchtable_switch__0_____[(int)(2)];;
    goto *__cbas_dispatchtable_switch__0_____[(int)(3)];;
    goto *__cbas_dispatchtable_switch__0_____[(int)(3)];;
    goto *__cbas_dispatchtable_switch__0_____[(int)(3)];;
    goto *__cbas_dispatchtable_switch__0_____[(int)(3)];;
    __cbas_mangled_label__aa:1;/*Statement with no effect generated for label...*/
    println(("Got a 0!"));
    goto __cbas_mangled_label__ee;
    __cbas_mangled_label__bb:1;/*Statement with no effect generated for label...*/
    println(("Got a 1!"));
    goto __cbas_mangled_label__ee;
    __cbas_mangled_label__cc:1;/*Statement with no effect generated for label...*/
    println(("Got a 2!"));
    goto __cbas_mangled_label__ee;
    __cbas_mangled_label__dd:1;/*Statement with no effect generated for label...*/
    println(("Got a 3!"));
    goto __cbas_mangled_label__ee;
    __cbas_mangled_label__ee:1;/*Statement with no effect generated for label...*/
    if((int)(1)){

    unsigned char __cbas_local_variable_mangled_name___pprint_local_buf[250];
    println(("Goodbye!"));
    println(("This should be 8:"));
    mutoa((__cbas_local_variable_mangled_name___pprint_local_buf),((1) << (3)));
    println((__cbas_local_variable_mangled_name___pprint_local_buf));
    }

    }

    for((my_integer) = ((int)(0));((int)(my_integer)) < ((int)(500));(my_integer)++){

    if((((int)(my_integer)) % ((int)(10))) == ((int)(0))){

    if((int)(1)){

    unsigned char __cbas_local_variable_mangled_name___pprint_local_buf[250];
    println(("Printing atomic variable my_integer"));
    mutoa((__cbas_local_variable_mangled_name___pprint_local_buf),((unsigned)(my_integer)));
    println((__cbas_local_variable_mangled_name___pprint_local_buf));
    }

    }

    }

    __method_bbb_____init(((&__cbas_local_variable_mangled_name_my_bbb)));
    for((__cbas_local_variable_mangled_name_i) = ((int)(0));((int)(__cbas_local_variable_mangled_name_i)) < ((int)(10));(__cbas_local_variable_mangled_name_i)++){

    if((int)(1)){

    unsigned char __cbas_local_variable_mangled_name___pprint_local_buf[250];
    println(("Another iteration?"));
    mutoa((__cbas_local_variable_mangled_name___pprint_local_buf),((unsigned)(__method_bbb_____do_stuff2(((&__cbas_local_variable_mangled_name_my_bbb)),((int)(12)),((int)(12)),((int)(12))))));
    println((__cbas_local_variable_mangled_name___pprint_local_buf));
    }

    }

    (((&__cbas_local_variable_mangled_name_my_bbb_2))->a) = ((int)(0));
    (((&__cbas_local_variable_mangled_name_my_bbb_2))->b) = ((float)(0));
    (*((&__cbas_local_variable_mangled_name_my_bbb_2)) = *((&__cbas_local_variable_mangled_name_my_bbb)));
    if((int)(1)){

    unsigned char __cbas_local_variable_mangled_name___pprint_local_buf[250];
    println(("my_bbb_2.a = "));
    mutoa((__cbas_local_variable_mangled_name___pprint_local_buf),((unsigned)(((&__cbas_local_variable_mangled_name_my_bbb_2))->a)));
    println((__cbas_local_variable_mangled_name___pprint_local_buf));
    println(("my_bbb.a = "));
    mutoa((__cbas_local_variable_mangled_name___pprint_local_buf),((unsigned)(((&__cbas_local_variable_mangled_name_my_bbb))->a)));
    println((__cbas_local_variable_mangled_name___pprint_local_buf));
    }

    if((int)(1)){

    unsigned char __cbas_local_variable_mangled_name___pprint_local_buf[250];
    println(("This is what I get for inf_loop3:"));
    mutoa((__cbas_local_variable_mangled_name___pprint_local_buf),((unsigned)(inf_loop3(((int)(23))))));
    println((__cbas_local_variable_mangled_name___pprint_local_buf));
    println(("As you can see, it wasn't really an infinite loop after all!"));
    println(("This is intentional..."));
    }

    (((my_bbbs) + ((int)(23)))->a) = ((int)(17));
    (((my_bbbs) + ((int)(24)))->a) = ((int)(90));
    __method_bbb_____do_stuff(((my_bbbs) + ((int)(23))),((int)(1)));
    if((int)(1)){

    unsigned char __cbas_local_variable_mangled_name___pprint_local_buf[250];
    println(("Here's what happened to my_bbbs+23:"));
    mutoa((__cbas_local_variable_mangled_name___pprint_local_buf),((unsigned)(((my_bbbs) + ((int)(23)))->a)));
    println((__cbas_local_variable_mangled_name___pprint_local_buf));
    println(("And +24:"));
    mutoa((__cbas_local_variable_mangled_name___pprint_local_buf),((unsigned)(((my_bbbs) + ((int)(24)))->a)));
    println((__cbas_local_variable_mangled_name___pprint_local_buf));
    println(("Furthermore, if we use getglobalptr and get the 24th element..."));
    mutoa((__cbas_local_variable_mangled_name___pprint_local_buf),((unsigned)(((my_bbbs) + ((int)(24)))->a)));
    println((__cbas_local_variable_mangled_name___pprint_local_buf));
    println(("Interpreted as a float, this is..."));
    mutoa((__cbas_local_variable_mangled_name___pprint_local_buf),((unsigned)(((my_bbbs) + ((int)(24)))->b)));
    println((__cbas_local_variable_mangled_name___pprint_local_buf));
    }

    (((my_bbbs) + ((int)(24)))->b) = (25.2999992370605468750000000000000000000000000000000000000000000000);
    if((int)(1)){

    unsigned char __cbas_local_variable_mangled_name___pprint_local_buf[250];
    println(("I've assigned the float with the value 25.3. Here it is multiplied by 10 as an int:"));
    mutoa((__cbas_local_variable_mangled_name___pprint_local_buf),((unsigned)(((float)(((my_bbbs) + ((int)(24)))->b)) * ((float)(10)))));
    println((__cbas_local_variable_mangled_name___pprint_local_buf));
    println(("And here is the integer interpretation:"));
    mutoa((__cbas_local_variable_mangled_name___pprint_local_buf),((unsigned)(((my_bbbs) + ((int)(24)))->a)));
    println((__cbas_local_variable_mangled_name___pprint_local_buf));
    println(("And the size of my_bbb is..."));
    println(("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~"));
    mutoa((__cbas_local_variable_mangled_name___pprint_local_buf),(sizeof(bbb)));
    println((__cbas_local_variable_mangled_name___pprint_local_buf));
    println(("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~"));
    }

    if((int)(1)){

    this_type_has_a_destructor __cbas_local_variable_mangled_name_qq;
    this_type_has_a_destructor __cbas_local_variable_mangled_name_qq2;
    int __cbas_local_variable_mangled_name_ffff;
    __method_this_type_has_a_destructor_____ctor(((&__cbas_local_variable_mangled_name_qq)));
    __method_this_type_has_a_destructor_____ctor(((&__cbas_local_variable_mangled_name_qq2)));
    for((__cbas_local_variable_mangled_name_ffff) = ((int)(0));((int)(__cbas_local_variable_mangled_name_ffff)) < ((int)(10));(__cbas_local_variable_mangled_name_ffff)++){

    this_type_has_a_destructor __cbas_local_variable_mangled_name_a;
    this_type_has_a_destructor __cbas_local_variable_mangled_name_b;
    this_type_has_a_destructor __cbas_local_variable_mangled_name_c;
    int __cbas_local_variable_mangled_name_quiet;
    __method_this_type_has_a_destructor_____ctor(((&__cbas_local_variable_mangled_name_a)));
    __method_this_type_has_a_destructor_____ctor(((&__cbas_local_variable_mangled_name_b)));
    __method_this_type_has_a_destructor_____ctor(((&__cbas_local_variable_mangled_name_c)));
    (((&__cbas_local_variable_mangled_name_a))->a) = ((int)(1));
    (((&__cbas_local_variable_mangled_name_b))->a) = ((int)(2));
    (((&__cbas_local_variable_mangled_name_c))->a) = ((int)(3));
    for((__cbas_local_variable_mangled_name_quiet) = ((int)(0));((int)(__cbas_local_variable_mangled_name_quiet)) < ((int)(3));(__cbas_local_variable_mangled_name_quiet)++){

    if((int)(1)){

    unsigned char __cbas_local_variable_mangled_name___pprint_local_buf[250];
    println(("Do do do..."));
    mutoa((__cbas_local_variable_mangled_name___pprint_local_buf),((unsigned)(((((int)(__cbas_local_variable_mangled_name_quiet)) * ((int)(__cbas_local_variable_mangled_name_quiet))) * ((int)(__cbas_local_variable_mangled_name_quiet))) * ((int)(__cbas_local_variable_mangled_name_quiet)))));
    println((__cbas_local_variable_mangled_name___pprint_local_buf));
    }

    }

    if(((int)(__cbas_local_variable_mangled_name_ffff)) == ((int)(3))){

    __method_this_type_has_a_destructor_____dtor(((&__cbas_local_variable_mangled_name_c)));
    __method_this_type_has_a_destructor_____dtor(((&__cbas_local_variable_mangled_name_b)));
    __method_this_type_has_a_destructor_____dtor(((&__cbas_local_variable_mangled_name_a)));
    break;
    }

    if(((int)(__cbas_local_variable_mangled_name_ffff)) == ((int)(9))){

    __method_this_type_has_a_destructor_____dtor(((&__cbas_local_variable_mangled_name_c)));
    __method_this_type_has_a_destructor_____dtor(((&__cbas_local_variable_mangled_name_b)));
    __method_this_type_has_a_destructor_____dtor(((&__cbas_local_variable_mangled_name_a)));
    __method_this_type_has_a_destructor_____dtor(((&__cbas_local_variable_mangled_name_qq2)));
    __method_this_type_has_a_destructor_____dtor(((&__cbas_local_variable_mangled_name_qq)));
    goto __cbas_mangled_label__real_end;
    }

    if((int)(0)){

    __method_this_type_has_a_destructor_____dtor(((&__cbas_local_variable_mangled_name_c)));
    __method_this_type_has_a_destructor_____dtor(((&__cbas_local_variable_mangled_name_b)));
    __method_this_type_has_a_destructor_____dtor(((&__cbas_local_variable_mangled_name_a)));
    __method_this_type_has_a_destructor_____dtor(((&__cbas_local_variable_mangled_name_qq2)));
    __method_this_type_has_a_destructor_____dtor(((&__cbas_local_variable_mangled_name_qq)));
    
    return(int)(0);
    println(("This should never print!"));
    }

    __method_this_type_has_a_destructor_____dtor(((&__cbas_local_variable_mangled_name_c)));
    __method_this_type_has_a_destructor_____dtor(((&__cbas_local_variable_mangled_name_b)));
    __method_this_type_has_a_destructor_____dtor(((&__cbas_local_variable_mangled_name_a)));
    }

    __method_this_type_has_a_destructor_____dtor(((&__cbas_local_variable_mangled_name_qq2)));
    __method_this_type_has_a_destructor_____dtor(((&__cbas_local_variable_mangled_name_qq)));
    }

    if((int)(1)){

    membuf __cbas_local_variable_mangled_name_buffer;
    __method_membuf_____ctor(((&__cbas_local_variable_mangled_name_buffer)));
    __method_membuf_____req(((&__cbas_local_variable_mangled_name_buffer)),((1) << (10)));
    println(("Enter 10 characters"));
    read_bytes((get_stdin_handle()),((unsigned char*)(((&__cbas_local_variable_mangled_name_buffer))->d)),(10));
    println(("Here are your characters back:"));
    write_bytes((get_stdout_handle()),((unsigned char*)(((&__cbas_local_variable_mangled_name_buffer))->d)),(10));
    __method_membuf_____dtor(((&__cbas_local_variable_mangled_name_buffer)));
    }

    __cbas_mangled_label__real_end:1;/*Statement with no effect generated for label...*/
    
}
/*
Blessed be the Lord Jesus Christ of Nazareth, forever. Amen.
*/
