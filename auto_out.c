
/*
* ---- EXPORTED BY TOC.HBAS ----
* Beloved, if God so loved us, we ought also to love one another.
* 1 Jn 4:11
*/

#define SEABASS_64_BIT 1
#define i32 int
#define u32 unsigned int
#define _STRTOU_ strtoull
#define _STRTOI_ strtoll
#define TGT_UMAX unsigned long long
#define TGT_IMAX long long


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <errno.h>
#include <sys/types.h>
#include <dirent.h>

/*for unsupported platforms...*/
#ifdef __APPLE__
#define __CBAS_SINGLE_THREADED__
#endif


#ifndef __CBAS_SINGLE_THREADED__
#include <pthread.h>
typedef struct {
    pthread_mutex_t myMutex;
    pthread_barrier_t myBarrier;
    pthread_t myThread;
    int isThreadLive;
    int shouldKillThread;
    int state;
    void (*execute)(unsigned char*);
    unsigned char* argument;
} lsthread;
static inline void init_lsthread(lsthread* t);
static inline void start_lsthread(lsthread* t);
static inline void kill_lsthread(lsthread* t);
static inline void destroy_lsthread(lsthread* t);
static inline void lock(lsthread* t);
static inline void step(lsthread* t);
static void* lsthread_func(void* me_void);

static inline void init_lsthread(lsthread* t){
    pthread_mutex_init(&t->myMutex, NULL);
    pthread_barrier_init(&t->myBarrier, NULL, 2);
    t->isThreadLive = 0;
    t->shouldKillThread = 0;
    t->state = 0;
    t->execute = NULL;
    t->argument = NULL;
}
static inline void destroy_lsthread(lsthread* t){
    pthread_mutex_destroy(&t->myMutex);
    pthread_barrier_destroy(&t->myBarrier);
}
static inline void lock(lsthread* t){
    if(t->state == 1)return;
    if(!t->isThreadLive)return;
    pthread_barrier_wait(&t->myBarrier);
    if(pthread_mutex_lock(&t->myMutex))
        exit(1);
    t->state = 1;
}

static inline void step(lsthread* t){
    if(t->state == -1)return;
    if(!t->isThreadLive)return;
    if(pthread_mutex_unlock(&(t->myMutex)))
        exit(1);
    pthread_barrier_wait(&t->myBarrier);
    t->state = -1;
}
static inline void kill_lsthread(lsthread* t){
    if(!t->isThreadLive)return;
    if(t->state != 1){
        lock(t);
    }
    t->shouldKillThread = 1;
    step(t);
    
    pthread_join(t->myThread,NULL);
    
    t->isThreadLive = 0;
    t->shouldKillThread = 0;
}
static void* lsthread_func(void* me_void){
    lsthread* me = (lsthread*) me_void;
    if (!me)pthread_exit(NULL);
    while (1) {
        pthread_barrier_wait(&me->myBarrier);
        pthread_mutex_lock(&me->myMutex);
        if (!(me->shouldKillThread) && me->execute)
            me->execute(me->argument);
        else if(me->shouldKillThread){
            pthread_mutex_unlock(&me->myMutex);
            pthread_exit(NULL);
        }

        pthread_mutex_unlock(&me->myMutex);
        pthread_barrier_wait(&me->myBarrier);
    }
    pthread_exit(NULL);
}
static inline void start_lsthread(lsthread* t){
    if(t->isThreadLive)return;
    t->isThreadLive = 1;
    t->shouldKillThread = 0;
    if(pthread_mutex_lock(&t->myMutex))
        exit(1);
    t->state = 1;/*LOCKED*/
    pthread_create(
        &t->myThread,
        NULL,
        lsthread_func,
        (void*)t
    );
}

#endif

static inline double __CBAS__atof(unsigned char* text){
    return strtod((char*)text, NULL);
}
static inline TGT_UMAX __CBAS__atou(unsigned char* text){
#ifndef SEABASS_16_BIT
    return _STRTOU_((char*)text, NULL, 0);
#else
    return _STRTOU_((char*)text);
#endif
}
static inline TGT_IMAX __CBAS__atoi(unsigned char* text){
#ifndef SEABASS_16_BIT
    return _STRTOI_((char*)text, NULL, 0);
#else
    return _STRTOI_((char*)text);
#endif
}

static inline void __CBAS__srand(u32 seed){
    srand(seed);
}
static inline i32 __CBAS__rand(){
    return rand();
}
static inline TGT_IMAX __CBAS__strcmp(unsigned char* a, unsigned char* b){
    return strcmp((char*)a,(char*)b);
}
static inline TGT_IMAX __CBAS__memcmp(unsigned char* a, unsigned char* b, TGT_UMAX sz){
    return memcmp((char*)a,(char*)b, sz);
}
static inline TGT_UMAX __CBAS__strlen(unsigned char* s){
    return strlen((char*)s);
}

static inline TGT_UMAX __CBAS__fopen(unsigned char* fname, unsigned char* mode){
    return (TGT_UMAX)fopen((char*)fname, (char*)mode);
}

static inline i32 __CBAS__fclose(TGT_UMAX p){
    return fclose((FILE*)p);
}
static inline i32 __CBAS__fflush(TGT_UMAX p){
    return fflush((FILE*)p);
}

static inline TGT_UMAX __CBAS__popen(unsigned char* fname, unsigned char* mode){
    return (TGT_UMAX)popen((char*)fname, (char*)mode);
}

static inline i32 __CBAS__pclose(TGT_UMAX p){
    return pclose((FILE*)p);
}

/*fn predecl noexport getdirlist(char* dirname, char*** entry_listing, u32* nentries)->i32;*/
static inline i32 __CBAS__getdirlist(unsigned char* dirname, unsigned char*** entry_listing, u32* nentries){
    DIR* dir;
    struct dirent* de;
    dir = opendir((char*)dirname);
    if(dir == NULL) return 1; 
    nentries[0] = 0;
    entry_listing[0] = 0;
    while(1){
        de = readdir(dir);
        if(de == NULL) break;
        entry_listing[0] = realloc(entry_listing[0], sizeof(void*) * ++(nentries[0]));
        entry_listing[0][nentries[0]-1] = strdup(de->d_name);
    }
    closedir(dir);
    return 0;
}

static inline size_t __CBAS__system(unsigned char* cmd){
    return system((char*)cmd);
}

static inline TGT_UMAX __CBAS__get_stdout_handle(){
    return (TGT_UMAX)stdout;
}

static inline TGT_UMAX __CBAS__get_stdin_handle(){
    return (TGT_UMAX)stdin;
}

static inline TGT_UMAX __CBAS__get_stderr_handle(){
    return (TGT_UMAX)stderr;
}

static inline TGT_UMAX __CBAS__fwrite(TGT_UMAX fhandle, unsigned char* buf, TGT_UMAX nbytes){
    return fwrite(buf, 1, nbytes,(FILE*)fhandle);
}

static inline TGT_UMAX __CBAS__fread(TGT_UMAX fhandle, unsigned char* buf, TGT_UMAX nbytes){
    return fread(buf, 1, nbytes,(FILE*)fhandle);
}

static inline TGT_UMAX __CBAS__unixtime(){
    return time(0);
}
static inline TGT_UMAX __CBAS__time(TGT_UMAX* a){
    time_t qq;
    qq = *a;
    time_t retval = time(&qq);
    *a = qq;
    return retval;
}
static inline unsigned char* __CBAS__ctime(TGT_UMAX* a){
    time_t qq;
    qq = *a;
    unsigned char* retval = (unsigned char*)ctime(&qq);
    *a = qq;
    return retval;
}
static inline double __CBAS__difftime(TGT_UMAX a, TGT_UMAX b){
    return difftime(a,b);
}
static inline TGT_UMAX __CBAS__clock(){
    return clock();
}
static inline TGT_UMAX __CBAS__CLOCKS_PER_SEC(){
    return CLOCKS_PER_SEC;
}
static inline unsigned char* __CBAS__localtime(TGT_UMAX a){
    time_t tt = a;
    struct tm * timeinfo;
    timeinfo = localtime(&tt);
    return (unsigned char*)timeinfo;
}
static inline unsigned char* __CBAS__gmtime(TGT_UMAX a){
    time_t tt = a;
    struct tm * timeinfo;
    timeinfo = gmtime(&tt);
    return (unsigned char*)timeinfo;
}
static inline unsigned char* __CBAS__asctime(unsigned char* tm_struct){
   struct tm timeinfo;
   memcpy(&timeinfo, tm_struct, sizeof(struct tm));
   return (unsigned char*)asctime(&timeinfo);
}

static inline i32 __CBAS__tm_sec(unsigned char* tm_struct){
   struct tm timeinfo;
   
   memcpy(&timeinfo, tm_struct, sizeof(struct tm));
   return timeinfo.tm_sec;
}
static inline i32 __CBAS__tm_min(unsigned char* tm_struct){
   struct tm timeinfo;
   
   memcpy(&timeinfo, tm_struct, sizeof(struct tm));
   return timeinfo.tm_min;
}
static inline i32 __CBAS__tm_hour(unsigned char* tm_struct){
   struct tm timeinfo;
   
   memcpy(&timeinfo, tm_struct, sizeof(struct tm));
   return timeinfo.tm_hour;
}
static inline i32 __CBAS__tm_mday(unsigned char* tm_struct){
   struct tm timeinfo;
   
   memcpy(&timeinfo, tm_struct, sizeof(struct tm));
   return timeinfo.tm_mday;
}
static inline i32 __CBAS__tm_mon(unsigned char* tm_struct){
   struct tm timeinfo;
   
   memcpy(&timeinfo, tm_struct, sizeof(struct tm));
   return timeinfo.tm_mon;
}
static inline i32 __CBAS__tm_year(unsigned char* tm_struct){
   struct tm timeinfo;
   
   memcpy(&timeinfo, tm_struct, sizeof(struct tm));
   return timeinfo.tm_year;
}
static inline i32 __CBAS__tm_wday(unsigned char* tm_struct){
   struct tm timeinfo;
   
   memcpy(&timeinfo, tm_struct, sizeof(struct tm));
   return timeinfo.tm_wday;
}
static inline i32 __CBAS__tm_yday(unsigned char* tm_struct){
   struct tm timeinfo;
   
   memcpy(&timeinfo, tm_struct, sizeof(struct tm));
   return timeinfo.tm_yday;
}
static inline i32 __CBAS__tm_isdst(unsigned char* tm_struct){
   struct tm timeinfo;
   
   memcpy(&timeinfo, tm_struct, sizeof(struct tm));
   return timeinfo.tm_isdst;
}
static inline void __CBAS__set_errno(i32 new_errno){
    errno = new_errno;
}
static inline i32 __CBAS__get_errno(){
    return errno;
}
static inline void __CBAS__mcpy(unsigned char* dst, unsigned char* src, TGT_UMAX sz){
    memcpy(dst, src, sz);
}
static inline void __CBAS__mmove(unsigned char* dst, unsigned char* src, TGT_UMAX sz){
    memmove(dst, src, sz);
}
static inline unsigned char* __CBAS__malloc(TGT_UMAX amt){
    return malloc(amt);
}
static inline void __CBAS__free(unsigned char* p){
    free(p);
}
static inline unsigned char* __CBAS__realloc(unsigned char* b, TGT_UMAX amt){
    return realloc(b, amt);
}
static inline void __CBAS__println(unsigned char* s){
    puts((char*)s);
}
static inline void __CBAS__sys_exit(i32 a){
    exit(a);
}
    static inline double __CBAS__sin(double a){
        return sin(a);
    }

    static inline double __CBAS__cos(double a){
        return cos(a);
    }

    static inline double __CBAS__tan(double a){
        return tan(a);
    }

    static inline float __CBAS__sinf(float a){
        return sinf(a);
    }

    static inline float __CBAS__cosf(float a){
        return cosf(a);
    }

    static inline float __CBAS__tanf(float a){
        return tanf(a);
    }

    static inline double __CBAS__asin(double a){
        return asin(a);
    }

    static inline double __CBAS__acos(double a){
        return acos(a);
    }

    static inline double __CBAS__atan(double a){
        return atan(a);
    }

    static inline float __CBAS__asinf(float a){
        return asinf(a);
    }

    static inline float __CBAS__acosf(float a){
        return acosf(a);
    }

    static inline float __CBAS__atanf(float a){
        return atanf(a);
    }
    static inline double __CBAS__atan2(double a, double b){
        return atan2(a, b);
    }
    static inline float __CBAS__atan2f(float a, float b){
        return atan2f(a, b);
    }
    static inline double __CBAS__sinh(double a){
        return sinh(a);
    }
    static inline double __CBAS__cosh(double a){
        return cosh(a);
    }
    static inline double __CBAS__tanh(double a){
        return tanh(a);
    }
    static inline float __CBAS__sinhf(float a){
        return sinhf(a);
    }
    static inline float __CBAS__coshf(float a){
        return coshf(a);
    }
    static inline float __CBAS__tanhf(float a){
        return tanhf(a);
    }
    static inline double __CBAS__asinh(double a){
        return asinh(a);
    }
    static inline double __CBAS__acosh(double a){
        return acosh(a);
    }
    static inline double __CBAS__atanh(double a){
        return atanh(a);
    }
    static inline float __CBAS__asinhf(float a){
        return asinhf(a);
    }
    static inline float __CBAS__acoshf(float a){
        return acoshf(a);
    }
    static inline float __CBAS__atanhf(float a){
        return atanhf(a);
    }
    static inline double __CBAS__exp(double a){
        return exp(a);
    }
    static inline float __CBAS__expf(float a){
        return expf(a);
    }
    
    static inline double __CBAS__frexp(double a, i32* b){
        int cc = *b;
        double retval = frexp(a,&cc);
        *b = cc;
        return retval;
    }
    static inline float __CBAS__frexpf(float a, i32* b){
        int cc = *b;
        float retval = frexpf(a,&cc);
        *b = cc;
        return retval;
    }
    static inline double __CBAS__ldexp(double a, i32 b){
        return ldexp(a,b);
    }
    static inline float __CBAS__ldexpf(float a, i32 b){
        return ldexp(a,b);
    }
    static inline double __CBAS__log(double a){
        return log(a);
    }
    static inline float __CBAS__logf(float a){
        return logf(a);
    }
    static inline double __CBAS__log10(double a){
        return log10(a);
    }
    static inline float __CBAS__log10f(float a){
        return log10f(a);
    }
    static inline double __CBAS__log2(double a){
        return log2(a);
    }
    static inline float __CBAS__log2f(float a){
        return log2f(a);
    }
    static inline double __CBAS__logb(double a){
        return logb(a);
    }
    static inline float __CBAS__logbf(float a){
        return logbf(a);
    }
    static inline double __CBAS__log1p(double a){
        return log1p(a);
    }
    static inline float __CBAS__log1pf(float a){
        return log1pf(a);
    }
    static inline double __CBAS__modf(double a, double* b){
        return modf(a, b);
    }
    static inline float __CBAS__modff(float a, float* b){
        return modff(a, b);
    }
    static inline double __CBAS__exp2(double a){
        return exp2(a);
    }
    static inline float __CBAS__exp2f(float a){
        return exp2f(a);
    }
    static inline double __CBAS__expm1(double a){
        return expm1(a);
    }
    static inline float __CBAS__expm1f(float a){
        return expm1f(a);
    }
    static inline i32 __CBAS__ilogb(double a){
        return ilogb(a);
    }
    static inline i32 __CBAS__ilogbf(float a){
        return ilogbf(a);
    }
    static inline double __CBAS__scalbn(double a, i32 n){
        return scalbn(a,n);
    }
    static inline float __CBAS__scalbnf(float a, i32 n){
        return scalbnf(a,n);
    }
    static inline double __CBAS__pow(double a, double b){
        return pow(a, b);
    }
    static inline float __CBAS__powf(float a, float b){
        return powf(a, b);
    }
    static inline double __CBAS__sqrt(double a){
        return sqrt(a);
    }
    static inline float __CBAS__sqrtf(float a){
        return sqrtf(a);
    }
    static inline double __CBAS__cbrt(double a){
        return cbrt(a);
    }
    static inline float __CBAS__cbrtf(float a){
        return cbrtf(a);
    }
    static inline double __CBAS__hypot(double a, double b){
        return hypot(a, b);
    }
    static inline float __CBAS__hypotf(float a, float b){
        return hypotf(a, b);
    }
    static inline double __CBAS__erf(double a){
        return erf(a);
    }
    static inline float __CBAS__erff(float a){
        return erff(a);
    }
    static inline double __CBAS__erfc(double a){
        return erfc(a);
    }
    static inline float __CBAS__erfcf(float a){
        return erfcf(a);
    }
    static inline double __CBAS__tgamma(double a){
        return tgamma(a);
    }
    static inline float __CBAS__tgammaf(float a){
        return tgammaf(a);
    }
    static inline double __CBAS__lgamma(double a){
        return lgamma(a);
    }
    static inline float __CBAS__lgammaf(float a){
        return lgammaf(a);
    }
    static inline double __CBAS__ceil(double a){
        return ceil(a);
    }
    static inline float __CBAS__ceilf(float a){
        return ceilf(a);
    }
    static inline double __CBAS__floor(double a){
        return floor(a);
    }
    static inline float __CBAS__floorf(float a){
        return floorf(a);
    }    
    static inline double __CBAS__trunc(double a){
        return trunc(a);
    }
    static inline float __CBAS__truncf(float a){
        return truncf(a);
    }
    static inline double __CBAS__round(double a){
        return round(a);
    }
    static inline float __CBAS__roundf(float a){
        return roundf(a);
    }
    static inline double __CBAS__fmod(double a, double b){
        return fmod(a,b);
    }
    static inline float __CBAS__fmodf(float a, float b){
        return fmodf(a,b);
    }
    static inline double __CBAS__nearbyint(double a){
        return nearbyint(a);
    }
    static inline float __CBAS__nearbyintf(float a){
        return nearbyintf(a);
    }
    static inline double __CBAS__remainder(double a, double b){
        return remainder(a,b);
    }
    static inline float __CBAS__remainderf(float a, float b){
        return remainderf(a,b);
    }    
    static inline double __CBAS__remquo(double a, double b, i32* q){
        int p = *q;
        double retval = remquo(a,b,&p);
        *q = p;
        return retval;
    }
    static inline float __CBAS__remquof(float a, float b, i32* q){
        int p = *q;
        float retval = remquof(a,b,&p);
        *q = p;
        return retval;
    }
    static inline double __CBAS__copysign(double a, double b){
        return copysign(a,b);
    }
    static inline float __CBAS__copysignf(float a, float b){
        return copysignf(a,b);
    }
    static inline double __CBAS__nan(unsigned char* tagp){
        return nan((const char*)tagp);
    }
    static inline float __CBAS__nanf(unsigned char* tagp){
        return nanf((const char*)tagp);
    }
    static inline double __CBAS__nextafter(double a, double b){
        return nextafter(a,b);
    }
    static inline float __CBAS__nextafterf(float a, float b){
        return nextafterf(a,b);
    }
    static inline double __CBAS__fdim(double a, double b){
        return fdim(a,b);
    }
    static inline float __CBAS__fdimf(float a, float b){
        return fdimf(a,b);
    }
    static inline double __CBAS__fmax(double a, double b){
        return fmax(a,b);
    }
    static inline float __CBAS__fmaxf(float a, float b){
        return fmaxf(a,b);
    }
    static inline double __CBAS__fmin(double a, double b){
        return fmin(a,b);
    }
    static inline float __CBAS__fminf(float a, float b){
        return fminf(a,b);
    }
    static inline double __CBAS__fabs(double a){
        return fabs(a);
    }
    static inline float __CBAS__fabsf(float a){
        return fabsf(a);
    }
    static inline double __CBAS__fma(double a, double b, double z){
        return fma(a,b,z);
    }
    static inline float __CBAS__fmaf(float a, float b, float z){
        return fmaf(a,b,z);
    }
    static inline i32 __CBAS__fpclassify(double q){
        return fpclassify(q);
    }
    static inline i32 __CBAS__isfinite(double q){
        return isfinite(q);
    }
    static inline i32 __CBAS__isinf(double q){
        return isinf(q);
    }
    static inline i32 __CBAS__isnan(double q){
        return isnan(q);
    }
    static inline i32 __CBAS__isnormal(double q){
        return isnormal(q);
    }
    static inline i32 __CBAS__signbit(double q){
        return signbit(q);
    }
    static inline i32 __CBAS__fpclassifyf(float q){
        return fpclassify(q);
    }
    static inline i32 __CBAS__isfinitef(float q){
        return isfinite(q);
    }
    static inline i32 __CBAS__isinff(float q){
        return isinf(q);
    }
    static inline i32 __CBAS__isnanf(float q){
        return isnan(q);
    }
    static inline i32 __CBAS__isnormalf(float q){
        return isnormal(q);
    }
    static inline i32 __CBAS__signbitf(float q){
        return signbit(q);
    }

    static inline i32 __CBAS__isunordered(double a, double b){
        return isunordered(a,b);
    }
    static inline i32 __CBAS__isunorderedf(float a, float b){
        return isunordered(a,b);
    }

    static inline float __CBAS__INFINITY(){
        return INFINITY;
    }
    static inline float __CBAS__NAN(){
        return NAN;
    }
    static inline double __CBAS__HUGE_VAL(){
        return HUGE_VAL;
    }
    static inline float __CBAS__HUGE_VALF(){
        return HUGE_VALF;
    }

/*MULTITHREADING LIBRARY*/
#ifndef __CBAS_SINGLE_THREADED__


static inline unsigned char* __CBAS__thread_new(){
    lsthread* p = malloc(sizeof(lsthread));
    init_lsthread(p);
    return (unsigned char*)p;
}
static inline void __CBAS__thread_delete(unsigned char* tr){
    lsthread* p = (lsthread*)tr;
    destroy_lsthread(p);
    free(p);
    return;
}

static inline void __CBAS__thread_assign_fn(unsigned char* tr, unsigned char* funk){
    lsthread* p = (lsthread*)tr;
    void* fnk = funk;
    p->execute = fnk;
}

static inline void __CBAS__thread_assign_arg(unsigned char* tr, unsigned char* arg){
    lsthread* p = (lsthread*)tr;
    p->argument = arg;
}

static inline void __CBAS__thread_start(unsigned char* tr){
    lsthread* p = (lsthread*)tr;
    start_lsthread(p);
}
static inline void __CBAS__thread_kill(unsigned char* tr){
    lsthread* p = (lsthread*)tr;
    kill_lsthread(p);
}

static inline void __CBAS__thread_step(unsigned char* tr){
    lsthread* p = (lsthread*)tr;
    step(p);
}

static inline void __CBAS__thread_lock(unsigned char* tr){
    lsthread* p = (lsthread*)tr;
    lock(p);
}

static inline unsigned char* __CBAS__mutex_new(){
    pthread_mutex_t* m = malloc(sizeof(pthread_mutex_t));
    pthread_mutex_init(m, NULL);
    return (unsigned char*)m;
}

static inline void __CBAS__mutex_delete(unsigned char* mtx){
    pthread_mutex_t* m = (pthread_mutex_t*)mtx;
    pthread_mutex_destroy(m);
    free(m);
}

static inline void __CBAS__mutex_lock(unsigned char* mtx){
    pthread_mutex_t* m = (pthread_mutex_t*)mtx;
    pthread_mutex_lock(m);
}
static inline void __CBAS__mutex_unlock(unsigned char* mtx){
    pthread_mutex_t* m = (pthread_mutex_t*)mtx;
    pthread_mutex_unlock(m);
}
#else
static inline unsigned char* __CBAS__thread_new(){
    return malloc(sizeof(char*) * 2);
}
static inline void __CBAS__thread_delete(unsigned char* tr){
    free(tr);
    return;
}

static inline void __CBAS__thread_assign_fn(unsigned char* tr, unsigned char* funk){
    void** t = (void**)tr;
    t[0] = funk;
}

static inline void __CBAS__thread_assign_arg(unsigned char* tr, unsigned char* arg){
    void** t = (void**)tr;
    t[1] = arg;
}

static inline void __CBAS__thread_start(unsigned char* tr){
    return;
}
static inline void __CBAS__thread_kill(unsigned char* tr){
    return;
}

static inline void __CBAS__thread_step(unsigned char* tr){
    void** t = (void**)tr;
    /*get thread function...*/
    void (*execute)(unsigned char*);
    execute = (void*)(t[0]);
    execute(t[1]);
    return;
}

static inline void __CBAS__thread_lock(unsigned char* tr){
    return;
}

static inline unsigned char* __CBAS__mutex_new(){
    return NULL;
}

static inline void __CBAS__mutex_delete(unsigned char* mtx){
    free(mtx);
}

static inline void __CBAS__mutex_lock(unsigned char* mtx){
    return;
}
static inline void __CBAS__mutex_unlock(unsigned char* mtx){
    return;
}

#endif

/*
TYPEDECLS
*/
struct __CBAS__type__ldouble{ _Alignas(16) 
    unsigned char  __CBAS__member___d_[16ll];
} ;
struct __CBAS__type__str{
    unsigned char*  __CBAS__member__d;
    unsigned long long  __CBAS__member__len;
} ;
struct __CBAS__type__pstr{
    unsigned char*  __CBAS__member__d;
    unsigned long long  __CBAS__member__len;
} ;
/*
GLOBAL VARIABLES
*/
/*
FUNCTION PROTOTYPES
*/
static inline long long __CBAS__impl_streq(unsigned char* __cbas_local_variable_mangled_name_a,unsigned char* __cbas_local_variable_mangled_name_b);
static inline long long __CBAS__memeq(unsigned char* __cbas_local_variable_mangled_name_a,unsigned char* __cbas_local_variable_mangled_name_b,unsigned long long __cbas_local_variable_mangled_name_l);
static inline void __CBAS____method_str_____ctor(struct __CBAS__type__str* __cbas_local_variable_mangled_name_this);
static inline void __CBAS____method_str_____dtor(struct __CBAS__type__str* __cbas_local_variable_mangled_name_this);
static inline void __CBAS____method_pstr_____ctor(struct __CBAS__type__pstr* __cbas_local_variable_mangled_name_this);
static inline void __CBAS____method_pstr_____dtor(struct __CBAS__type__pstr* __cbas_local_variable_mangled_name_this);
static inline void __CBAS____method_str_____free(struct __CBAS__type__str* __cbas_local_variable_mangled_name_this);
static inline void __CBAS____method_pstr_____free(struct __CBAS__type__pstr* __cbas_local_variable_mangled_name_this);
static inline long long __CBAS____method_str_____equal(struct __CBAS__type__str* __cbas_local_variable_mangled_name_this,struct __CBAS__type__str* __cbas_local_variable_mangled_name_o);
static inline long long __CBAS____method_str_____nequal(struct __CBAS__type__str* __cbas_local_variable_mangled_name_this,struct __CBAS__type__str* __cbas_local_variable_mangled_name_o);
static inline long long __CBAS____method_pstr_____equal(struct __CBAS__type__pstr* __cbas_local_variable_mangled_name_this,struct __CBAS__type__pstr* __cbas_local_variable_mangled_name_o);
static inline long long __CBAS____method_pstr_____nequal(struct __CBAS__type__pstr* __cbas_local_variable_mangled_name_this,struct __CBAS__type__pstr* __cbas_local_variable_mangled_name_o);
static inline struct __CBAS__type__str* __CBAS____method_str_____copy(struct __CBAS__type__str* __cbas_local_variable_mangled_name_this,struct __CBAS__type__str* __cbas_local_variable_mangled_name_o);
static inline struct __CBAS__type__str* __CBAS____method_str_____frompstr(struct __CBAS__type__str* __cbas_local_variable_mangled_name_this,struct __CBAS__type__pstr* __cbas_local_variable_mangled_name_o);
static inline struct __CBAS__type__pstr* __CBAS____method_pstr_____copy(struct __CBAS__type__pstr* __cbas_local_variable_mangled_name_this,struct __CBAS__type__pstr* __cbas_local_variable_mangled_name_o);
static inline struct __CBAS__type__pstr* __CBAS____method_pstr_____fromstr(struct __CBAS__type__pstr* __cbas_local_variable_mangled_name_this,struct __CBAS__type__str* __cbas_local_variable_mangled_name_o);
static inline struct __CBAS__type__str* __CBAS____method_str_____new(struct __CBAS__type__str* __cbas_local_variable_mangled_name_this,unsigned char* __cbas_local_variable_mangled_name_o);
static inline struct __CBAS__type__pstr* __CBAS____method_pstr_____new(struct __CBAS__type__pstr* __cbas_local_variable_mangled_name_this,unsigned char* __cbas_local_variable_mangled_name_o);
static inline void __CBAS____method_str_____copyto(struct __CBAS__type__str* __cbas_local_variable_mangled_name_this,struct __CBAS__type__str* __cbas_local_variable_mangled_name_o);
static inline void __CBAS____method_pstr_____copyto(struct __CBAS__type__pstr* __cbas_local_variable_mangled_name_this,struct __CBAS__type__pstr* __cbas_local_variable_mangled_name_o);
static inline void __CBAS____method_str_____move(struct __CBAS__type__str* __cbas_local_variable_mangled_name_this,struct __CBAS__type__str* __cbas_local_variable_mangled_name_o);
static inline void __CBAS____method_pstr_____move(struct __CBAS__type__pstr* __cbas_local_variable_mangled_name_this,struct __CBAS__type__pstr* __cbas_local_variable_mangled_name_o);
static inline void __CBAS____method_str_____moveto(struct __CBAS__type__str* __cbas_local_variable_mangled_name_this,struct __CBAS__type__str* __cbas_local_variable_mangled_name_o);
static inline void __CBAS____method_pstr_____moveto(struct __CBAS__type__pstr* __cbas_local_variable_mangled_name_this,struct __CBAS__type__pstr* __cbas_local_variable_mangled_name_o);
static inline struct __CBAS__type__str* __CBAS____method_str_____clone(struct __CBAS__type__str* __cbas_local_variable_mangled_name_this);
static inline struct __CBAS__type__pstr* __CBAS____method_pstr_____clone(struct __CBAS__type__pstr* __cbas_local_variable_mangled_name_this);
static inline unsigned long long __CBAS____method_str_____isnull(struct __CBAS__type__str* __cbas_local_variable_mangled_name_this);
static inline unsigned long long __CBAS____method_pstr_____isnull(struct __CBAS__type__pstr* __cbas_local_variable_mangled_name_this);
static inline unsigned long long __CBAS____method_str_____isempty(struct __CBAS__type__str* __cbas_local_variable_mangled_name_this);
static inline unsigned long long __CBAS____method_str_____empty(struct __CBAS__type__str* __cbas_local_variable_mangled_name_this);
static inline unsigned long long __CBAS____method_pstr_____isempty(struct __CBAS__type__pstr* __cbas_local_variable_mangled_name_this);
static inline unsigned long long __CBAS____method_pstr_____empty(struct __CBAS__type__pstr* __cbas_local_variable_mangled_name_this);
static inline unsigned long long __CBAS____method_str_____prefixed(struct __CBAS__type__str* __cbas_local_variable_mangled_name_this,struct __CBAS__type__str* __cbas_local_variable_mangled_name_o);
static inline unsigned long long __CBAS____method_pstr_____prefixed(struct __CBAS__type__pstr* __cbas_local_variable_mangled_name_this,struct __CBAS__type__pstr* __cbas_local_variable_mangled_name_o);
static inline unsigned long long __CBAS____method_str_____postfixed(struct __CBAS__type__str* __cbas_local_variable_mangled_name_this,struct __CBAS__type__str* __cbas_local_variable_mangled_name_o);
static inline unsigned long long __CBAS____method_pstr_____postfixed(struct __CBAS__type__pstr* __cbas_local_variable_mangled_name_this,struct __CBAS__type__pstr* __cbas_local_variable_mangled_name_o);
static inline long long __CBAS____method_str_____findfrom(struct __CBAS__type__str* __cbas_local_variable_mangled_name_this,struct __CBAS__type__str* __cbas_local_variable_mangled_name_findme,unsigned long long __cbas_local_variable_mangled_name_where);
static inline long long __CBAS____method_pstr_____findfrom(struct __CBAS__type__pstr* __cbas_local_variable_mangled_name_this,struct __CBAS__type__pstr* __cbas_local_variable_mangled_name_findme,unsigned long long __cbas_local_variable_mangled_name_where);
static inline long long __CBAS____method_str_____find(struct __CBAS__type__str* __cbas_local_variable_mangled_name_this,struct __CBAS__type__str* __cbas_local_variable_mangled_name_findme);
static inline long long __CBAS____method_pstr_____find(struct __CBAS__type__pstr* __cbas_local_variable_mangled_name_this,struct __CBAS__type__pstr* __cbas_local_variable_mangled_name_findme);
static inline unsigned long long __CBAS____method_str_____add(struct __CBAS__type__str* __cbas_local_variable_mangled_name_this,struct __CBAS__type__str* __cbas_local_variable_mangled_name_o);
static inline unsigned long long __CBAS____method_pstr_____add(struct __CBAS__type__pstr* __cbas_local_variable_mangled_name_this,struct __CBAS__type__pstr* __cbas_local_variable_mangled_name_o);
static inline unsigned long long __CBAS____method_str_____addc(struct __CBAS__type__str* __cbas_local_variable_mangled_name_this,unsigned char* __cbas_local_variable_mangled_name_oo);
static inline unsigned long long __CBAS____method_pstr_____addc(struct __CBAS__type__pstr* __cbas_local_variable_mangled_name_this,unsigned char* __cbas_local_variable_mangled_name_oo);
static inline struct __CBAS__type__str* __CBAS____method_str_____substr(struct __CBAS__type__str* __cbas_local_variable_mangled_name_this,unsigned long long __cbas_local_variable_mangled_name_beginning,unsigned long long __cbas_local_variable_mangled_name_howmuch);
static inline struct __CBAS__type__pstr* __CBAS____method_pstr_____substr(struct __CBAS__type__pstr* __cbas_local_variable_mangled_name_this,unsigned long long __cbas_local_variable_mangled_name_beginning,unsigned long long __cbas_local_variable_mangled_name_howmuch);
static inline long long __CBAS____method_str_____replace_first_from(struct __CBAS__type__str* __cbas_local_variable_mangled_name_this,struct __CBAS__type__str* __cbas_local_variable_mangled_name_what,struct __CBAS__type__str* __cbas_local_variable_mangled_name_with,unsigned long long __cbas_local_variable_mangled_name_maxwhere);
static inline long long __CBAS____method_pstr_____replace_first_from(struct __CBAS__type__pstr* __cbas_local_variable_mangled_name_this,struct __CBAS__type__pstr* __cbas_local_variable_mangled_name_what,struct __CBAS__type__pstr* __cbas_local_variable_mangled_name_with,unsigned long long __cbas_local_variable_mangled_name_maxwhere);
static inline long long __CBAS____method_str_____replace_first(struct __CBAS__type__str* __cbas_local_variable_mangled_name_this,struct __CBAS__type__str* __cbas_local_variable_mangled_name_what,struct __CBAS__type__str* __cbas_local_variable_mangled_name_with);
static inline long long __CBAS____method_pstr_____replace_first(struct __CBAS__type__pstr* __cbas_local_variable_mangled_name_this,struct __CBAS__type__pstr* __cbas_local_variable_mangled_name_what,struct __CBAS__type__pstr* __cbas_local_variable_mangled_name_with);
static inline long long __CBAS____method_str_____replace(struct __CBAS__type__str* __cbas_local_variable_mangled_name_this,struct __CBAS__type__str* __cbas_local_variable_mangled_name_what,struct __CBAS__type__str* __cbas_local_variable_mangled_name_with);
static inline long long __CBAS____method_pstr_____replace(struct __CBAS__type__pstr* __cbas_local_variable_mangled_name_this,struct __CBAS__type__pstr* __cbas_local_variable_mangled_name_what,struct __CBAS__type__pstr* __cbas_local_variable_mangled_name_with);
static inline void __CBAS__utoa(unsigned char* __cbas_local_variable_mangled_name_dest,unsigned long long __cbas_local_variable_mangled_name_value);
static inline void __CBAS__itoa(unsigned char* __cbas_local_variable_mangled_name_dest,long long __cbas_local_variable_mangled_name_value);
static inline void __CBAS__ftoa_n(unsigned char* __cbas_local_variable_mangled_name_dest,double __cbas_local_variable_mangled_name_v,int __cbas_local_variable_mangled_name_ndigits);
static inline void __CBAS__ftoa(unsigned char* __cbas_local_variable_mangled_name_dest,double __cbas_local_variable_mangled_name_v);
static void __CBAS__mutoa(unsigned char* __cbas_local_variable_mangled_name_dest,unsigned int __cbas_local_variable_mangled_name_value);
static unsigned int __CBAS__matou(unsigned char* __cbas_local_variable_mangled_name_in);
static inline unsigned int __CBAS__fib(unsigned int __cbas_local_variable_mangled_name_n);
int main(int __cbas_local_variable_mangled_name_argc,char** __cbas_local_variable_mangled_name_argv);
/*
FUNCTION BODIES
*/
static inline long long __CBAS__impl_streq(unsigned char* __cbas_local_variable_mangled_name_a,unsigned char* __cbas_local_variable_mangled_name_b)
{
    
    return (__CBAS__strcmp(((unsigned char*)(__cbas_local_variable_mangled_name_a)),((unsigned char*)(__cbas_local_variable_mangled_name_b)))) == ((long long)(0ull));
    
}
static inline long long __CBAS__memeq(unsigned char* __cbas_local_variable_mangled_name_a,unsigned char* __cbas_local_variable_mangled_name_b,unsigned long long __cbas_local_variable_mangled_name_l)
{
    
    return (__CBAS__memcmp(((unsigned char*)(__cbas_local_variable_mangled_name_a)),((unsigned char*)(__cbas_local_variable_mangled_name_b)),((unsigned long long)(__cbas_local_variable_mangled_name_l)))) == ((long long)(0ull));
    
}
static inline void __CBAS____method_str_____ctor(struct __CBAS__type__str* __cbas_local_variable_mangled_name_this)
{
    ((((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_this))->__CBAS__member__d)) = ((unsigned char*)(0ull));
    ((((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_this))->__CBAS__member__len)) = (0ull);
    
}
static inline void __CBAS____method_str_____dtor(struct __CBAS__type__str* __cbas_local_variable_mangled_name_this)
{
    if(((unsigned char*)((((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_this))->__CBAS__member__d))) != ((unsigned char*)(0ull))){

    __CBAS__free(((unsigned char*)((((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_this))->__CBAS__member__d))));
    }

    __CBAS____method_str_____ctor(((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_this)));
    
}
static inline void __CBAS____method_pstr_____ctor(struct __CBAS__type__pstr* __cbas_local_variable_mangled_name_this)
{
    ((((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_this))->__CBAS__member__d)) = ((unsigned char*)(0ull));
    ((((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_this))->__CBAS__member__len)) = (0ull);
    
}
static inline void __CBAS____method_pstr_____dtor(struct __CBAS__type__pstr* __cbas_local_variable_mangled_name_this)
{
    if(((unsigned char*)((((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_this))->__CBAS__member__d))) != ((unsigned char*)(0ull))){

    __CBAS__free(((unsigned char*)((((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_this))->__CBAS__member__d))));
    }

    __CBAS____method_pstr_____ctor(((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_this)));
    
}
static inline void __CBAS____method_str_____free(struct __CBAS__type__str* __cbas_local_variable_mangled_name_this)
{
    __CBAS____method_str_____dtor(((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_this)));
    __CBAS__free(((unsigned char*)(__cbas_local_variable_mangled_name_this)));
    
}
static inline void __CBAS____method_pstr_____free(struct __CBAS__type__pstr* __cbas_local_variable_mangled_name_this)
{
    __CBAS____method_pstr_____dtor(((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_this)));
    __CBAS__free(((unsigned char*)(__cbas_local_variable_mangled_name_this)));
    
}
static inline long long __CBAS____method_str_____equal(struct __CBAS__type__str* __cbas_local_variable_mangled_name_this,struct __CBAS__type__str* __cbas_local_variable_mangled_name_o)
{
    if(((unsigned long long)((((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_this))->__CBAS__member__len))) != ((unsigned long long)((((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_o))->__CBAS__member__len)))){

    
    return (long long)(0ull);
    }

    if(((unsigned long long)((((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_this))->__CBAS__member__len))) == (0ull)){

    
    return (long long)(1ull);
    }

    
    return (__CBAS__memcmp(((unsigned char*)((((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_this))->__CBAS__member__d))),((unsigned char*)((((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_o))->__CBAS__member__d))),((unsigned long long)((((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_this))->__CBAS__member__len))))) == ((long long)(0ull));
    
}
static inline long long __CBAS____method_str_____nequal(struct __CBAS__type__str* __cbas_local_variable_mangled_name_this,struct __CBAS__type__str* __cbas_local_variable_mangled_name_o)
{
    
    return (long long)(!(__CBAS____method_str_____equal(((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_this)),((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_o)))));
    
}
static inline long long __CBAS____method_pstr_____equal(struct __CBAS__type__pstr* __cbas_local_variable_mangled_name_this,struct __CBAS__type__pstr* __cbas_local_variable_mangled_name_o)
{
    if(((unsigned long long)((((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_this))->__CBAS__member__len))) != ((unsigned long long)((((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_o))->__CBAS__member__len)))){

    
    return (long long)(0ull);
    }

    if(((unsigned long long)((((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_this))->__CBAS__member__len))) == (0ull)){

    
    return (long long)(1ull);
    }

    
    return (__CBAS__memcmp(((unsigned char*)((((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_this))->__CBAS__member__d))),((unsigned char*)((((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_o))->__CBAS__member__d))),((unsigned long long)((((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_this))->__CBAS__member__len))))) == ((long long)(0ull));
    
}
static inline long long __CBAS____method_pstr_____nequal(struct __CBAS__type__pstr* __cbas_local_variable_mangled_name_this,struct __CBAS__type__pstr* __cbas_local_variable_mangled_name_o)
{
    
    return (long long)(!(__CBAS____method_pstr_____equal(((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_this)),((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_o)))));
    
}
static inline struct __CBAS__type__str* __CBAS____method_str_____copy(struct __CBAS__type__str* __cbas_local_variable_mangled_name_this,struct __CBAS__type__str* __cbas_local_variable_mangled_name_o)
{
    __CBAS____method_str_____dtor(((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_this)));
    ((((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_this))->__CBAS__member__len)) = ((unsigned long long)((((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_o))->__CBAS__member__len)));
    ((((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_this))->__CBAS__member__d)) = (__CBAS__realloc(((unsigned char*)(0ull)),(((unsigned long long)((((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_o))->__CBAS__member__len))) + (1ull))));
    if(((unsigned char*)((((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_this))->__CBAS__member__d))) == ((unsigned char*)(0ull))){

    
    return (struct __CBAS__type__str*)(0ull);
    }

    __CBAS__mcpy(((unsigned char*)((((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_this))->__CBAS__member__d))),((unsigned char*)((((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_o))->__CBAS__member__d))),(((unsigned long long)((((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_this))->__CBAS__member__len))) + (1ull)));
    
    return (struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_this);
    
}
static inline struct __CBAS__type__str* __CBAS____method_str_____frompstr(struct __CBAS__type__str* __cbas_local_variable_mangled_name_this,struct __CBAS__type__pstr* __cbas_local_variable_mangled_name_o)
{
    __CBAS____method_str_____dtor(((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_this)));
    ((((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_this))->__CBAS__member__len)) = ((unsigned long long)((((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_o))->__CBAS__member__len)));
    ((((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_this))->__CBAS__member__d)) = (__CBAS__realloc(((unsigned char*)(0ull)),(((unsigned long long)((((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_o))->__CBAS__member__len))) + (1ull))));
    if(((unsigned char*)((((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_this))->__CBAS__member__d))) == ((unsigned char*)(0ull))){

    
    return (struct __CBAS__type__str*)(0ull);
    }

    __CBAS__mcpy(((unsigned char*)((((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_this))->__CBAS__member__d))),((unsigned char*)((((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_o))->__CBAS__member__d))),((unsigned long long)((((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_this))->__CBAS__member__len))));
    (((unsigned char*)((((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_this))->__CBAS__member__d)))[(long long)((((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_this))->__CBAS__member__len))]) = ((unsigned char)(0ull));
    ((((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_this))->__CBAS__member__len)) = (__CBAS__strlen(((unsigned char*)((((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_this))->__CBAS__member__d)))));
    
    return (struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_this);
    
}
static inline struct __CBAS__type__pstr* __CBAS____method_pstr_____copy(struct __CBAS__type__pstr* __cbas_local_variable_mangled_name_this,struct __CBAS__type__pstr* __cbas_local_variable_mangled_name_o)
{
    __CBAS____method_pstr_____dtor(((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_this)));
    ((((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_this))->__CBAS__member__len)) = ((unsigned long long)((((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_o))->__CBAS__member__len)));
    ((((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_this))->__CBAS__member__d)) = (__CBAS__realloc(((unsigned char*)(0ull)),((unsigned long long)((((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_o))->__CBAS__member__len)))));
    if(((unsigned char*)((((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_this))->__CBAS__member__d))) == ((unsigned char*)(0ull))){

    
    return (struct __CBAS__type__pstr*)(0ull);
    }

    __CBAS__mcpy(((unsigned char*)((((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_this))->__CBAS__member__d))),((unsigned char*)((((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_o))->__CBAS__member__d))),((unsigned long long)((((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_this))->__CBAS__member__len))));
    
    return (struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_this);
    
}
static inline struct __CBAS__type__pstr* __CBAS____method_pstr_____fromstr(struct __CBAS__type__pstr* __cbas_local_variable_mangled_name_this,struct __CBAS__type__str* __cbas_local_variable_mangled_name_o)
{
    __CBAS____method_pstr_____dtor(((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_this)));
    ((((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_this))->__CBAS__member__len)) = ((unsigned long long)((((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_o))->__CBAS__member__len)));
    ((((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_this))->__CBAS__member__d)) = (__CBAS__realloc(((unsigned char*)(0ull)),((unsigned long long)((((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_o))->__CBAS__member__len)))));
    if(((unsigned char*)((((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_this))->__CBAS__member__d))) == ((unsigned char*)(0ull))){

    
    return (struct __CBAS__type__pstr*)(0ull);
    }

    __CBAS__mcpy(((unsigned char*)((((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_this))->__CBAS__member__d))),((unsigned char*)((((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_o))->__CBAS__member__d))),((unsigned long long)((((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_this))->__CBAS__member__len))));
    
    return (struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_this);
    
}
static inline struct __CBAS__type__str* __CBAS____method_str_____new(struct __CBAS__type__str* __cbas_local_variable_mangled_name_this,unsigned char* __cbas_local_variable_mangled_name_o)
{
    unsigned long long __cbas_local_variable_mangled_name_qq;
    __CBAS____method_str_____dtor(((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_this)));
    (__cbas_local_variable_mangled_name_qq) = (__CBAS__strlen(((unsigned char*)(__cbas_local_variable_mangled_name_o))));
    ((((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_this))->__CBAS__member__len)) = ((unsigned long long)(__cbas_local_variable_mangled_name_qq));
    ((((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_this))->__CBAS__member__d)) = (__CBAS__realloc(((unsigned char*)(0ull)),(((unsigned long long)(__cbas_local_variable_mangled_name_qq)) + (1ull))));
    if(((unsigned char*)((((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_this))->__CBAS__member__d))) == ((unsigned char*)(0ull))){

    
    return (struct __CBAS__type__str*)(0ull);
    }

    __CBAS__mcpy(((unsigned char*)((((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_this))->__CBAS__member__d))),((unsigned char*)(__cbas_local_variable_mangled_name_o)),(((unsigned long long)(__cbas_local_variable_mangled_name_qq)) + (1ull)));
    
    return (struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_this);
    
}
static inline struct __CBAS__type__pstr* __CBAS____method_pstr_____new(struct __CBAS__type__pstr* __cbas_local_variable_mangled_name_this,unsigned char* __cbas_local_variable_mangled_name_o)
{
    unsigned long long __cbas_local_variable_mangled_name_qq;
    __CBAS____method_pstr_____dtor(((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_this)));
    (__cbas_local_variable_mangled_name_qq) = (__CBAS__strlen(((unsigned char*)(__cbas_local_variable_mangled_name_o))));
    ((((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_this))->__CBAS__member__len)) = ((unsigned long long)(__cbas_local_variable_mangled_name_qq));
    ((((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_this))->__CBAS__member__d)) = (__CBAS__realloc(((unsigned char*)(0ull)),((unsigned long long)(__cbas_local_variable_mangled_name_qq))));
    if(((unsigned char*)((((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_this))->__CBAS__member__d))) == ((unsigned char*)(0ull))){

    
    return (struct __CBAS__type__pstr*)(0ull);
    }

    __CBAS__mcpy(((unsigned char*)((((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_this))->__CBAS__member__d))),((unsigned char*)(__cbas_local_variable_mangled_name_o)),((unsigned long long)(__cbas_local_variable_mangled_name_qq)));
    
    return (struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_this);
    
}
static inline void __CBAS____method_str_____copyto(struct __CBAS__type__str* __cbas_local_variable_mangled_name_this,struct __CBAS__type__str* __cbas_local_variable_mangled_name_o)
{
    __CBAS____method_str_____copy(((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_o)),((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_this)));
    
}
static inline void __CBAS____method_pstr_____copyto(struct __CBAS__type__pstr* __cbas_local_variable_mangled_name_this,struct __CBAS__type__pstr* __cbas_local_variable_mangled_name_o)
{
    __CBAS____method_pstr_____copy(((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_o)),((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_this)));
    
}
static inline void __CBAS____method_str_____move(struct __CBAS__type__str* __cbas_local_variable_mangled_name_this,struct __CBAS__type__str* __cbas_local_variable_mangled_name_o)
{
    ((((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_this))->__CBAS__member__d)) = ((unsigned char*)((((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_o))->__CBAS__member__d)));
    ((((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_this))->__CBAS__member__len)) = ((unsigned long long)((((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_o))->__CBAS__member__len)));
    ((((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_o))->__CBAS__member__d)) = ((unsigned char*)(0ull));
    ((((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_o))->__CBAS__member__len)) = (0ull);
    
}
static inline void __CBAS____method_pstr_____move(struct __CBAS__type__pstr* __cbas_local_variable_mangled_name_this,struct __CBAS__type__pstr* __cbas_local_variable_mangled_name_o)
{
    ((((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_this))->__CBAS__member__d)) = ((unsigned char*)((((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_o))->__CBAS__member__d)));
    ((((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_this))->__CBAS__member__len)) = ((unsigned long long)((((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_o))->__CBAS__member__len)));
    ((((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_o))->__CBAS__member__d)) = ((unsigned char*)(0ull));
    ((((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_o))->__CBAS__member__len)) = (0ull);
    
}
static inline void __CBAS____method_str_____moveto(struct __CBAS__type__str* __cbas_local_variable_mangled_name_this,struct __CBAS__type__str* __cbas_local_variable_mangled_name_o)
{
    __CBAS____method_str_____move(((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_o)),((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_this)));
    
}
static inline void __CBAS____method_pstr_____moveto(struct __CBAS__type__pstr* __cbas_local_variable_mangled_name_this,struct __CBAS__type__pstr* __cbas_local_variable_mangled_name_o)
{
    __CBAS____method_pstr_____move(((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_o)),((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_this)));
    
}
static inline struct __CBAS__type__str* __CBAS____method_str_____clone(struct __CBAS__type__str* __cbas_local_variable_mangled_name_this)
{
    struct __CBAS__type__str* __cbas_local_variable_mangled_name_p;
    (__cbas_local_variable_mangled_name_p) = ((struct __CBAS__type__str*)(__CBAS__malloc((sizeof(struct __CBAS__type__str)))));
    if(((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_p)) == ((struct __CBAS__type__str*)(0ull))){

    
    return (struct __CBAS__type__str*)(0ull);
    }

    __CBAS____method_str_____copy(((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_p)),((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_this)));
    
    return (struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_p);
    
}
static inline struct __CBAS__type__pstr* __CBAS____method_pstr_____clone(struct __CBAS__type__pstr* __cbas_local_variable_mangled_name_this)
{
    struct __CBAS__type__pstr* __cbas_local_variable_mangled_name_p;
    (__cbas_local_variable_mangled_name_p) = ((struct __CBAS__type__pstr*)(__CBAS__malloc((sizeof(struct __CBAS__type__pstr)))));
    if(((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_p)) == ((struct __CBAS__type__pstr*)(0ull))){

    
    return (struct __CBAS__type__pstr*)(0ull);
    }

    __CBAS____method_pstr_____copy(((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_p)),((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_this)));
    
    return (struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_p);
    
}
static inline unsigned long long __CBAS____method_str_____isnull(struct __CBAS__type__str* __cbas_local_variable_mangled_name_this)
{
    
    return (unsigned long long)(((unsigned char*)((((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_this))->__CBAS__member__d))) == ((unsigned char*)(0ull)));
    
}
static inline unsigned long long __CBAS____method_pstr_____isnull(struct __CBAS__type__pstr* __cbas_local_variable_mangled_name_this)
{
    
    return (unsigned long long)(((unsigned char*)((((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_this))->__CBAS__member__d))) == ((unsigned char*)(0ull)));
    
}
static inline unsigned long long __CBAS____method_str_____isempty(struct __CBAS__type__str* __cbas_local_variable_mangled_name_this)
{
    if((long long)(((unsigned long long)(((unsigned long long)((((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_this))->__CBAS__member__len))) == (0ull))) || ((unsigned long long)(((unsigned char*)((((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_this))->__CBAS__member__d))) == ((unsigned char*)(0ull)))))){

    
    return 1ull;
    }

    
    return 0ull;
    
}
static inline unsigned long long __CBAS____method_str_____empty(struct __CBAS__type__str* __cbas_local_variable_mangled_name_this)
{
    
    return __CBAS____method_str_____isempty(((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_this)));
    
}
static inline unsigned long long __CBAS____method_pstr_____isempty(struct __CBAS__type__pstr* __cbas_local_variable_mangled_name_this)
{
    if((long long)(((unsigned long long)(((unsigned long long)((((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_this))->__CBAS__member__len))) == (0ull))) || ((unsigned long long)(((unsigned char*)((((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_this))->__CBAS__member__d))) == ((unsigned char*)(0ull)))))){

    
    return 1ull;
    }

    
    return 0ull;
    
}
static inline unsigned long long __CBAS____method_pstr_____empty(struct __CBAS__type__pstr* __cbas_local_variable_mangled_name_this)
{
    
    return __CBAS____method_pstr_____isempty(((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_this)));
    
}
static inline unsigned long long __CBAS____method_str_____prefixed(struct __CBAS__type__str* __cbas_local_variable_mangled_name_this,struct __CBAS__type__str* __cbas_local_variable_mangled_name_o)
{
    unsigned char* __cbas_local_variable_mangled_name_a;
    unsigned char* __cbas_local_variable_mangled_name_b;
    if((long long)(__CBAS____method_str_____isempty(((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_o))))){

    
    return 1ull;
    }

    if((long long)(__CBAS____method_str_____isempty(((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_this))))){

    
    return 0ull;
    }

    if(((unsigned long long)((((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_o))->__CBAS__member__len))) > ((unsigned long long)((((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_this))->__CBAS__member__len)))){

    
    return 0ull;
    }

    (__cbas_local_variable_mangled_name_a) = ((unsigned char*)((((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_o))->__CBAS__member__d)));
    (__cbas_local_variable_mangled_name_b) = ((unsigned char*)((((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_this))->__CBAS__member__d)));
    while((long long)(1ull)){

    if(((unsigned long long)(((unsigned char*)(__cbas_local_variable_mangled_name_a))[(long long)(0ull)])) == (0ull)){

    
    return 1ull;
    }

    if(((unsigned char)(((unsigned char*)(__cbas_local_variable_mangled_name_a))[(long long)(0ull)])) != ((unsigned char)(((unsigned char*)(__cbas_local_variable_mangled_name_b))[(long long)(0ull)]))){

    
    return 0ull;
    }

    (__cbas_local_variable_mangled_name_a)++;
    (__cbas_local_variable_mangled_name_b)++;
    }

    
}
static inline unsigned long long __CBAS____method_pstr_____prefixed(struct __CBAS__type__pstr* __cbas_local_variable_mangled_name_this,struct __CBAS__type__pstr* __cbas_local_variable_mangled_name_o)
{
    unsigned char* __cbas_local_variable_mangled_name_a;
    unsigned char* __cbas_local_variable_mangled_name_b;
    unsigned long long __cbas_local_variable_mangled_name_i;
    if((long long)(__CBAS____method_pstr_____isempty(((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_o))))){

    
    return 1ull;
    }

    if((long long)(__CBAS____method_pstr_____isempty(((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_this))))){

    
    return 0ull;
    }

    if(((unsigned long long)((((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_o))->__CBAS__member__len))) > ((unsigned long long)((((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_this))->__CBAS__member__len)))){

    
    return 0ull;
    }

    (__cbas_local_variable_mangled_name_a) = ((unsigned char*)((((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_o))->__CBAS__member__d)));
    (__cbas_local_variable_mangled_name_b) = ((unsigned char*)((((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_this))->__CBAS__member__d)));
    for((__cbas_local_variable_mangled_name_i) = (0ull);((unsigned long long)(__cbas_local_variable_mangled_name_i)) < ((unsigned long long)((((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_o))->__CBAS__member__len)));(__cbas_local_variable_mangled_name_i)++){

    if(((unsigned char)(((unsigned char*)(__cbas_local_variable_mangled_name_a))[(long long)(0ull)])) != ((unsigned char)(((unsigned char*)(__cbas_local_variable_mangled_name_b))[(long long)(0ull)]))){

    
    return 0ull;
    }

    (__cbas_local_variable_mangled_name_a)++;
    (__cbas_local_variable_mangled_name_b)++;
    }

    
    return 1ull;
    
}
static inline unsigned long long __CBAS____method_str_____postfixed(struct __CBAS__type__str* __cbas_local_variable_mangled_name_this,struct __CBAS__type__str* __cbas_local_variable_mangled_name_o)
{
    unsigned char* __cbas_local_variable_mangled_name_a;
    unsigned char* __cbas_local_variable_mangled_name_b;
    if((long long)(__CBAS____method_str_____isempty(((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_o))))){

    
    return 1ull;
    }

    if((long long)(__CBAS____method_str_____isempty(((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_this))))){

    
    return 0ull;
    }

    if(((unsigned long long)((((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_o))->__CBAS__member__len))) > ((unsigned long long)((((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_this))->__CBAS__member__len)))){

    
    return 0ull;
    }

    (__cbas_local_variable_mangled_name_a) = ((unsigned char*)((((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_o))->__CBAS__member__d)));
    (__cbas_local_variable_mangled_name_b) = (((unsigned char*)((((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_this))->__CBAS__member__d))) + ((long long)(((unsigned long long)((((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_this))->__CBAS__member__len))) - ((unsigned long long)((((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_o))->__CBAS__member__len))))));
    while((long long)(1ull)){

    if(((unsigned long long)(((unsigned char*)(__cbas_local_variable_mangled_name_a))[(long long)(0ull)])) == (0ull)){

    
    return 1ull;
    }

    if(((unsigned char)(((unsigned char*)(__cbas_local_variable_mangled_name_a))[(long long)(0ull)])) != ((unsigned char)(((unsigned char*)(__cbas_local_variable_mangled_name_b))[(long long)(0ull)]))){

    
    return 0ull;
    }

    (__cbas_local_variable_mangled_name_a)++;
    (__cbas_local_variable_mangled_name_b)++;
    }

    
}
static inline unsigned long long __CBAS____method_pstr_____postfixed(struct __CBAS__type__pstr* __cbas_local_variable_mangled_name_this,struct __CBAS__type__pstr* __cbas_local_variable_mangled_name_o)
{
    unsigned char* __cbas_local_variable_mangled_name_a;
    unsigned char* __cbas_local_variable_mangled_name_b;
    unsigned long long __cbas_local_variable_mangled_name_i;
    if((long long)(__CBAS____method_pstr_____isempty(((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_o))))){

    
    return 1ull;
    }

    if((long long)(__CBAS____method_pstr_____isempty(((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_this))))){

    
    return 0ull;
    }

    if(((unsigned long long)((((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_o))->__CBAS__member__len))) > ((unsigned long long)((((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_this))->__CBAS__member__len)))){

    
    return 0ull;
    }

    (__cbas_local_variable_mangled_name_a) = ((unsigned char*)((((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_o))->__CBAS__member__d)));
    (__cbas_local_variable_mangled_name_b) = (((unsigned char*)((((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_this))->__CBAS__member__d))) + ((long long)(((unsigned long long)((((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_this))->__CBAS__member__len))) - ((unsigned long long)((((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_o))->__CBAS__member__len))))));
    for((__cbas_local_variable_mangled_name_i) = (0ull);((unsigned long long)(__cbas_local_variable_mangled_name_i)) < ((unsigned long long)((((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_o))->__CBAS__member__len)));(__cbas_local_variable_mangled_name_i)++){

    if(((unsigned char)(((unsigned char*)(__cbas_local_variable_mangled_name_a))[(long long)(0ull)])) != ((unsigned char)(((unsigned char*)(__cbas_local_variable_mangled_name_b))[(long long)(0ull)]))){

    
    return 0ull;
    }

    (__cbas_local_variable_mangled_name_a)++;
    (__cbas_local_variable_mangled_name_b)++;
    }

    
    return 1ull;
    
}
static inline long long __CBAS____method_str_____findfrom(struct __CBAS__type__str* __cbas_local_variable_mangled_name_this,struct __CBAS__type__str* __cbas_local_variable_mangled_name_findme,unsigned long long __cbas_local_variable_mangled_name_where)
{
    long long __cbas_local_variable_mangled_name_retval;
    unsigned char* __cbas_local_variable_mangled_name_text;
    unsigned char* __cbas_local_variable_mangled_name_subtext;
    long long __cbas_local_variable_mangled_name_ti;
    long long __cbas_local_variable_mangled_name_si;
    long long __cbas_local_variable_mangled_name_st;
    (__cbas_local_variable_mangled_name_retval) = (-((long long)(1ull)));
    if((long long)(__CBAS____method_str_____isempty(((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_findme))))){

    
    return (long long)(0ull);
    }

    if((long long)(__CBAS____method_str_____isempty(((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_this))))){

    
    return -((long long)(1ull));
    }

    if(((unsigned long long)((((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_findme))->__CBAS__member__len))) > ((unsigned long long)((((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_this))->__CBAS__member__len)))){

    
    return -((long long)(1ull));
    }

    if(((unsigned long long)(__cbas_local_variable_mangled_name_where)) >= ((unsigned long long)((((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_this))->__CBAS__member__len)))){

    
    return -((long long)(1ull));
    }

    (__cbas_local_variable_mangled_name_text) = ((unsigned char*)((((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_this))->__CBAS__member__d)));
    (__cbas_local_variable_mangled_name_subtext) = ((unsigned char*)((((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_findme))->__CBAS__member__d)));
    (__cbas_local_variable_mangled_name_ti) = ((long long)(__cbas_local_variable_mangled_name_where));
    (__cbas_local_variable_mangled_name_si) = ((long long)(0ull));
    (__cbas_local_variable_mangled_name_st) = ((long long)((((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_findme))->__CBAS__member__len)));
    for(0ull;((unsigned long long)(((unsigned char*)(__cbas_local_variable_mangled_name_text))[(long long)(__cbas_local_variable_mangled_name_ti)])) != (0ull);(__cbas_local_variable_mangled_name_ti)++){

    if(((unsigned char)(((unsigned char*)(__cbas_local_variable_mangled_name_text))[(long long)(__cbas_local_variable_mangled_name_ti)])) == ((unsigned char)(((unsigned char*)(__cbas_local_variable_mangled_name_subtext))[(long long)(__cbas_local_variable_mangled_name_si)]))){

    (__cbas_local_variable_mangled_name_si)++;
    if(((unsigned long long)(((unsigned char*)(__cbas_local_variable_mangled_name_subtext))[(long long)(__cbas_local_variable_mangled_name_si)])) == (0ull)){

    
    return (((long long)(__cbas_local_variable_mangled_name_ti)) - ((long long)(__cbas_local_variable_mangled_name_st))) + ((long long)(1ull));
    }

    }

    else {
    if(((unsigned long long)(((unsigned char*)(__cbas_local_variable_mangled_name_subtext))[(long long)(__cbas_local_variable_mangled_name_si)])) == (0ull)){

    
    return (((long long)(__cbas_local_variable_mangled_name_ti)) - ((long long)(__cbas_local_variable_mangled_name_st))) + ((long long)(1ull));
    }

    (__cbas_local_variable_mangled_name_ti) = (((long long)(__cbas_local_variable_mangled_name_ti)) - ((long long)(__cbas_local_variable_mangled_name_si)));
    (__cbas_local_variable_mangled_name_si) = ((long long)(0ull));
    }

    }

    
    return -((long long)(1ull));
    
}
static inline long long __CBAS____method_pstr_____findfrom(struct __CBAS__type__pstr* __cbas_local_variable_mangled_name_this,struct __CBAS__type__pstr* __cbas_local_variable_mangled_name_findme,unsigned long long __cbas_local_variable_mangled_name_where)
{
    long long __cbas_local_variable_mangled_name_retval;
    unsigned char* __cbas_local_variable_mangled_name_text;
    unsigned char* __cbas_local_variable_mangled_name_subtext;
    long long __cbas_local_variable_mangled_name_ti;
    long long __cbas_local_variable_mangled_name_si;
    long long __cbas_local_variable_mangled_name_st;
    (__cbas_local_variable_mangled_name_retval) = (-((long long)(1ull)));
    if((long long)(__CBAS____method_pstr_____isempty(((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_findme))))){

    
    return (long long)(0ull);
    }

    if((long long)(__CBAS____method_pstr_____isempty(((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_this))))){

    
    return -((long long)(1ull));
    }

    if(((unsigned long long)((((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_findme))->__CBAS__member__len))) > ((unsigned long long)((((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_this))->__CBAS__member__len)))){

    
    return -((long long)(1ull));
    }

    if(((unsigned long long)(__cbas_local_variable_mangled_name_where)) >= ((unsigned long long)((((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_this))->__CBAS__member__len)))){

    
    return -((long long)(1ull));
    }

    (__cbas_local_variable_mangled_name_text) = ((unsigned char*)((((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_this))->__CBAS__member__d)));
    (__cbas_local_variable_mangled_name_subtext) = ((unsigned char*)((((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_findme))->__CBAS__member__d)));
    (__cbas_local_variable_mangled_name_ti) = ((long long)(__cbas_local_variable_mangled_name_where));
    (__cbas_local_variable_mangled_name_si) = ((long long)(0ull));
    (__cbas_local_variable_mangled_name_st) = ((long long)((((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_findme))->__CBAS__member__len)));
    for(0ull;((long long)(__cbas_local_variable_mangled_name_ti)) < ((long long)((((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_this))->__CBAS__member__len)));(__cbas_local_variable_mangled_name_ti)++){

    if(((unsigned char)(((unsigned char*)(__cbas_local_variable_mangled_name_text))[(long long)(__cbas_local_variable_mangled_name_ti)])) == ((unsigned char)(((unsigned char*)(__cbas_local_variable_mangled_name_subtext))[(long long)(__cbas_local_variable_mangled_name_si)]))){

    (__cbas_local_variable_mangled_name_si)++;
    if(((long long)(__cbas_local_variable_mangled_name_si)) >= ((long long)((((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_findme))->__CBAS__member__len)))){

    
    return (((long long)(__cbas_local_variable_mangled_name_ti)) - ((long long)(__cbas_local_variable_mangled_name_st))) + ((long long)(1ull));
    }

    }

    else {
    if(((long long)(__cbas_local_variable_mangled_name_si)) >= ((long long)((((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_findme))->__CBAS__member__len)))){

    
    return (((long long)(__cbas_local_variable_mangled_name_ti)) - ((long long)(__cbas_local_variable_mangled_name_st))) + ((long long)(1ull));
    }

    (__cbas_local_variable_mangled_name_ti) = (((long long)(__cbas_local_variable_mangled_name_ti)) - ((long long)(__cbas_local_variable_mangled_name_si)));
    (__cbas_local_variable_mangled_name_si) = ((long long)(0ull));
    }

    }

    
    return -((long long)(1ull));
    
}
static inline long long __CBAS____method_str_____find(struct __CBAS__type__str* __cbas_local_variable_mangled_name_this,struct __CBAS__type__str* __cbas_local_variable_mangled_name_findme)
{
    
    return __CBAS____method_str_____findfrom(((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_this)),((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_findme)),(0ull));
    
}
static inline long long __CBAS____method_pstr_____find(struct __CBAS__type__pstr* __cbas_local_variable_mangled_name_this,struct __CBAS__type__pstr* __cbas_local_variable_mangled_name_findme)
{
    
    return __CBAS____method_pstr_____findfrom(((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_this)),((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_findme)),(0ull));
    
}
static inline unsigned long long __CBAS____method_str_____add(struct __CBAS__type__str* __cbas_local_variable_mangled_name_this,struct __CBAS__type__str* __cbas_local_variable_mangled_name_o)
{
    unsigned long long __cbas_local_variable_mangled_name_old_len;
    (__cbas_local_variable_mangled_name_old_len) = ((unsigned long long)((((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_this))->__CBAS__member__len)));
    if((long long)(__CBAS____method_str_____isempty(((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_o))))){

    
    return 0ull;
    }

    ((((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_this))->__CBAS__member__len)) = ((((unsigned long long)((((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_o))->__CBAS__member__len))) + ((unsigned long long)((((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_this))->__CBAS__member__len)))) + (1ull));
    ((((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_this))->__CBAS__member__d)) = (__CBAS__realloc(((unsigned char*)((((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_this))->__CBAS__member__d))),((((unsigned long long)((((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_this))->__CBAS__member__len))) + ((unsigned long long)((((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_o))->__CBAS__member__len)))) + (1ull))));
    if(((unsigned char*)((((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_this))->__CBAS__member__d))) == ((unsigned char*)(0ull))){

    
    return 1ull;
    }

    __CBAS__mcpy((((unsigned char*)((((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_this))->__CBAS__member__d))) + ((long long)(__cbas_local_variable_mangled_name_old_len))),((unsigned char*)((((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_o))->__CBAS__member__d))),(((unsigned long long)((((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_o))->__CBAS__member__len))) + (1ull)));
    
    return 0ull;
    
}
static inline unsigned long long __CBAS____method_pstr_____add(struct __CBAS__type__pstr* __cbas_local_variable_mangled_name_this,struct __CBAS__type__pstr* __cbas_local_variable_mangled_name_o)
{
    unsigned long long __cbas_local_variable_mangled_name_old_len;
    (__cbas_local_variable_mangled_name_old_len) = ((unsigned long long)((((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_this))->__CBAS__member__len)));
    if((long long)(__CBAS____method_pstr_____isempty(((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_o))))){

    
    return 0ull;
    }

    ((((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_this))->__CBAS__member__len)) = (((unsigned long long)((((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_o))->__CBAS__member__len))) + ((unsigned long long)((((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_this))->__CBAS__member__len))));
    ((((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_this))->__CBAS__member__d)) = (__CBAS__realloc(((unsigned char*)((((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_this))->__CBAS__member__d))),(((unsigned long long)((((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_this))->__CBAS__member__len))) + ((unsigned long long)((((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_o))->__CBAS__member__len))))));
    if(((unsigned char*)((((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_this))->__CBAS__member__d))) == ((unsigned char*)(0ull))){

    
    return 1ull;
    }

    __CBAS__mcpy((((unsigned char*)((((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_this))->__CBAS__member__d))) + ((long long)(__cbas_local_variable_mangled_name_old_len))),((unsigned char*)((((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_o))->__CBAS__member__d))),((unsigned long long)((((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_o))->__CBAS__member__len))));
    
    return 0ull;
    
}
static inline unsigned long long __CBAS____method_str_____addc(struct __CBAS__type__str* __cbas_local_variable_mangled_name_this,unsigned char* __cbas_local_variable_mangled_name_oo)
{
    unsigned long long __cbas_local_variable_mangled_name_qq;
    unsigned long long __cbas_local_variable_mangled_name_old_len;
    (__cbas_local_variable_mangled_name_qq) = (0ull);
    if(((unsigned char*)(__cbas_local_variable_mangled_name_oo)) == ((unsigned char*)(0ull))){

    
    return 0ull;
    }

    (__cbas_local_variable_mangled_name_qq) = (__CBAS__strlen(((unsigned char*)(__cbas_local_variable_mangled_name_oo))));
    if(((unsigned long long)(__cbas_local_variable_mangled_name_qq)) == (0ull)){

    
    return 0ull;
    }

    (__cbas_local_variable_mangled_name_old_len) = ((unsigned long long)((((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_this))->__CBAS__member__len)));
    ((((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_this))->__CBAS__member__len)) = ((((unsigned long long)(__cbas_local_variable_mangled_name_qq)) + ((unsigned long long)((((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_this))->__CBAS__member__len)))) + (1ull));
    ((((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_this))->__CBAS__member__d)) = (__CBAS__realloc(((unsigned char*)((((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_this))->__CBAS__member__d))),((((unsigned long long)((((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_this))->__CBAS__member__len))) + ((unsigned long long)(__cbas_local_variable_mangled_name_qq))) + (1ull))));
    if(((unsigned char*)((((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_this))->__CBAS__member__d))) == ((unsigned char*)(0ull))){

    
    return 1ull;
    }

    __CBAS__mcpy((((unsigned char*)((((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_this))->__CBAS__member__d))) + ((long long)(__cbas_local_variable_mangled_name_old_len))),((unsigned char*)(__cbas_local_variable_mangled_name_oo)),(((unsigned long long)(__cbas_local_variable_mangled_name_qq)) + (1ull)));
    
    return 0ull;
    
}
static inline unsigned long long __CBAS____method_pstr_____addc(struct __CBAS__type__pstr* __cbas_local_variable_mangled_name_this,unsigned char* __cbas_local_variable_mangled_name_oo)
{
    unsigned long long __cbas_local_variable_mangled_name_qq;
    unsigned long long __cbas_local_variable_mangled_name_old_len;
    (__cbas_local_variable_mangled_name_qq) = (0ull);
    if(((unsigned char*)(__cbas_local_variable_mangled_name_oo)) == ((unsigned char*)(0ull))){

    
    return 0ull;
    }

    (__cbas_local_variable_mangled_name_qq) = (__CBAS__strlen(((unsigned char*)(__cbas_local_variable_mangled_name_oo))));
    if(((unsigned long long)(__cbas_local_variable_mangled_name_qq)) == (0ull)){

    
    return 0ull;
    }

    (__cbas_local_variable_mangled_name_old_len) = ((unsigned long long)((((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_this))->__CBAS__member__len)));
    ((((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_this))->__CBAS__member__len)) = (((unsigned long long)(__cbas_local_variable_mangled_name_qq)) + ((unsigned long long)((((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_this))->__CBAS__member__len))));
    ((((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_this))->__CBAS__member__d)) = (__CBAS__realloc(((unsigned char*)((((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_this))->__CBAS__member__d))),(((unsigned long long)((((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_this))->__CBAS__member__len))) + ((unsigned long long)(__cbas_local_variable_mangled_name_qq)))));
    if(((unsigned char*)((((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_this))->__CBAS__member__d))) == ((unsigned char*)(0ull))){

    
    return 1ull;
    }

    __CBAS__mcpy((((unsigned char*)((((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_this))->__CBAS__member__d))) + ((long long)(__cbas_local_variable_mangled_name_old_len))),((unsigned char*)(__cbas_local_variable_mangled_name_oo)),((unsigned long long)(__cbas_local_variable_mangled_name_qq)));
    
    return 0ull;
    
}
static inline struct __CBAS__type__str* __CBAS____method_str_____substr(struct __CBAS__type__str* __cbas_local_variable_mangled_name_this,unsigned long long __cbas_local_variable_mangled_name_beginning,unsigned long long __cbas_local_variable_mangled_name_howmuch)
{
    unsigned char* __cbas_local_variable_mangled_name_b;
    struct __CBAS__type__str* __cbas_local_variable_mangled_name_p;
    if((long long)(((unsigned long long)(((unsigned long long)(__cbas_local_variable_mangled_name_beginning)) >= ((unsigned long long)((((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_this))->__CBAS__member__len))))) || ((unsigned long long)((((unsigned long long)(__cbas_local_variable_mangled_name_beginning)) + ((unsigned long long)(__cbas_local_variable_mangled_name_howmuch))) >= ((unsigned long long)((((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_this))->__CBAS__member__len))))))){

    
    return (struct __CBAS__type__str*)(0ull);
    }

    if(((unsigned long long)(__cbas_local_variable_mangled_name_howmuch)) == (0ull)){

    struct __CBAS__type__str* __cbas_local_variable_mangled_name_p2;
    (__cbas_local_variable_mangled_name_p2) = ((struct __CBAS__type__str*)(__CBAS__malloc((sizeof(struct __CBAS__type__str)))));
    if(((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_p2)) == ((struct __CBAS__type__str*)(0ull))){

    
    return (struct __CBAS__type__str*)(0ull);
    }

    __CBAS____method_str_____ctor(((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_p2)));
    
    return (struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_p2);
    }

    (__cbas_local_variable_mangled_name_b) = (__CBAS__malloc((((unsigned long long)(__cbas_local_variable_mangled_name_howmuch)) + (1ull))));
    if(((unsigned char*)(__cbas_local_variable_mangled_name_b)) == ((unsigned char*)(0ull))){

    
    return (struct __CBAS__type__str*)(0ull);
    }

    __CBAS__mcpy(((unsigned char*)(__cbas_local_variable_mangled_name_b)),(((unsigned char*)((((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_this))->__CBAS__member__d))) + ((long long)(__cbas_local_variable_mangled_name_beginning))),((unsigned long long)(__cbas_local_variable_mangled_name_howmuch)));
    (((unsigned char*)(__cbas_local_variable_mangled_name_b))[(long long)(__cbas_local_variable_mangled_name_howmuch)]) = ((unsigned char)(0ull));
    (__cbas_local_variable_mangled_name_p) = ((struct __CBAS__type__str*)(__CBAS__malloc((sizeof(struct __CBAS__type__str)))));
    if(((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_p)) == ((struct __CBAS__type__str*)(0ull))){

    __CBAS__free(((unsigned char*)(__cbas_local_variable_mangled_name_b)));
    
    return (struct __CBAS__type__str*)(0ull);
    }

    ((((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_p))->__CBAS__member__d)) = ((unsigned char*)(__cbas_local_variable_mangled_name_b));
    ((((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_p))->__CBAS__member__len)) = ((unsigned long long)(__cbas_local_variable_mangled_name_howmuch));
    
    return (struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_p);
    
}
static inline struct __CBAS__type__pstr* __CBAS____method_pstr_____substr(struct __CBAS__type__pstr* __cbas_local_variable_mangled_name_this,unsigned long long __cbas_local_variable_mangled_name_beginning,unsigned long long __cbas_local_variable_mangled_name_howmuch)
{
    unsigned char* __cbas_local_variable_mangled_name_b;
    struct __CBAS__type__pstr* __cbas_local_variable_mangled_name_p;
    if((long long)(((unsigned long long)(((unsigned long long)(__cbas_local_variable_mangled_name_beginning)) >= ((unsigned long long)((((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_this))->__CBAS__member__len))))) || ((unsigned long long)((((unsigned long long)(__cbas_local_variable_mangled_name_beginning)) + ((unsigned long long)(__cbas_local_variable_mangled_name_howmuch))) >= ((unsigned long long)((((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_this))->__CBAS__member__len))))))){

    
    return (struct __CBAS__type__pstr*)(0ull);
    }

    if(((unsigned long long)(__cbas_local_variable_mangled_name_howmuch)) == (0ull)){

    struct __CBAS__type__pstr* __cbas_local_variable_mangled_name_p2;
    (__cbas_local_variable_mangled_name_p2) = ((struct __CBAS__type__pstr*)(__CBAS__malloc((sizeof(struct __CBAS__type__pstr)))));
    if(((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_p2)) == ((struct __CBAS__type__pstr*)(0ull))){

    
    return (struct __CBAS__type__pstr*)(0ull);
    }

    __CBAS____method_pstr_____ctor(((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_p2)));
    
    return (struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_p2);
    }

    (__cbas_local_variable_mangled_name_b) = (__CBAS__malloc(((unsigned long long)(__cbas_local_variable_mangled_name_howmuch))));
    if(((unsigned char*)(__cbas_local_variable_mangled_name_b)) == ((unsigned char*)(0ull))){

    
    return (struct __CBAS__type__pstr*)(0ull);
    }

    __CBAS__mcpy(((unsigned char*)(__cbas_local_variable_mangled_name_b)),(((unsigned char*)((((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_this))->__CBAS__member__d))) + ((long long)(__cbas_local_variable_mangled_name_beginning))),((unsigned long long)(__cbas_local_variable_mangled_name_howmuch)));
    (__cbas_local_variable_mangled_name_p) = ((struct __CBAS__type__pstr*)(__CBAS__malloc((sizeof(struct __CBAS__type__str)))));
    if(((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_p)) == ((struct __CBAS__type__pstr*)(0ull))){

    __CBAS__free(((unsigned char*)(__cbas_local_variable_mangled_name_b)));
    
    return (struct __CBAS__type__pstr*)(0ull);
    }

    ((((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_p))->__CBAS__member__d)) = ((unsigned char*)(__cbas_local_variable_mangled_name_b));
    ((((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_p))->__CBAS__member__len)) = ((unsigned long long)(__cbas_local_variable_mangled_name_howmuch));
    
    return (struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_p);
    
}
static inline long long __CBAS____method_str_____replace_first_from(struct __CBAS__type__str* __cbas_local_variable_mangled_name_this,struct __CBAS__type__str* __cbas_local_variable_mangled_name_what,struct __CBAS__type__str* __cbas_local_variable_mangled_name_with,unsigned long long __cbas_local_variable_mangled_name_maxwhere)
{
    long long __cbas_local_variable_mangled_name_where;
    unsigned long long __cbas_local_variable_mangled_name_is_shorter;
    unsigned long long __cbas_local_variable_mangled_name_is_equal;
    if((long long)(__CBAS____method_str_____empty(((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_what))))){

    
    return -((long long)(1ull));
    }

    if(((unsigned long long)(__cbas_local_variable_mangled_name_maxwhere)) >= ((unsigned long long)((((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_this))->__CBAS__member__len)))){

    
    return -((long long)(1ull));
    }

    if((long long)(__CBAS____method_str_____empty(((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_this))))){

    
    return (long long)(0ull);
    }

    (__cbas_local_variable_mangled_name_where) = (__CBAS____method_str_____findfrom(((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_this)),((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_what)),((unsigned long long)(__cbas_local_variable_mangled_name_maxwhere))));
    if(((long long)(__cbas_local_variable_mangled_name_where)) == (-((long long)(1ull)))){

    
    return -((long long)(2ull));
    }

    (__cbas_local_variable_mangled_name_is_shorter) = ((unsigned long long)(((unsigned long long)((((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_with))->__CBAS__member__len))) < ((unsigned long long)((((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_what))->__CBAS__member__len)))));
    (__cbas_local_variable_mangled_name_is_equal) = ((unsigned long long)(((unsigned long long)((((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_with))->__CBAS__member__len))) == ((unsigned long long)((((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_what))->__CBAS__member__len)))));
    if((long long)(__cbas_local_variable_mangled_name_is_equal)){

    __CBAS__mcpy((((unsigned char*)((((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_this))->__CBAS__member__d))) + ((long long)(__cbas_local_variable_mangled_name_where))),((unsigned char*)((((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_with))->__CBAS__member__d))),((unsigned long long)((((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_with))->__CBAS__member__len))));
    
    return (long long)(__cbas_local_variable_mangled_name_where);
    }

    else if((long long)(__cbas_local_variable_mangled_name_is_shorter)){

    __CBAS__mcpy((((unsigned char*)((((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_this))->__CBAS__member__d))) + ((long long)(__cbas_local_variable_mangled_name_where))),((unsigned char*)((((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_with))->__CBAS__member__d))),((unsigned long long)((((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_with))->__CBAS__member__len))));
    __CBAS__mmove(((((unsigned char*)((((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_this))->__CBAS__member__d))) + ((long long)(__cbas_local_variable_mangled_name_where))) + ((long long)((((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_with))->__CBAS__member__len)))),((((unsigned char*)((((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_this))->__CBAS__member__d))) + ((long long)(__cbas_local_variable_mangled_name_where))) + ((long long)((((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_what))->__CBAS__member__len)))),((unsigned long long)((((long long)((((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_this))->__CBAS__member__len))) - (((long long)(__cbas_local_variable_mangled_name_where)) + ((long long)((((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_what))->__CBAS__member__len))))) + ((long long)(1ull)))));
    ((((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_this))->__CBAS__member__len)) = ((unsigned long long)((((long long)(__cbas_local_variable_mangled_name_where)) + ((long long)((((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_with))->__CBAS__member__len)))) + (((long long)((((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_this))->__CBAS__member__len))) - (((long long)(__cbas_local_variable_mangled_name_where)) + ((long long)((((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_what))->__CBAS__member__len)))))));
    
    return (long long)(__cbas_local_variable_mangled_name_where);
    }

    else {
    unsigned char* __cbas_local_variable_mangled_name_resbuf;
    (__cbas_local_variable_mangled_name_resbuf) = (__CBAS__realloc(((unsigned char*)(0ull)),((((unsigned long long)((((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_this))->__CBAS__member__len))) + (((unsigned long long)((((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_with))->__CBAS__member__len))) - ((unsigned long long)((((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_what))->__CBAS__member__len))))) + (1ull))));
    if(((unsigned char*)(__cbas_local_variable_mangled_name_resbuf)) == ((unsigned char*)(0ull))){

    
    return -((long long)(1ull));
    }

    __CBAS__mcpy(((unsigned char*)(__cbas_local_variable_mangled_name_resbuf)),((unsigned char*)((((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_this))->__CBAS__member__d))),((unsigned long long)(__cbas_local_variable_mangled_name_where)));
    __CBAS__mcpy((((unsigned char*)(__cbas_local_variable_mangled_name_resbuf)) + ((long long)(__cbas_local_variable_mangled_name_where))),((unsigned char*)((((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_with))->__CBAS__member__d))),((unsigned long long)((((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_with))->__CBAS__member__len))));
    __CBAS__mcpy(((((unsigned char*)(__cbas_local_variable_mangled_name_resbuf)) + ((long long)(__cbas_local_variable_mangled_name_where))) + ((long long)((((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_with))->__CBAS__member__len)))),((((unsigned char*)((((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_this))->__CBAS__member__d))) + ((long long)(__cbas_local_variable_mangled_name_where))) + ((long long)((((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_what))->__CBAS__member__len)))),((unsigned long long)((((long long)((((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_this))->__CBAS__member__len))) - (((long long)(__cbas_local_variable_mangled_name_where)) + ((long long)((((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_what))->__CBAS__member__len))))) + ((long long)(1ull)))));
    __CBAS__free(((unsigned char*)((((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_this))->__CBAS__member__d))));
    ((((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_this))->__CBAS__member__d)) = ((unsigned char*)(__cbas_local_variable_mangled_name_resbuf));
    ((((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_this))->__CBAS__member__len)) = ((unsigned long long)((((long long)(__cbas_local_variable_mangled_name_where)) + ((long long)((((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_with))->__CBAS__member__len)))) + (((long long)((((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_this))->__CBAS__member__len))) - (((long long)(__cbas_local_variable_mangled_name_where)) + ((long long)((((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_what))->__CBAS__member__len)))))));
    
    return (long long)(__cbas_local_variable_mangled_name_where);
    }

    
}
static inline long long __CBAS____method_pstr_____replace_first_from(struct __CBAS__type__pstr* __cbas_local_variable_mangled_name_this,struct __CBAS__type__pstr* __cbas_local_variable_mangled_name_what,struct __CBAS__type__pstr* __cbas_local_variable_mangled_name_with,unsigned long long __cbas_local_variable_mangled_name_maxwhere)
{
    long long __cbas_local_variable_mangled_name_where;
    unsigned long long __cbas_local_variable_mangled_name_is_shorter;
    unsigned long long __cbas_local_variable_mangled_name_is_equal;
    if((long long)(__CBAS____method_pstr_____empty(((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_what))))){

    
    return -((long long)(1ull));
    }

    if(((unsigned long long)(__cbas_local_variable_mangled_name_maxwhere)) >= ((unsigned long long)((((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_this))->__CBAS__member__len)))){

    
    return -((long long)(1ull));
    }

    if((long long)(__CBAS____method_pstr_____empty(((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_this))))){

    
    return (long long)(0ull);
    }

    (__cbas_local_variable_mangled_name_where) = (__CBAS____method_pstr_____findfrom(((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_this)),((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_what)),((unsigned long long)(__cbas_local_variable_mangled_name_maxwhere))));
    if(((long long)(__cbas_local_variable_mangled_name_where)) == (-((long long)(1ull)))){

    
    return -((long long)(2ull));
    }

    (__cbas_local_variable_mangled_name_is_shorter) = ((unsigned long long)(((unsigned long long)((((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_with))->__CBAS__member__len))) < ((unsigned long long)((((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_what))->__CBAS__member__len)))));
    (__cbas_local_variable_mangled_name_is_equal) = ((unsigned long long)(((unsigned long long)((((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_with))->__CBAS__member__len))) == ((unsigned long long)((((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_what))->__CBAS__member__len)))));
    if((long long)(__cbas_local_variable_mangled_name_is_equal)){

    __CBAS__mcpy((((unsigned char*)((((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_this))->__CBAS__member__d))) + ((long long)(__cbas_local_variable_mangled_name_where))),((unsigned char*)((((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_with))->__CBAS__member__d))),((unsigned long long)((((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_with))->__CBAS__member__len))));
    
    return (long long)(__cbas_local_variable_mangled_name_where);
    }

    else if((long long)(__cbas_local_variable_mangled_name_is_shorter)){

    __CBAS__mcpy((((unsigned char*)((((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_this))->__CBAS__member__d))) + ((long long)(__cbas_local_variable_mangled_name_where))),((unsigned char*)((((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_with))->__CBAS__member__d))),((unsigned long long)((((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_with))->__CBAS__member__len))));
    __CBAS__mmove(((((unsigned char*)((((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_this))->__CBAS__member__d))) + ((long long)(__cbas_local_variable_mangled_name_where))) + ((long long)((((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_with))->__CBAS__member__len)))),((((unsigned char*)((((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_this))->__CBAS__member__d))) + ((long long)(__cbas_local_variable_mangled_name_where))) + ((long long)((((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_what))->__CBAS__member__len)))),((unsigned long long)(((long long)((((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_this))->__CBAS__member__len))) - (((long long)(__cbas_local_variable_mangled_name_where)) + ((long long)((((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_what))->__CBAS__member__len)))))));
    ((((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_this))->__CBAS__member__len)) = ((unsigned long long)((((long long)(__cbas_local_variable_mangled_name_where)) + ((long long)((((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_with))->__CBAS__member__len)))) + (((long long)((((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_this))->__CBAS__member__len))) - (((long long)(__cbas_local_variable_mangled_name_where)) + ((long long)((((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_what))->__CBAS__member__len)))))));
    
    return (long long)(__cbas_local_variable_mangled_name_where);
    }

    else {
    unsigned char* __cbas_local_variable_mangled_name_resbuf;
    (__cbas_local_variable_mangled_name_resbuf) = (__CBAS__realloc(((unsigned char*)(0ull)),(((unsigned long long)((((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_this))->__CBAS__member__len))) + (((unsigned long long)((((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_with))->__CBAS__member__len))) - ((unsigned long long)((((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_what))->__CBAS__member__len)))))));
    if(((unsigned char*)(__cbas_local_variable_mangled_name_resbuf)) == ((unsigned char*)(0ull))){

    
    return -((long long)(1ull));
    }

    __CBAS__mcpy(((unsigned char*)(__cbas_local_variable_mangled_name_resbuf)),((unsigned char*)((((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_this))->__CBAS__member__d))),((unsigned long long)(__cbas_local_variable_mangled_name_where)));
    __CBAS__mcpy((((unsigned char*)(__cbas_local_variable_mangled_name_resbuf)) + ((long long)(__cbas_local_variable_mangled_name_where))),((unsigned char*)((((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_with))->__CBAS__member__d))),((unsigned long long)((((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_with))->__CBAS__member__len))));
    __CBAS__mcpy(((((unsigned char*)(__cbas_local_variable_mangled_name_resbuf)) + ((long long)(__cbas_local_variable_mangled_name_where))) + ((long long)((((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_with))->__CBAS__member__len)))),((((unsigned char*)((((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_this))->__CBAS__member__d))) + ((long long)(__cbas_local_variable_mangled_name_where))) + ((long long)((((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_what))->__CBAS__member__len)))),((unsigned long long)(((long long)((((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_this))->__CBAS__member__len))) - (((long long)(__cbas_local_variable_mangled_name_where)) + ((long long)((((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_what))->__CBAS__member__len)))))));
    __CBAS__free(((unsigned char*)((((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_this))->__CBAS__member__d))));
    ((((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_this))->__CBAS__member__d)) = ((unsigned char*)(__cbas_local_variable_mangled_name_resbuf));
    ((((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_this))->__CBAS__member__len)) = ((unsigned long long)((((long long)(__cbas_local_variable_mangled_name_where)) + ((long long)((((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_with))->__CBAS__member__len)))) + (((long long)((((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_this))->__CBAS__member__len))) - (((long long)(__cbas_local_variable_mangled_name_where)) + ((long long)((((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_what))->__CBAS__member__len)))))));
    
    return (long long)(__cbas_local_variable_mangled_name_where);
    }

    
}
static inline long long __CBAS____method_str_____replace_first(struct __CBAS__type__str* __cbas_local_variable_mangled_name_this,struct __CBAS__type__str* __cbas_local_variable_mangled_name_what,struct __CBAS__type__str* __cbas_local_variable_mangled_name_with)
{
    
    return __CBAS____method_str_____replace_first_from(((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_this)),((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_what)),((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_with)),(0ull));
    
}
static inline long long __CBAS____method_pstr_____replace_first(struct __CBAS__type__pstr* __cbas_local_variable_mangled_name_this,struct __CBAS__type__pstr* __cbas_local_variable_mangled_name_what,struct __CBAS__type__pstr* __cbas_local_variable_mangled_name_with)
{
    
    return __CBAS____method_pstr_____replace_first_from(((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_this)),((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_what)),((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_with)),(0ull));
    
}
static inline long long __CBAS____method_str_____replace(struct __CBAS__type__str* __cbas_local_variable_mangled_name_this,struct __CBAS__type__str* __cbas_local_variable_mangled_name_what,struct __CBAS__type__str* __cbas_local_variable_mangled_name_with)
{
    long long __cbas_local_variable_mangled_name_w;
    if((long long)(__CBAS____method_str_____empty(((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_what))))){

    
    return -((long long)(1ull));
    }

    if((long long)(__CBAS____method_str_____empty(((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_this))))){

    
    return (long long)(0ull);
    }

    (__cbas_local_variable_mangled_name_w) = ((long long)(0ull));
    while((long long)(1ull)){

    long long __cbas_local_variable_mangled_name_w_new;
    (__cbas_local_variable_mangled_name_w_new) = (__CBAS____method_str_____replace_first_from(((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_this)),((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_what)),((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_with)),((unsigned long long)(__cbas_local_variable_mangled_name_w))));
    if(((long long)(__cbas_local_variable_mangled_name_w_new)) == (-((long long)(1ull)))){

    
    return (long long)(0ull);
    }

    if(((long long)(__cbas_local_variable_mangled_name_w_new)) == (-((long long)(2ull)))){

    
    return (long long)(0ull);
    }

    if(((long long)(__cbas_local_variable_mangled_name_w_new)) == ((long long)(__cbas_local_variable_mangled_name_w))){

    
    return (long long)(0ull);
    }

    (__cbas_local_variable_mangled_name_w) = (((long long)(__cbas_local_variable_mangled_name_w_new)) + ((long long)((((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_with))->__CBAS__member__len))));
    }

    
}
static inline long long __CBAS____method_pstr_____replace(struct __CBAS__type__pstr* __cbas_local_variable_mangled_name_this,struct __CBAS__type__pstr* __cbas_local_variable_mangled_name_what,struct __CBAS__type__pstr* __cbas_local_variable_mangled_name_with)
{
    long long __cbas_local_variable_mangled_name_w;
    if((long long)(__CBAS____method_pstr_____empty(((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_what))))){

    
    return -((long long)(1ull));
    }

    if((long long)(__CBAS____method_pstr_____empty(((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_this))))){

    
    return (long long)(0ull);
    }

    (__cbas_local_variable_mangled_name_w) = ((long long)(0ull));
    while((long long)(1ull)){

    long long __cbas_local_variable_mangled_name_w_new;
    (__cbas_local_variable_mangled_name_w_new) = (__CBAS____method_pstr_____replace_first_from(((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_this)),((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_what)),((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_with)),((unsigned long long)(__cbas_local_variable_mangled_name_w))));
    if(((long long)(__cbas_local_variable_mangled_name_w_new)) == (-((long long)(1ull)))){

    
    return (long long)(0ull);
    }

    if(((long long)(__cbas_local_variable_mangled_name_w_new)) == (-((long long)(2ull)))){

    
    return (long long)(0ull);
    }

    if(((long long)(__cbas_local_variable_mangled_name_w_new)) == ((long long)(__cbas_local_variable_mangled_name_w))){

    
    return (long long)(0ull);
    }

    (__cbas_local_variable_mangled_name_w) = (((long long)(__cbas_local_variable_mangled_name_w_new)) + ((long long)((((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_with))->__CBAS__member__len))));
    }

    
}
static inline void __CBAS__utoa(unsigned char* __cbas_local_variable_mangled_name_dest,unsigned long long __cbas_local_variable_mangled_name_value)
{
    if(((unsigned long long)(__cbas_local_variable_mangled_name_value)) == (0ull)){

    (((unsigned char*)(__cbas_local_variable_mangled_name_dest))[(long long)(0ull)]) = ((unsigned char)(48ull));
    (((unsigned char*)(__cbas_local_variable_mangled_name_dest))[(long long)(1ull)]) = ((unsigned char)(0ull));
    
    return;
    }

    if((long long)(1ull)){

    unsigned long long __cbas_local_variable_mangled_name_pow;
    (__cbas_local_variable_mangled_name_pow) = (1ull);
    while((((unsigned long long)(__cbas_local_variable_mangled_name_value)) / ((unsigned long long)(__cbas_local_variable_mangled_name_pow))) >= (10ull)){

    (__cbas_local_variable_mangled_name_pow) = (((unsigned long long)(__cbas_local_variable_mangled_name_pow)) * (10ull));
    }

    while((long long)(__cbas_local_variable_mangled_name_pow)){

    unsigned int __cbas_local_variable_mangled_name_temp;
    (__cbas_local_variable_mangled_name_temp) = ((unsigned int)(((unsigned long long)(__cbas_local_variable_mangled_name_value)) / ((unsigned long long)(__cbas_local_variable_mangled_name_pow))));
    (((unsigned char*)(__cbas_local_variable_mangled_name_dest))[(long long)(0ull)]) = ((unsigned char)(((unsigned long long)(__cbas_local_variable_mangled_name_temp)) + (48ull)));
    (__cbas_local_variable_mangled_name_dest)++;
    (__cbas_local_variable_mangled_name_value) = (((unsigned long long)(__cbas_local_variable_mangled_name_value)) - (((unsigned long long)(__cbas_local_variable_mangled_name_temp)) * ((unsigned long long)(__cbas_local_variable_mangled_name_pow))));
    (__cbas_local_variable_mangled_name_pow) = (((unsigned long long)(__cbas_local_variable_mangled_name_pow)) / (10ull));
    }

    }

    __cbas_mangled_label__ending:1;/*Statement with no effect generated for label...*/
    (((unsigned char*)(__cbas_local_variable_mangled_name_dest))[(long long)(0ull)]) = ((unsigned char)(0ull));
    
    return;
    
}
static inline void __CBAS__itoa(unsigned char* __cbas_local_variable_mangled_name_dest,long long __cbas_local_variable_mangled_name_value)
{
    if(((long long)(__cbas_local_variable_mangled_name_value)) >= ((long long)(0ull))){

    __CBAS__utoa(((unsigned char*)(__cbas_local_variable_mangled_name_dest)),((unsigned long long)(__cbas_local_variable_mangled_name_value)));
    }

    else {
    (((unsigned char*)(__cbas_local_variable_mangled_name_dest))[(long long)(0ull)]) = ((unsigned char)(45ull));
    __CBAS__utoa((((unsigned char*)(__cbas_local_variable_mangled_name_dest)) + ((long long)(1ull))),((unsigned long long)(-((long long)(__cbas_local_variable_mangled_name_value)))));
    }

    
}
static inline void __CBAS__ftoa_n(unsigned char* __cbas_local_variable_mangled_name_dest,double __cbas_local_variable_mangled_name_v,int __cbas_local_variable_mangled_name_ndigits)
{
    unsigned long long __cbas_local_variable_mangled_name_is_neg;
    unsigned long long __cbas_local_variable_mangled_name_dg;
    long long __cbas_local_variable_mangled_name_pow10;
    unsigned long long __cbas_local_variable_mangled_name_dgi;
    double __cbas_local_variable_mangled_name_ftoa_errmargin;
    (__cbas_local_variable_mangled_name_is_neg) = (0ull);
    (__cbas_local_variable_mangled_name_dg) = (0ull);
    (__cbas_local_variable_mangled_name_pow10) = ((long long)(0ull));
    (__cbas_local_variable_mangled_name_dgi) = (0ull);
    (__cbas_local_variable_mangled_name_ndigits)++;
    if(((long long)(__cbas_local_variable_mangled_name_ndigits)) <= ((long long)(0ull))){

    (((unsigned char*)(__cbas_local_variable_mangled_name_dest))[(long long)(0ull)]) = ((unsigned char)(69ull));
    (((unsigned char*)(__cbas_local_variable_mangled_name_dest))[(long long)(1ull)]) = ((unsigned char)(82ull));
    (((unsigned char*)(__cbas_local_variable_mangled_name_dest))[(long long)(2ull)]) = ((unsigned char)(82ull));
    (((unsigned char*)(__cbas_local_variable_mangled_name_dest))[(long long)(3ull)]) = ((unsigned char)(0ull));
    
    return;
    }

    if((long long)(__CBAS__signbit(((double)(__cbas_local_variable_mangled_name_v))))){

    (__cbas_local_variable_mangled_name_is_neg) = (1ull);
    (((unsigned char*)(__cbas_local_variable_mangled_name_dest))[(long long)(0ull)]) = ((unsigned char)(45ull));
    (__cbas_local_variable_mangled_name_dest)++;
    (__cbas_local_variable_mangled_name_v) = (-((double)(__cbas_local_variable_mangled_name_v)));
    }

    if((long long)(__CBAS__isnan(((double)(__cbas_local_variable_mangled_name_v))))){

    (((unsigned char*)(__cbas_local_variable_mangled_name_dest))[(long long)(0ull)]) = ((unsigned char)(78ull));
    (((unsigned char*)(__cbas_local_variable_mangled_name_dest))[(long long)(1ull)]) = ((unsigned char)(65ull));
    (((unsigned char*)(__cbas_local_variable_mangled_name_dest))[(long long)(2ull)]) = ((unsigned char)(78ull));
    (((unsigned char*)(__cbas_local_variable_mangled_name_dest))[(long long)(3ull)]) = ((unsigned char)(0ull));
    
    return;
    }

    if((long long)(!((long long)(__CBAS__isfinite(((double)(__cbas_local_variable_mangled_name_v))))))){

    (((unsigned char*)(__cbas_local_variable_mangled_name_dest))[(long long)(0ull)]) = ((unsigned char)(73ull));
    (((unsigned char*)(__cbas_local_variable_mangled_name_dest))[(long long)(1ull)]) = ((unsigned char)(78ull));
    (((unsigned char*)(__cbas_local_variable_mangled_name_dest))[(long long)(2ull)]) = ((unsigned char)(70ull));
    (((unsigned char*)(__cbas_local_variable_mangled_name_dest))[(long long)(3ull)]) = ((unsigned char)(0ull));
    
    return;
    }

    if(((double)(__cbas_local_variable_mangled_name_v)) == ((double)(0ull))){

    (((unsigned char*)(__cbas_local_variable_mangled_name_dest))[(long long)(0ull)]) = ((unsigned char)(48ull));
    (((unsigned char*)(__cbas_local_variable_mangled_name_dest))[(long long)(1ull)]) = ((unsigned char)(0ull));
    
    return;
    }

    (__cbas_local_variable_mangled_name_pow10) = ((long long)(__CBAS__log10(((double)(__cbas_local_variable_mangled_name_v)))));
    if(((long long)(__cbas_local_variable_mangled_name_pow10)) >= (((long long)(__cbas_local_variable_mangled_name_ndigits)) + ((long long)(1ull)))){

    goto __cbas_mangled_label__science;
    }

    else if(((long long)(__cbas_local_variable_mangled_name_pow10)) <= (-(((long long)(__cbas_local_variable_mangled_name_ndigits)) + ((long long)(1ull))))){

    goto __cbas_mangled_label__science;
    }

    else {
    while(((long long)(__cbas_local_variable_mangled_name_pow10)) >= ((long long)(0ull))){

    long long __cbas_local_variable_mangled_name_temp;
    (__cbas_local_variable_mangled_name_temp) = ((long long)(((double)(__cbas_local_variable_mangled_name_v)) / (__CBAS__pow(((double)(10ull)),((double)(__cbas_local_variable_mangled_name_pow10))))));
    (((unsigned char*)(__cbas_local_variable_mangled_name_dest))[(long long)(0ull)]) = ((unsigned char)(((long long)(__cbas_local_variable_mangled_name_temp)) + ((long long)(48ull))));
    (__cbas_local_variable_mangled_name_dest)++;
    (__cbas_local_variable_mangled_name_ndigits)--;
    (__cbas_local_variable_mangled_name_v) = (((double)(__cbas_local_variable_mangled_name_v)) - (((double)(__cbas_local_variable_mangled_name_temp)) * (__CBAS__pow(((double)(10ull)),((double)(__cbas_local_variable_mangled_name_pow10))))));
    (__cbas_local_variable_mangled_name_pow10)--;
    }

    if((long long)(((unsigned long long)(((double)(__cbas_local_variable_mangled_name_v)) == ((double)(0ull)))) || ((unsigned long long)(((long long)(__cbas_local_variable_mangled_name_ndigits)) == ((long long)(0ull)))))){

    (((unsigned char*)(__cbas_local_variable_mangled_name_dest))[(long long)(0ull)]) = ((unsigned char)(46ull));
    (__cbas_local_variable_mangled_name_dest)++;
    (((unsigned char*)(__cbas_local_variable_mangled_name_dest))[(long long)(0ull)]) = ((unsigned char)(48ull));
    (__cbas_local_variable_mangled_name_dest)++;
    (((unsigned char*)(__cbas_local_variable_mangled_name_dest))[(long long)(0ull)]) = ((unsigned char)(0ull));
    
    return;
    }

    (((unsigned char*)(__cbas_local_variable_mangled_name_dest))[(long long)(0ull)]) = ((unsigned char)(46ull));
    (__cbas_local_variable_mangled_name_dest)++;
    while(((long long)(__cbas_local_variable_mangled_name_ndigits)) > ((long long)(0ull))){

    long long __cbas_local_variable_mangled_name_temp;
    (__cbas_local_variable_mangled_name_v) = (((double)(__cbas_local_variable_mangled_name_v)) * ((double)(10ull)));
    (__cbas_local_variable_mangled_name_temp) = ((long long)(__cbas_local_variable_mangled_name_v));
    (((unsigned char*)(__cbas_local_variable_mangled_name_dest))[(long long)(0ull)]) = ((unsigned char)(((long long)(__cbas_local_variable_mangled_name_temp)) + ((long long)(48ull))));
    (__cbas_local_variable_mangled_name_dest)++;
    (__cbas_local_variable_mangled_name_v) = (((double)(__cbas_local_variable_mangled_name_v)) - ((double)(__cbas_local_variable_mangled_name_temp)));
    (__cbas_local_variable_mangled_name_ndigits)--;
    }

    (((unsigned char*)(__cbas_local_variable_mangled_name_dest))[(long long)(0ull)]) = ((unsigned char)(0ull));
    
    return;
    }

    
    return;
    __cbas_mangled_label__science:1;/*Statement with no effect generated for label...*/
    if((long long)(1ull)){

    long long __cbas_local_variable_mangled_name_dgc;
    (__cbas_local_variable_mangled_name_dgc) = ((long long)(0ull));
    (__cbas_local_variable_mangled_name_v) = (((double)(__cbas_local_variable_mangled_name_v)) / (__CBAS__pow(((double)(10ull)),((double)(__cbas_local_variable_mangled_name_pow10)))));
    while(((long long)(__cbas_local_variable_mangled_name_ndigits)) > ((long long)(0ull))){

    long long __cbas_local_variable_mangled_name_temp;
    (__cbas_local_variable_mangled_name_temp) = ((long long)(__cbas_local_variable_mangled_name_v));
    (((unsigned char*)(__cbas_local_variable_mangled_name_dest))[(long long)(0ull)]) = ((unsigned char)(((long long)(__cbas_local_variable_mangled_name_temp)) + ((long long)(48ull))));
    (__cbas_local_variable_mangled_name_dest)++;
    (__cbas_local_variable_mangled_name_v) = (((double)(__cbas_local_variable_mangled_name_v)) - ((double)(__cbas_local_variable_mangled_name_temp)));
    (__cbas_local_variable_mangled_name_ndigits)--;
    if((long long)(((unsigned long long)(((long long)(__cbas_local_variable_mangled_name_dgc)) == ((long long)(0ull)))) && ((unsigned long long)(((long long)(__cbas_local_variable_mangled_name_ndigits)) != ((long long)(0ull)))))){

    (((unsigned char*)(__cbas_local_variable_mangled_name_dest))[(long long)(0ull)]) = ((unsigned char)(46ull));
    (__cbas_local_variable_mangled_name_dest)++;
    }

    (__cbas_local_variable_mangled_name_dgc)++;
    (__cbas_local_variable_mangled_name_v) = (((double)(__cbas_local_variable_mangled_name_v)) * ((double)(10ull)));
    }

    (((unsigned char*)(__cbas_local_variable_mangled_name_dest))[(long long)(0ull)]) = ((unsigned char)(69ull));
    (__cbas_local_variable_mangled_name_dest)++;
    __CBAS__itoa(((unsigned char*)(__cbas_local_variable_mangled_name_dest)),((long long)(__cbas_local_variable_mangled_name_pow10)));
    
    return;
    }

    
}
static inline void __CBAS__ftoa(unsigned char* __cbas_local_variable_mangled_name_dest,double __cbas_local_variable_mangled_name_v)
{
    __CBAS__ftoa_n(((unsigned char*)(__cbas_local_variable_mangled_name_dest)),((double)(__cbas_local_variable_mangled_name_v)),((int)(54ull)));
    
}
static void __CBAS__mutoa(unsigned char* __cbas_local_variable_mangled_name_dest,unsigned int __cbas_local_variable_mangled_name_value)
{
    if(((unsigned long long)(__cbas_local_variable_mangled_name_value)) == (0ull)){

    (((unsigned char*)(__cbas_local_variable_mangled_name_dest))[(long long)(0ull)]) = ((unsigned char)(48ull));
    (((unsigned char*)(__cbas_local_variable_mangled_name_dest))[(long long)(1ull)]) = ((unsigned char)(0ull));
    
    return;
    }

    if((long long)(1ull)){

    unsigned int __cbas_local_variable_mangled_name_pow;
    (__cbas_local_variable_mangled_name_pow) = ((unsigned int)(1ull));
    while(((unsigned long long)(((unsigned int)(__cbas_local_variable_mangled_name_value)) / ((unsigned int)(__cbas_local_variable_mangled_name_pow)))) >= (10ull)){

    (__cbas_local_variable_mangled_name_pow) = ((unsigned int)(((unsigned long long)(__cbas_local_variable_mangled_name_pow)) * (10ull)));
    }

    while((long long)(__cbas_local_variable_mangled_name_pow)){

    unsigned int __cbas_local_variable_mangled_name_temp;
    (__cbas_local_variable_mangled_name_temp) = (((unsigned int)(__cbas_local_variable_mangled_name_value)) / ((unsigned int)(__cbas_local_variable_mangled_name_pow)));
    (((unsigned char*)(__cbas_local_variable_mangled_name_dest))[(long long)(0ull)]) = ((unsigned char)(((unsigned long long)(__cbas_local_variable_mangled_name_temp)) + (48ull)));
    (__cbas_local_variable_mangled_name_dest)++;
    (__cbas_local_variable_mangled_name_value) = (((unsigned int)(__cbas_local_variable_mangled_name_value)) - (((unsigned int)(__cbas_local_variable_mangled_name_temp)) * ((unsigned int)(__cbas_local_variable_mangled_name_pow))));
    (__cbas_local_variable_mangled_name_pow) = ((unsigned int)(((unsigned long long)(__cbas_local_variable_mangled_name_pow)) / (10ull)));
    }

    }

    __cbas_mangled_label__ending:1;/*Statement with no effect generated for label...*/
    (((unsigned char*)(__cbas_local_variable_mangled_name_dest))[(long long)(0ull)]) = ((unsigned char)(0ull));
    
    return;
    
}
static unsigned int __CBAS__matou(unsigned char* __cbas_local_variable_mangled_name_in)
{
    unsigned int __cbas_local_variable_mangled_name_retval;
    (__cbas_local_variable_mangled_name_retval) = ((unsigned int)(0ull));
    while((long long)(((unsigned long long)(((unsigned long long)(((unsigned char*)(__cbas_local_variable_mangled_name_in))[(long long)(0ull)])) >= (48ull))) && ((unsigned long long)(((unsigned long long)(((unsigned char*)(__cbas_local_variable_mangled_name_in))[(long long)(0ull)])) <= (57ull))))){

    (__cbas_local_variable_mangled_name_retval) = ((unsigned int)(((unsigned long long)(__cbas_local_variable_mangled_name_retval)) * (10ull)));
    (__cbas_local_variable_mangled_name_retval) = ((unsigned int)(((unsigned long long)(__cbas_local_variable_mangled_name_retval)) + (((unsigned long long)(((unsigned char*)(__cbas_local_variable_mangled_name_in))[(long long)(0ull)])) - (48ull))));
    (__cbas_local_variable_mangled_name_in)++;
    }

    
    return (unsigned int)(__cbas_local_variable_mangled_name_retval);
    
}
static inline unsigned int __CBAS__fib(unsigned int __cbas_local_variable_mangled_name_n)
{
    unsigned int __cbas_local_variable_mangled_name_a;
    unsigned int __cbas_local_variable_mangled_name_b;
    unsigned int __cbas_local_variable_mangled_name_c;
    if(((unsigned long long)(__cbas_local_variable_mangled_name_n)) < (2ull)){

    
    return (unsigned int)(1ull);
    }

    (__cbas_local_variable_mangled_name_a) = ((unsigned int)(1ull));
    (__cbas_local_variable_mangled_name_b) = ((unsigned int)(1ull));
    (__cbas_local_variable_mangled_name_c) = ((unsigned int)(1ull));
    (__cbas_local_variable_mangled_name_n) = ((unsigned int)(((unsigned long long)(__cbas_local_variable_mangled_name_n)) - (2ull)));
    while((long long)(__cbas_local_variable_mangled_name_n)){

    (__cbas_local_variable_mangled_name_c) = (((unsigned int)(__cbas_local_variable_mangled_name_a)) + ((unsigned int)(__cbas_local_variable_mangled_name_b)));
    (__cbas_local_variable_mangled_name_a) = ((unsigned int)(__cbas_local_variable_mangled_name_b));
    (__cbas_local_variable_mangled_name_b) = ((unsigned int)(__cbas_local_variable_mangled_name_c));
    (__cbas_local_variable_mangled_name_n)--;
    }

    
    return (unsigned int)(__cbas_local_variable_mangled_name_c);
    
}
int main(int __cbas_local_variable_mangled_name_argc,char** __cbas_local_variable_mangled_name_argv)
{
    unsigned int __cbas_local_variable_mangled_name_qq;
    if(((long long)(__cbas_local_variable_mangled_name_argc)) < ((long long)(2ull))){

    if((long long)(1ull)){

    unsigned char __cbas_local_variable_mangled_name___pprint_local_buf[250ll];
    __CBAS__println((((unsigned char*)"Usage: fib 13")));
    }

    __CBAS__sys_exit(((int)(1ull)));
    }

    __CBAS__println((((unsigned char*)"Welcome to the fibonacci number calculator!")));
    (__cbas_local_variable_mangled_name_qq) = (__CBAS__matou(((unsigned char*)(((char**)(__cbas_local_variable_mangled_name_argv))[(long long)(1ull)]))));
    if((long long)(1ull)){

    unsigned char __cbas_local_variable_mangled_name___pprint_local_buf[250ll];
    __CBAS__println((((unsigned char*)"You asked for the fibonacci number...")));
    __CBAS__mutoa((__cbas_local_variable_mangled_name___pprint_local_buf),((unsigned int)(__cbas_local_variable_mangled_name_qq)));
    __CBAS__println((__cbas_local_variable_mangled_name___pprint_local_buf));
    __CBAS__println((((unsigned char*)"That fibonacci number is:")));
    __CBAS__mutoa((__cbas_local_variable_mangled_name___pprint_local_buf),(__CBAS__fib(((unsigned int)(__cbas_local_variable_mangled_name_qq)))));
    __CBAS__println((__cbas_local_variable_mangled_name___pprint_local_buf));
    }

    
    return (int)(0ull);
    
}
/*
Blessed be the Lord Jesus Christ of Nazareth, forever. Amen.
*/
