
/*
* ---- EXPORTED BY TOC.HBAS ----
* Beloved, if God so loved us, we ought also to love one another.
* 1 Jn 4:11
*/

#define SEABASS_32_BIT 1
#define i32 long
#define u32 unsigned long
#define _STRTOU_ strtoul
#define _STRTOI_ strtol
#define TGT_UMAX unsigned long
#define TGT_IMAX long


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <errno.h>


#ifndef __APPLE__
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

//function declarations

static inline void init_lsthread(lsthread* t){
	//t->myMutex = (pthread_mutex_t)PTHREAD_MUTEX_INITIALIZER;
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
	if(t->state == 1)return;//if already locked, nono
	if(!t->isThreadLive)return;
	pthread_barrier_wait(&t->myBarrier);
	if(pthread_mutex_lock(&t->myMutex))
		exit(1);
	t->state = 1;
}

static inline void step(lsthread* t){
	if(t->state == -1)return; //if already stepping, nono
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
	t->state = 1; //LOCKED
	pthread_create(
		&t->myThread,
		NULL,
		lsthread_func,
		(void*)t
	);
}

#endif

static inline double __CBAS__atof(unsigned char* text){
    return atof((char*)text);
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
//MATH LIBRARY
//trig
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

//arcXX functions
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
//atan2
    static inline double __CBAS__atan2(double a, double b){
        return atan2(a, b);
    }
    static inline float __CBAS__atan2f(float a, float b){
        return atan2f(a, b);
    }
//hyperbolics
    static inline double __CBAS__sinh(double a){
        return sinh(a);
    }
    static inline double __CBAS__cosh(double a){
        return cosh(a);
    }
    static inline double __CBAS__tanh(double a){
        return tanh(a);
    }
//float versions
    static inline float __CBAS__sinhf(float a){
        return sinhf(a);
    }
    static inline float __CBAS__coshf(float a){
        return coshf(a);
    }
    static inline float __CBAS__tanhf(float a){
        return tanhf(a);
    }
//archyperbolics
    static inline double __CBAS__asinh(double a){
        return asinh(a);
    }
    static inline double __CBAS__acosh(double a){
        return acosh(a);
    }
    static inline double __CBAS__atanh(double a){
        return atanh(a);
    }
//float versions
    static inline float __CBAS__asinhf(float a){
        return asinhf(a);
    }
    static inline float __CBAS__acoshf(float a){
        return acoshf(a);
    }
    static inline float __CBAS__atanhf(float a){
        return atanhf(a);
    }
//exp
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
//ldexp 
    static inline double __CBAS__ldexp(double a, i32 b){
        return ldexp(a,b);
    }
    static inline float __CBAS__ldexpf(float a, i32 b){
        return ldexp(a,b);
    }
//logarithms...
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
//modf
    static inline double __CBAS__modf(double a, double* b){
        return modf(a, b);
    }
    static inline float __CBAS__modff(float a, float* b){
        return modff(a, b);
    }
//exp2
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
//ilogb (RETURNS INT!!!!)
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
//pow
    static inline double __CBAS__pow(double a, double b){
        return pow(a, b);
    }
    static inline float __CBAS__powf(float a, float b){
        return powf(a, b);
    }
//sqrt
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
//hypot
    static inline double __CBAS__hypot(double a, double b){
        return hypot(a, b);
    }
    static inline float __CBAS__hypotf(float a, float b){
        return hypotf(a, b);
    }
//erf and erfc
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
//gamma fns
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
//ceil floor
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
//fmod
    static inline double __CBAS__fmod(double a, double b){
        return fmod(a,b);
    }
    static inline float __CBAS__fmodf(float a, float b){
        return fmodf(a,b);
    }
//nearbyint
    static inline double __CBAS__nearbyint(double a){
        return nearbyint(a);
    }
    static inline float __CBAS__nearbyintf(float a){
        return nearbyintf(a);
    }
//remainder
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
//copysign
    static inline double __CBAS__copysign(double a, double b){
        return copysign(a,b);
    }
    static inline float __CBAS__copysignf(float a, float b){
        return copysignf(a,b);
    }
//nan
    static inline double __CBAS__nan(unsigned char* tagp){
        return nan((const char*)tagp);
    }
    static inline float __CBAS__nanf(unsigned char* tagp){
        return nanf((const char*)tagp);
    }
//nextafter
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
//fmaxmin
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
//fabs
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
//fpclassify
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
//fpclassify
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
//isunordered
    static inline i32 __CBAS__isunordered(double a, double b){
        return isunordered(a,b);
    }
    static inline i32 __CBAS__isunorderedf(float a, float b){
        return isunordered(a,b);
    }
//infinity, nan, huge_val
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

//MULTITHREADING LIBRARY
#ifndef __APPLE__


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
    return NULL;
}
static inline void __CBAS__thread_delete(unsigned char* tr){
    free(tr);
    return;
}

static inline void __CBAS__thread_assign_fn(unsigned char* tr, unsigned char* funk){
    return;
}

static inline void __CBAS__thread_assign_arg(unsigned char* tr, unsigned char* arg){
    return;
}

static inline void __CBAS__thread_start(unsigned char* tr){
    return;
}
static inline void __CBAS__thread_kill(unsigned char* tr){
    return;
}

static inline void __CBAS__thread_step(unsigned char* tr){
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
    unsigned char  __CBAS__member___d_[16l];
} ;
struct __CBAS__type__str{
    unsigned char*  __CBAS__member__d;
    unsigned long  __CBAS__member__len;
} ;
struct __CBAS__type__pstr{
    unsigned char*  __CBAS__member__d;
    unsigned long  __CBAS__member__len;
} ;
union __CBAS__type__bbb{ _Alignas(16) 
    float  __CBAS__member__b;
    long  __CBAS__member__a;
    union __CBAS__type__bbb*  __CBAS__member__beebeebee;
} ;
struct __CBAS__type__bbb2{ _Alignas(32) 
    union __CBAS__type__bbb  __CBAS__member__mymember;
    float  __CBAS__member__q;
} ;
struct __CBAS__type__membuf{
    unsigned char*  __CBAS__member__d;
} ;
struct __CBAS__type__this_type_has_a_destructor{
    long  __CBAS__member__a;
    long  __CBAS__member__b;
    unsigned char*  __CBAS__member__qq;
} ;
/*
GLOBAL VARIABLES
*/
static union __CBAS__type__bbb __CBAS__my_bbbs[60l];
static long __CBAS__myintegers[7l] = {
3l, 4l, 5l, 7l, 9l, 10l, 1l
};
static float __CBAS__myfloats[3l] = {
3.0, 3.1415901184082031250000000000000000000000000000000000000000000000, -14.6999998092651367187500000000000000000000000000000000000000000000
};
long __CBAS__myint = 1l;
static long __CBAS__myint2 = 2l;
unsigned char __CBAS__mystring[59l] = {
65ul, 65ul, 65ul, 65ul, 84ul, 104ul, 105ul, 115ul, 
32ul, 105ul, 115ul, 32ul, 97ul, 32ul, 115ul, 116ul, 
114ul, 105ul, 110ul, 103ul, 33ul, 32ul, 73ul, 32ul, 
119ul, 111ul, 110ul, 100ul, 101ul, 114ul, 32ul, 119ul, 
104ul, 97ul, 116ul, 32ul, 116ul, 104ul, 101ul, 32ul, 
108ul, 105ul, 115ul, 116ul, 32ul, 99ul, 104ul, 97ul, 
114ul, 97ul, 99ul, 116ul, 101ul, 114ul, 32ul, 105ul, 
115ul, 63ul, 0ul
};
static _Atomic long __CBAS__my_integer = 37l;
/*
FUNCTION PROTOTYPES
*/
static inline long __CBAS__impl_streq(unsigned char* __cbas_local_variable_mangled_name_a,unsigned char* __cbas_local_variable_mangled_name_b);
static inline long __CBAS__memeq(unsigned char* __cbas_local_variable_mangled_name_a,unsigned char* __cbas_local_variable_mangled_name_b,unsigned long __cbas_local_variable_mangled_name_l);
static inline void __CBAS____method_str_____ctor(struct __CBAS__type__str* __cbas_local_variable_mangled_name_this);
static inline void __CBAS____method_str_____dtor(struct __CBAS__type__str* __cbas_local_variable_mangled_name_this);
static inline void __CBAS____method_pstr_____ctor(struct __CBAS__type__pstr* __cbas_local_variable_mangled_name_this);
static inline void __CBAS____method_pstr_____dtor(struct __CBAS__type__pstr* __cbas_local_variable_mangled_name_this);
static inline void __CBAS____method_str_____free(struct __CBAS__type__str* __cbas_local_variable_mangled_name_this);
static inline void __CBAS____method_pstr_____free(struct __CBAS__type__pstr* __cbas_local_variable_mangled_name_this);
static inline long __CBAS____method_str_____equal(struct __CBAS__type__str* __cbas_local_variable_mangled_name_this,struct __CBAS__type__str* __cbas_local_variable_mangled_name_o);
static inline long __CBAS____method_str_____nequal(struct __CBAS__type__str* __cbas_local_variable_mangled_name_this,struct __CBAS__type__str* __cbas_local_variable_mangled_name_o);
static inline long __CBAS____method_pstr_____equal(struct __CBAS__type__pstr* __cbas_local_variable_mangled_name_this,struct __CBAS__type__pstr* __cbas_local_variable_mangled_name_o);
static inline long __CBAS____method_pstr_____nequal(struct __CBAS__type__pstr* __cbas_local_variable_mangled_name_this,struct __CBAS__type__pstr* __cbas_local_variable_mangled_name_o);
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
static inline unsigned long __CBAS____method_str_____isnull(struct __CBAS__type__str* __cbas_local_variable_mangled_name_this);
static inline unsigned long __CBAS____method_pstr_____isnull(struct __CBAS__type__pstr* __cbas_local_variable_mangled_name_this);
static inline unsigned long __CBAS____method_str_____isempty(struct __CBAS__type__str* __cbas_local_variable_mangled_name_this);
static inline unsigned long __CBAS____method_str_____empty(struct __CBAS__type__str* __cbas_local_variable_mangled_name_this);
static inline unsigned long __CBAS____method_pstr_____isempty(struct __CBAS__type__pstr* __cbas_local_variable_mangled_name_this);
static inline unsigned long __CBAS____method_pstr_____empty(struct __CBAS__type__pstr* __cbas_local_variable_mangled_name_this);
static inline unsigned long __CBAS____method_str_____prefixed(struct __CBAS__type__str* __cbas_local_variable_mangled_name_this,struct __CBAS__type__str* __cbas_local_variable_mangled_name_o);
static inline unsigned long __CBAS____method_pstr_____prefixed(struct __CBAS__type__pstr* __cbas_local_variable_mangled_name_this,struct __CBAS__type__pstr* __cbas_local_variable_mangled_name_o);
static inline unsigned long __CBAS____method_str_____postfixed(struct __CBAS__type__str* __cbas_local_variable_mangled_name_this,struct __CBAS__type__str* __cbas_local_variable_mangled_name_o);
static inline unsigned long __CBAS____method_pstr_____postfixed(struct __CBAS__type__pstr* __cbas_local_variable_mangled_name_this,struct __CBAS__type__pstr* __cbas_local_variable_mangled_name_o);
static inline long __CBAS____method_str_____findfrom(struct __CBAS__type__str* __cbas_local_variable_mangled_name_this,struct __CBAS__type__str* __cbas_local_variable_mangled_name_findme,unsigned long __cbas_local_variable_mangled_name_where);
static inline long __CBAS____method_pstr_____findfrom(struct __CBAS__type__pstr* __cbas_local_variable_mangled_name_this,struct __CBAS__type__pstr* __cbas_local_variable_mangled_name_findme,unsigned long __cbas_local_variable_mangled_name_where);
static inline long __CBAS____method_str_____find(struct __CBAS__type__str* __cbas_local_variable_mangled_name_this,struct __CBAS__type__str* __cbas_local_variable_mangled_name_findme);
static inline long __CBAS____method_pstr_____find(struct __CBAS__type__pstr* __cbas_local_variable_mangled_name_this,struct __CBAS__type__pstr* __cbas_local_variable_mangled_name_findme);
static inline unsigned long __CBAS____method_str_____add(struct __CBAS__type__str* __cbas_local_variable_mangled_name_this,struct __CBAS__type__str* __cbas_local_variable_mangled_name_o);
static inline unsigned long __CBAS____method_pstr_____add(struct __CBAS__type__pstr* __cbas_local_variable_mangled_name_this,struct __CBAS__type__pstr* __cbas_local_variable_mangled_name_o);
static inline unsigned long __CBAS____method_str_____addc(struct __CBAS__type__str* __cbas_local_variable_mangled_name_this,unsigned char* __cbas_local_variable_mangled_name_oo);
static inline unsigned long __CBAS____method_pstr_____addc(struct __CBAS__type__pstr* __cbas_local_variable_mangled_name_this,unsigned char* __cbas_local_variable_mangled_name_oo);
static inline struct __CBAS__type__str* __CBAS____method_str_____substr(struct __CBAS__type__str* __cbas_local_variable_mangled_name_this,unsigned long __cbas_local_variable_mangled_name_beginning,unsigned long __cbas_local_variable_mangled_name_howmuch);
static inline struct __CBAS__type__pstr* __CBAS____method_pstr_____substr(struct __CBAS__type__pstr* __cbas_local_variable_mangled_name_this,unsigned long __cbas_local_variable_mangled_name_beginning,unsigned long __cbas_local_variable_mangled_name_howmuch);
static inline long __CBAS____method_str_____replace_first_from(struct __CBAS__type__str* __cbas_local_variable_mangled_name_this,struct __CBAS__type__str* __cbas_local_variable_mangled_name_what,struct __CBAS__type__str* __cbas_local_variable_mangled_name_with,unsigned long __cbas_local_variable_mangled_name_maxwhere);
static inline long __CBAS____method_pstr_____replace_first_from(struct __CBAS__type__pstr* __cbas_local_variable_mangled_name_this,struct __CBAS__type__pstr* __cbas_local_variable_mangled_name_what,struct __CBAS__type__pstr* __cbas_local_variable_mangled_name_with,unsigned long __cbas_local_variable_mangled_name_maxwhere);
static inline long __CBAS____method_str_____replace_first(struct __CBAS__type__str* __cbas_local_variable_mangled_name_this,struct __CBAS__type__str* __cbas_local_variable_mangled_name_what,struct __CBAS__type__str* __cbas_local_variable_mangled_name_with);
static inline long __CBAS____method_pstr_____replace_first(struct __CBAS__type__pstr* __cbas_local_variable_mangled_name_this,struct __CBAS__type__pstr* __cbas_local_variable_mangled_name_what,struct __CBAS__type__pstr* __cbas_local_variable_mangled_name_with);
static inline long __CBAS____method_str_____replace(struct __CBAS__type__str* __cbas_local_variable_mangled_name_this,struct __CBAS__type__str* __cbas_local_variable_mangled_name_what,struct __CBAS__type__str* __cbas_local_variable_mangled_name_with);
static inline long __CBAS____method_pstr_____replace(struct __CBAS__type__pstr* __cbas_local_variable_mangled_name_this,struct __CBAS__type__pstr* __cbas_local_variable_mangled_name_what,struct __CBAS__type__pstr* __cbas_local_variable_mangled_name_with);
static inline void __CBAS__utoa(unsigned char* __cbas_local_variable_mangled_name_dest,unsigned long __cbas_local_variable_mangled_name_value);
static inline void __CBAS__itoa(unsigned char* __cbas_local_variable_mangled_name_dest,long __cbas_local_variable_mangled_name_value);
static inline void __CBAS__ftoa_n(unsigned char* __cbas_local_variable_mangled_name_dest,double __cbas_local_variable_mangled_name_v,long __cbas_local_variable_mangled_name_ndigits);
static inline void __CBAS__ftoa(unsigned char* __cbas_local_variable_mangled_name_dest,double __cbas_local_variable_mangled_name_v);
static void __CBAS____method_bbb2_____ctor(struct __CBAS__type__bbb2* __cbas_local_variable_mangled_name_this);
static void __CBAS____method_membuf_____ctor(struct __CBAS__type__membuf* __cbas_local_variable_mangled_name_this);
static void __CBAS____method_membuf_____dtor(struct __CBAS__type__membuf* __cbas_local_variable_mangled_name_this);
static void __CBAS____method_membuf_____req(struct __CBAS__type__membuf* __cbas_local_variable_mangled_name_this,unsigned long __cbas_local_variable_mangled_name_sz);
static inline long __CBAS____method_bbb_____do_stuff(union __CBAS__type__bbb* __cbas_local_variable_mangled_name_this,long __cbas_local_variable_mangled_name_incr);
static long __CBAS__myFunction(long __cbas_local_variable_mangled_name_arg1,long __cbas_local_variable_mangled_name_arg2,long __cbas_local_variable_mangled_name_arg3);
static inline long __CBAS____method_bbb_____do_stuff2(union __CBAS__type__bbb* __cbas_local_variable_mangled_name_this,long __cbas_local_variable_mangled_name_incr,long __cbas_local_variable_mangled_name_incr2,long __cbas_local_variable_mangled_name_incr3);
static void __CBAS____method_bbb_____init(union __CBAS__type__bbb* __cbas_local_variable_mangled_name_this);
static inline void __CBAS__mutoa(unsigned char* __cbas_local_variable_mangled_name_dest,unsigned long __cbas_local_variable_mangled_name_value);
static inline void __CBAS____method_this_type_has_a_destructor_____dtor(struct __CBAS__type__this_type_has_a_destructor* __cbas_local_variable_mangled_name_this);
static inline void __CBAS____method_this_type_has_a_destructor_____ctor(struct __CBAS__type__this_type_has_a_destructor* __cbas_local_variable_mangled_name_this);
static void __CBAS__whosa_yousa(long __cbas_local_variable_mangled_name_a);
static void __CBAS__wadda(long __cbas_local_variable_mangled_name_iters);
void __CBAS__inf_loop();
long __CBAS__inf_loop2();
static long __CBAS__inf_loop3(long __cbas_local_variable_mangled_name_a);
long main();
/*
FUNCTION BODIES
*/
static inline long __CBAS__impl_streq(unsigned char* __cbas_local_variable_mangled_name_a,unsigned char* __cbas_local_variable_mangled_name_b)
{
    
    return (__CBAS__strcmp(((unsigned char*)(__cbas_local_variable_mangled_name_a)),((unsigned char*)(__cbas_local_variable_mangled_name_b)))) == ((long)(0ul));
    
}
static inline long __CBAS__memeq(unsigned char* __cbas_local_variable_mangled_name_a,unsigned char* __cbas_local_variable_mangled_name_b,unsigned long __cbas_local_variable_mangled_name_l)
{
    
    return (__CBAS__memcmp(((unsigned char*)(__cbas_local_variable_mangled_name_a)),((unsigned char*)(__cbas_local_variable_mangled_name_b)),((unsigned long)(__cbas_local_variable_mangled_name_l)))) == ((long)(0ul));
    
}
static inline void __CBAS____method_str_____ctor(struct __CBAS__type__str* __cbas_local_variable_mangled_name_this)
{
    ((((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_this))->__CBAS__member__d)) = ((unsigned char*)(0ul));
    ((((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_this))->__CBAS__member__len)) = (0ul);
    
}
static inline void __CBAS____method_str_____dtor(struct __CBAS__type__str* __cbas_local_variable_mangled_name_this)
{
    if(((unsigned char*)((((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_this))->__CBAS__member__d))) != ((unsigned char*)(0ul))){

    __CBAS__free(((unsigned char*)((((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_this))->__CBAS__member__d))));
    }

    __CBAS____method_str_____ctor(((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_this)));
    
}
static inline void __CBAS____method_pstr_____ctor(struct __CBAS__type__pstr* __cbas_local_variable_mangled_name_this)
{
    ((((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_this))->__CBAS__member__d)) = ((unsigned char*)(0ul));
    ((((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_this))->__CBAS__member__len)) = (0ul);
    
}
static inline void __CBAS____method_pstr_____dtor(struct __CBAS__type__pstr* __cbas_local_variable_mangled_name_this)
{
    if(((unsigned char*)((((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_this))->__CBAS__member__d))) != ((unsigned char*)(0ul))){

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
static inline long __CBAS____method_str_____equal(struct __CBAS__type__str* __cbas_local_variable_mangled_name_this,struct __CBAS__type__str* __cbas_local_variable_mangled_name_o)
{
    if(((unsigned long)((((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_this))->__CBAS__member__len))) != ((unsigned long)((((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_o))->__CBAS__member__len)))){

    
    return (long)(0ul);
    }

    if(((unsigned long)((((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_this))->__CBAS__member__len))) == (0ul)){

    
    return (long)(1ul);
    }

    
    return (__CBAS__memcmp(((unsigned char*)((((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_this))->__CBAS__member__d))),((unsigned char*)((((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_o))->__CBAS__member__d))),((unsigned long)((((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_this))->__CBAS__member__len))))) == ((long)(0ul));
    
}
static inline long __CBAS____method_str_____nequal(struct __CBAS__type__str* __cbas_local_variable_mangled_name_this,struct __CBAS__type__str* __cbas_local_variable_mangled_name_o)
{
    
    return (long)(!(__CBAS____method_str_____equal(((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_this)),((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_o)))));
    
}
static inline long __CBAS____method_pstr_____equal(struct __CBAS__type__pstr* __cbas_local_variable_mangled_name_this,struct __CBAS__type__pstr* __cbas_local_variable_mangled_name_o)
{
    if(((unsigned long)((((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_this))->__CBAS__member__len))) != ((unsigned long)((((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_o))->__CBAS__member__len)))){

    
    return (long)(0ul);
    }

    if(((unsigned long)((((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_this))->__CBAS__member__len))) == (0ul)){

    
    return (long)(1ul);
    }

    
    return (__CBAS__memcmp(((unsigned char*)((((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_this))->__CBAS__member__d))),((unsigned char*)((((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_o))->__CBAS__member__d))),((unsigned long)((((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_this))->__CBAS__member__len))))) == ((long)(0ul));
    
}
static inline long __CBAS____method_pstr_____nequal(struct __CBAS__type__pstr* __cbas_local_variable_mangled_name_this,struct __CBAS__type__pstr* __cbas_local_variable_mangled_name_o)
{
    
    return (long)(!(__CBAS____method_pstr_____equal(((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_this)),((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_o)))));
    
}
static inline struct __CBAS__type__str* __CBAS____method_str_____copy(struct __CBAS__type__str* __cbas_local_variable_mangled_name_this,struct __CBAS__type__str* __cbas_local_variable_mangled_name_o)
{
    __CBAS____method_str_____dtor(((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_this)));
    ((((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_this))->__CBAS__member__len)) = ((unsigned long)((((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_o))->__CBAS__member__len)));
    ((((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_this))->__CBAS__member__d)) = (__CBAS__realloc(((unsigned char*)(0ul)),(((unsigned long)((((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_o))->__CBAS__member__len))) + (1ul))));
    if(((unsigned char*)((((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_this))->__CBAS__member__d))) == ((unsigned char*)(0ul))){

    
    return (struct __CBAS__type__str*)(0ul);
    }

    __CBAS__mcpy(((unsigned char*)((((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_this))->__CBAS__member__d))),((unsigned char*)((((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_o))->__CBAS__member__d))),(((unsigned long)((((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_this))->__CBAS__member__len))) + (1ul)));
    
    return (struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_this);
    
}
static inline struct __CBAS__type__str* __CBAS____method_str_____frompstr(struct __CBAS__type__str* __cbas_local_variable_mangled_name_this,struct __CBAS__type__pstr* __cbas_local_variable_mangled_name_o)
{
    __CBAS____method_str_____dtor(((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_this)));
    ((((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_this))->__CBAS__member__len)) = ((unsigned long)((((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_o))->__CBAS__member__len)));
    ((((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_this))->__CBAS__member__d)) = (__CBAS__realloc(((unsigned char*)(0ul)),(((unsigned long)((((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_o))->__CBAS__member__len))) + (1ul))));
    if(((unsigned char*)((((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_this))->__CBAS__member__d))) == ((unsigned char*)(0ul))){

    
    return (struct __CBAS__type__str*)(0ul);
    }

    __CBAS__mcpy(((unsigned char*)((((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_this))->__CBAS__member__d))),((unsigned char*)((((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_o))->__CBAS__member__d))),((unsigned long)((((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_this))->__CBAS__member__len))));
    (((unsigned char*)((((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_this))->__CBAS__member__d)))[(long)((((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_this))->__CBAS__member__len))]) = ((unsigned char)(0ul));
    ((((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_this))->__CBAS__member__len)) = (__CBAS__strlen(((unsigned char*)((((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_this))->__CBAS__member__d)))));
    
    return (struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_this);
    
}
static inline struct __CBAS__type__pstr* __CBAS____method_pstr_____copy(struct __CBAS__type__pstr* __cbas_local_variable_mangled_name_this,struct __CBAS__type__pstr* __cbas_local_variable_mangled_name_o)
{
    __CBAS____method_pstr_____dtor(((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_this)));
    ((((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_this))->__CBAS__member__len)) = ((unsigned long)((((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_o))->__CBAS__member__len)));
    ((((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_this))->__CBAS__member__d)) = (__CBAS__realloc(((unsigned char*)(0ul)),((unsigned long)((((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_o))->__CBAS__member__len)))));
    if(((unsigned char*)((((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_this))->__CBAS__member__d))) == ((unsigned char*)(0ul))){

    
    return (struct __CBAS__type__pstr*)(0ul);
    }

    __CBAS__mcpy(((unsigned char*)((((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_this))->__CBAS__member__d))),((unsigned char*)((((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_o))->__CBAS__member__d))),((unsigned long)((((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_this))->__CBAS__member__len))));
    
    return (struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_this);
    
}
static inline struct __CBAS__type__pstr* __CBAS____method_pstr_____fromstr(struct __CBAS__type__pstr* __cbas_local_variable_mangled_name_this,struct __CBAS__type__str* __cbas_local_variable_mangled_name_o)
{
    __CBAS____method_pstr_____dtor(((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_this)));
    ((((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_this))->__CBAS__member__len)) = ((unsigned long)((((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_o))->__CBAS__member__len)));
    ((((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_this))->__CBAS__member__d)) = (__CBAS__realloc(((unsigned char*)(0ul)),((unsigned long)((((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_o))->__CBAS__member__len)))));
    if(((unsigned char*)((((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_this))->__CBAS__member__d))) == ((unsigned char*)(0ul))){

    
    return (struct __CBAS__type__pstr*)(0ul);
    }

    __CBAS__mcpy(((unsigned char*)((((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_this))->__CBAS__member__d))),((unsigned char*)((((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_o))->__CBAS__member__d))),((unsigned long)((((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_this))->__CBAS__member__len))));
    
    return (struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_this);
    
}
static inline struct __CBAS__type__str* __CBAS____method_str_____new(struct __CBAS__type__str* __cbas_local_variable_mangled_name_this,unsigned char* __cbas_local_variable_mangled_name_o)
{
    unsigned long __cbas_local_variable_mangled_name_qq;
    __CBAS____method_str_____dtor(((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_this)));
    (__cbas_local_variable_mangled_name_qq) = (__CBAS__strlen(((unsigned char*)(__cbas_local_variable_mangled_name_o))));
    ((((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_this))->__CBAS__member__len)) = ((unsigned long)(__cbas_local_variable_mangled_name_qq));
    ((((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_this))->__CBAS__member__d)) = (__CBAS__realloc(((unsigned char*)(0ul)),(((unsigned long)(__cbas_local_variable_mangled_name_qq)) + (1ul))));
    if(((unsigned char*)((((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_this))->__CBAS__member__d))) == ((unsigned char*)(0ul))){

    
    return (struct __CBAS__type__str*)(0ul);
    }

    __CBAS__mcpy(((unsigned char*)((((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_this))->__CBAS__member__d))),((unsigned char*)(__cbas_local_variable_mangled_name_o)),(((unsigned long)(__cbas_local_variable_mangled_name_qq)) + (1ul)));
    
    return (struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_this);
    
}
static inline struct __CBAS__type__pstr* __CBAS____method_pstr_____new(struct __CBAS__type__pstr* __cbas_local_variable_mangled_name_this,unsigned char* __cbas_local_variable_mangled_name_o)
{
    unsigned long __cbas_local_variable_mangled_name_qq;
    __CBAS____method_pstr_____dtor(((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_this)));
    (__cbas_local_variable_mangled_name_qq) = (__CBAS__strlen(((unsigned char*)(__cbas_local_variable_mangled_name_o))));
    ((((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_this))->__CBAS__member__len)) = ((unsigned long)(__cbas_local_variable_mangled_name_qq));
    ((((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_this))->__CBAS__member__d)) = (__CBAS__realloc(((unsigned char*)(0ul)),((unsigned long)(__cbas_local_variable_mangled_name_qq))));
    if(((unsigned char*)((((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_this))->__CBAS__member__d))) == ((unsigned char*)(0ul))){

    
    return (struct __CBAS__type__pstr*)(0ul);
    }

    __CBAS__mcpy(((unsigned char*)((((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_this))->__CBAS__member__d))),((unsigned char*)(__cbas_local_variable_mangled_name_o)),((unsigned long)(__cbas_local_variable_mangled_name_qq)));
    
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
    ((((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_this))->__CBAS__member__len)) = ((unsigned long)((((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_o))->__CBAS__member__len)));
    ((((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_o))->__CBAS__member__d)) = ((unsigned char*)(0ul));
    ((((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_o))->__CBAS__member__len)) = (0ul);
    
}
static inline void __CBAS____method_pstr_____move(struct __CBAS__type__pstr* __cbas_local_variable_mangled_name_this,struct __CBAS__type__pstr* __cbas_local_variable_mangled_name_o)
{
    ((((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_this))->__CBAS__member__d)) = ((unsigned char*)((((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_o))->__CBAS__member__d)));
    ((((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_this))->__CBAS__member__len)) = ((unsigned long)((((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_o))->__CBAS__member__len)));
    ((((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_o))->__CBAS__member__d)) = ((unsigned char*)(0ul));
    ((((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_o))->__CBAS__member__len)) = (0ul);
    
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
    if(((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_p)) == ((struct __CBAS__type__str*)(0ul))){

    
    return (struct __CBAS__type__str*)(0ul);
    }

    __CBAS____method_str_____copy(((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_p)),((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_this)));
    
    return (struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_p);
    
}
static inline struct __CBAS__type__pstr* __CBAS____method_pstr_____clone(struct __CBAS__type__pstr* __cbas_local_variable_mangled_name_this)
{
    struct __CBAS__type__pstr* __cbas_local_variable_mangled_name_p;
    (__cbas_local_variable_mangled_name_p) = ((struct __CBAS__type__pstr*)(__CBAS__malloc((sizeof(struct __CBAS__type__pstr)))));
    if(((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_p)) == ((struct __CBAS__type__pstr*)(0ul))){

    
    return (struct __CBAS__type__pstr*)(0ul);
    }

    __CBAS____method_pstr_____copy(((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_p)),((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_this)));
    
    return (struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_p);
    
}
static inline unsigned long __CBAS____method_str_____isnull(struct __CBAS__type__str* __cbas_local_variable_mangled_name_this)
{
    
    return (unsigned long)(((unsigned char*)((((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_this))->__CBAS__member__d))) == ((unsigned char*)(0ul)));
    
}
static inline unsigned long __CBAS____method_pstr_____isnull(struct __CBAS__type__pstr* __cbas_local_variable_mangled_name_this)
{
    
    return (unsigned long)(((unsigned char*)((((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_this))->__CBAS__member__d))) == ((unsigned char*)(0ul)));
    
}
static inline unsigned long __CBAS____method_str_____isempty(struct __CBAS__type__str* __cbas_local_variable_mangled_name_this)
{
    if((long)(((unsigned long)(((unsigned long)((((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_this))->__CBAS__member__len))) == (0ul))) || ((unsigned long)(((unsigned char*)((((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_this))->__CBAS__member__d))) == ((unsigned char*)(0ul)))))){

    
    return 1ul;
    }

    
    return 0ul;
    
}
static inline unsigned long __CBAS____method_str_____empty(struct __CBAS__type__str* __cbas_local_variable_mangled_name_this)
{
    
    return __CBAS____method_str_____isempty(((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_this)));
    
}
static inline unsigned long __CBAS____method_pstr_____isempty(struct __CBAS__type__pstr* __cbas_local_variable_mangled_name_this)
{
    if((long)(((unsigned long)(((unsigned long)((((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_this))->__CBAS__member__len))) == (0ul))) || ((unsigned long)(((unsigned char*)((((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_this))->__CBAS__member__d))) == ((unsigned char*)(0ul)))))){

    
    return 1ul;
    }

    
    return 0ul;
    
}
static inline unsigned long __CBAS____method_pstr_____empty(struct __CBAS__type__pstr* __cbas_local_variable_mangled_name_this)
{
    
    return __CBAS____method_pstr_____isempty(((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_this)));
    
}
static inline unsigned long __CBAS____method_str_____prefixed(struct __CBAS__type__str* __cbas_local_variable_mangled_name_this,struct __CBAS__type__str* __cbas_local_variable_mangled_name_o)
{
    unsigned char* __cbas_local_variable_mangled_name_a;
    unsigned char* __cbas_local_variable_mangled_name_b;
    if((long)(__CBAS____method_str_____isempty(((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_o))))){

    
    return 1ul;
    }

    if((long)(__CBAS____method_str_____isempty(((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_this))))){

    
    return 0ul;
    }

    if(((unsigned long)((((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_o))->__CBAS__member__len))) > ((unsigned long)((((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_this))->__CBAS__member__len)))){

    
    return 0ul;
    }

    (__cbas_local_variable_mangled_name_a) = ((unsigned char*)((((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_o))->__CBAS__member__d)));
    (__cbas_local_variable_mangled_name_b) = ((unsigned char*)((((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_this))->__CBAS__member__d)));
    while((long)(1ul)){

    if(((unsigned long)(((unsigned char*)(__cbas_local_variable_mangled_name_a))[(long)(0ul)])) == (0ul)){

    
    return 1ul;
    }

    if(((unsigned char)(((unsigned char*)(__cbas_local_variable_mangled_name_a))[(long)(0ul)])) != ((unsigned char)(((unsigned char*)(__cbas_local_variable_mangled_name_b))[(long)(0ul)]))){

    
    return 0ul;
    }

    (__cbas_local_variable_mangled_name_a)++;
    (__cbas_local_variable_mangled_name_b)++;
    }

    
}
static inline unsigned long __CBAS____method_pstr_____prefixed(struct __CBAS__type__pstr* __cbas_local_variable_mangled_name_this,struct __CBAS__type__pstr* __cbas_local_variable_mangled_name_o)
{
    unsigned char* __cbas_local_variable_mangled_name_a;
    unsigned char* __cbas_local_variable_mangled_name_b;
    unsigned long __cbas_local_variable_mangled_name_i;
    if((long)(__CBAS____method_pstr_____isempty(((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_o))))){

    
    return 1ul;
    }

    if((long)(__CBAS____method_pstr_____isempty(((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_this))))){

    
    return 0ul;
    }

    if(((unsigned long)((((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_o))->__CBAS__member__len))) > ((unsigned long)((((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_this))->__CBAS__member__len)))){

    
    return 0ul;
    }

    (__cbas_local_variable_mangled_name_a) = ((unsigned char*)((((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_o))->__CBAS__member__d)));
    (__cbas_local_variable_mangled_name_b) = ((unsigned char*)((((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_this))->__CBAS__member__d)));
    for((__cbas_local_variable_mangled_name_i) = (0ul);((unsigned long)(__cbas_local_variable_mangled_name_i)) < ((unsigned long)((((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_o))->__CBAS__member__len)));(__cbas_local_variable_mangled_name_i)++){

    if(((unsigned char)(((unsigned char*)(__cbas_local_variable_mangled_name_a))[(long)(0ul)])) != ((unsigned char)(((unsigned char*)(__cbas_local_variable_mangled_name_b))[(long)(0ul)]))){

    
    return 0ul;
    }

    (__cbas_local_variable_mangled_name_a)++;
    (__cbas_local_variable_mangled_name_b)++;
    }

    
    return 1ul;
    
}
static inline unsigned long __CBAS____method_str_____postfixed(struct __CBAS__type__str* __cbas_local_variable_mangled_name_this,struct __CBAS__type__str* __cbas_local_variable_mangled_name_o)
{
    unsigned char* __cbas_local_variable_mangled_name_a;
    unsigned char* __cbas_local_variable_mangled_name_b;
    if((long)(__CBAS____method_str_____isempty(((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_o))))){

    
    return 1ul;
    }

    if((long)(__CBAS____method_str_____isempty(((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_this))))){

    
    return 0ul;
    }

    if(((unsigned long)((((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_o))->__CBAS__member__len))) > ((unsigned long)((((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_this))->__CBAS__member__len)))){

    
    return 0ul;
    }

    (__cbas_local_variable_mangled_name_a) = ((unsigned char*)((((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_o))->__CBAS__member__d)));
    (__cbas_local_variable_mangled_name_b) = (((unsigned char*)((((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_this))->__CBAS__member__d))) + ((long)(((unsigned long)((((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_this))->__CBAS__member__len))) - ((unsigned long)((((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_o))->__CBAS__member__len))))));
    while((long)(1ul)){

    if(((unsigned long)(((unsigned char*)(__cbas_local_variable_mangled_name_a))[(long)(0ul)])) == (0ul)){

    
    return 1ul;
    }

    if(((unsigned char)(((unsigned char*)(__cbas_local_variable_mangled_name_a))[(long)(0ul)])) != ((unsigned char)(((unsigned char*)(__cbas_local_variable_mangled_name_b))[(long)(0ul)]))){

    
    return 0ul;
    }

    (__cbas_local_variable_mangled_name_a)++;
    (__cbas_local_variable_mangled_name_b)++;
    }

    
}
static inline unsigned long __CBAS____method_pstr_____postfixed(struct __CBAS__type__pstr* __cbas_local_variable_mangled_name_this,struct __CBAS__type__pstr* __cbas_local_variable_mangled_name_o)
{
    unsigned char* __cbas_local_variable_mangled_name_a;
    unsigned char* __cbas_local_variable_mangled_name_b;
    unsigned long __cbas_local_variable_mangled_name_i;
    if((long)(__CBAS____method_pstr_____isempty(((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_o))))){

    
    return 1ul;
    }

    if((long)(__CBAS____method_pstr_____isempty(((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_this))))){

    
    return 0ul;
    }

    if(((unsigned long)((((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_o))->__CBAS__member__len))) > ((unsigned long)((((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_this))->__CBAS__member__len)))){

    
    return 0ul;
    }

    (__cbas_local_variable_mangled_name_a) = ((unsigned char*)((((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_o))->__CBAS__member__d)));
    (__cbas_local_variable_mangled_name_b) = (((unsigned char*)((((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_this))->__CBAS__member__d))) + ((long)(((unsigned long)((((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_this))->__CBAS__member__len))) - ((unsigned long)((((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_o))->__CBAS__member__len))))));
    for((__cbas_local_variable_mangled_name_i) = (0ul);((unsigned long)(__cbas_local_variable_mangled_name_i)) < ((unsigned long)((((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_o))->__CBAS__member__len)));(__cbas_local_variable_mangled_name_i)++){

    if(((unsigned char)(((unsigned char*)(__cbas_local_variable_mangled_name_a))[(long)(0ul)])) != ((unsigned char)(((unsigned char*)(__cbas_local_variable_mangled_name_b))[(long)(0ul)]))){

    
    return 0ul;
    }

    (__cbas_local_variable_mangled_name_a)++;
    (__cbas_local_variable_mangled_name_b)++;
    }

    
    return 1ul;
    
}
static inline long __CBAS____method_str_____findfrom(struct __CBAS__type__str* __cbas_local_variable_mangled_name_this,struct __CBAS__type__str* __cbas_local_variable_mangled_name_findme,unsigned long __cbas_local_variable_mangled_name_where)
{
    long __cbas_local_variable_mangled_name_retval;
    unsigned char* __cbas_local_variable_mangled_name_text;
    unsigned char* __cbas_local_variable_mangled_name_subtext;
    long __cbas_local_variable_mangled_name_ti;
    long __cbas_local_variable_mangled_name_si;
    long __cbas_local_variable_mangled_name_st;
    (__cbas_local_variable_mangled_name_retval) = (-((long)(1ul)));
    if((long)(__CBAS____method_str_____isempty(((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_findme))))){

    
    return (long)(0ul);
    }

    if((long)(__CBAS____method_str_____isempty(((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_this))))){

    
    return -((long)(1ul));
    }

    if(((unsigned long)((((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_findme))->__CBAS__member__len))) > ((unsigned long)((((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_this))->__CBAS__member__len)))){

    
    return -((long)(1ul));
    }

    if(((unsigned long)(__cbas_local_variable_mangled_name_where)) >= ((unsigned long)((((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_this))->__CBAS__member__len)))){

    
    return -((long)(1ul));
    }

    (__cbas_local_variable_mangled_name_text) = ((unsigned char*)((((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_this))->__CBAS__member__d)));
    (__cbas_local_variable_mangled_name_subtext) = ((unsigned char*)((((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_findme))->__CBAS__member__d)));
    (__cbas_local_variable_mangled_name_ti) = ((long)(__cbas_local_variable_mangled_name_where));
    (__cbas_local_variable_mangled_name_si) = ((long)(0ul));
    (__cbas_local_variable_mangled_name_st) = ((long)((((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_findme))->__CBAS__member__len)));
    for(0ul;((unsigned long)(((unsigned char*)(__cbas_local_variable_mangled_name_text))[(long)(__cbas_local_variable_mangled_name_ti)])) != (0ul);(__cbas_local_variable_mangled_name_ti)++){

    if(((unsigned char)(((unsigned char*)(__cbas_local_variable_mangled_name_text))[(long)(__cbas_local_variable_mangled_name_ti)])) == ((unsigned char)(((unsigned char*)(__cbas_local_variable_mangled_name_subtext))[(long)(__cbas_local_variable_mangled_name_si)]))){

    (__cbas_local_variable_mangled_name_si)++;
    if(((unsigned long)(((unsigned char*)(__cbas_local_variable_mangled_name_subtext))[(long)(__cbas_local_variable_mangled_name_si)])) == (0ul)){

    
    return (((long)(__cbas_local_variable_mangled_name_ti)) - ((long)(__cbas_local_variable_mangled_name_st))) + ((long)(1ul));
    }

    }

    else {
    if(((unsigned long)(((unsigned char*)(__cbas_local_variable_mangled_name_subtext))[(long)(__cbas_local_variable_mangled_name_si)])) == (0ul)){

    
    return (((long)(__cbas_local_variable_mangled_name_ti)) - ((long)(__cbas_local_variable_mangled_name_st))) + ((long)(1ul));
    }

    (__cbas_local_variable_mangled_name_ti) = (((long)(__cbas_local_variable_mangled_name_ti)) - ((long)(__cbas_local_variable_mangled_name_si)));
    (__cbas_local_variable_mangled_name_si) = ((long)(0ul));
    }

    }

    
    return -((long)(1ul));
    
}
static inline long __CBAS____method_pstr_____findfrom(struct __CBAS__type__pstr* __cbas_local_variable_mangled_name_this,struct __CBAS__type__pstr* __cbas_local_variable_mangled_name_findme,unsigned long __cbas_local_variable_mangled_name_where)
{
    long __cbas_local_variable_mangled_name_retval;
    unsigned char* __cbas_local_variable_mangled_name_text;
    unsigned char* __cbas_local_variable_mangled_name_subtext;
    long __cbas_local_variable_mangled_name_ti;
    long __cbas_local_variable_mangled_name_si;
    long __cbas_local_variable_mangled_name_st;
    (__cbas_local_variable_mangled_name_retval) = (-((long)(1ul)));
    if((long)(__CBAS____method_pstr_____isempty(((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_findme))))){

    
    return (long)(0ul);
    }

    if((long)(__CBAS____method_pstr_____isempty(((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_this))))){

    
    return -((long)(1ul));
    }

    if(((unsigned long)((((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_findme))->__CBAS__member__len))) > ((unsigned long)((((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_this))->__CBAS__member__len)))){

    
    return -((long)(1ul));
    }

    if(((unsigned long)(__cbas_local_variable_mangled_name_where)) >= ((unsigned long)((((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_this))->__CBAS__member__len)))){

    
    return -((long)(1ul));
    }

    (__cbas_local_variable_mangled_name_text) = ((unsigned char*)((((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_this))->__CBAS__member__d)));
    (__cbas_local_variable_mangled_name_subtext) = ((unsigned char*)((((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_findme))->__CBAS__member__d)));
    (__cbas_local_variable_mangled_name_ti) = ((long)(__cbas_local_variable_mangled_name_where));
    (__cbas_local_variable_mangled_name_si) = ((long)(0ul));
    (__cbas_local_variable_mangled_name_st) = ((long)((((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_findme))->__CBAS__member__len)));
    for(0ul;((long)(__cbas_local_variable_mangled_name_ti)) < ((long)((((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_this))->__CBAS__member__len)));(__cbas_local_variable_mangled_name_ti)++){

    if(((unsigned char)(((unsigned char*)(__cbas_local_variable_mangled_name_text))[(long)(__cbas_local_variable_mangled_name_ti)])) == ((unsigned char)(((unsigned char*)(__cbas_local_variable_mangled_name_subtext))[(long)(__cbas_local_variable_mangled_name_si)]))){

    (__cbas_local_variable_mangled_name_si)++;
    if(((long)(__cbas_local_variable_mangled_name_si)) >= ((long)((((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_findme))->__CBAS__member__len)))){

    
    return (((long)(__cbas_local_variable_mangled_name_ti)) - ((long)(__cbas_local_variable_mangled_name_st))) + ((long)(1ul));
    }

    }

    else {
    if(((long)(__cbas_local_variable_mangled_name_si)) >= ((long)((((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_findme))->__CBAS__member__len)))){

    
    return (((long)(__cbas_local_variable_mangled_name_ti)) - ((long)(__cbas_local_variable_mangled_name_st))) + ((long)(1ul));
    }

    (__cbas_local_variable_mangled_name_ti) = (((long)(__cbas_local_variable_mangled_name_ti)) - ((long)(__cbas_local_variable_mangled_name_si)));
    (__cbas_local_variable_mangled_name_si) = ((long)(0ul));
    }

    }

    
    return -((long)(1ul));
    
}
static inline long __CBAS____method_str_____find(struct __CBAS__type__str* __cbas_local_variable_mangled_name_this,struct __CBAS__type__str* __cbas_local_variable_mangled_name_findme)
{
    
    return __CBAS____method_str_____findfrom(((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_this)),((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_findme)),(0ul));
    
}
static inline long __CBAS____method_pstr_____find(struct __CBAS__type__pstr* __cbas_local_variable_mangled_name_this,struct __CBAS__type__pstr* __cbas_local_variable_mangled_name_findme)
{
    
    return __CBAS____method_pstr_____findfrom(((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_this)),((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_findme)),(0ul));
    
}
static inline unsigned long __CBAS____method_str_____add(struct __CBAS__type__str* __cbas_local_variable_mangled_name_this,struct __CBAS__type__str* __cbas_local_variable_mangled_name_o)
{
    unsigned long __cbas_local_variable_mangled_name_old_len;
    (__cbas_local_variable_mangled_name_old_len) = ((unsigned long)((((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_this))->__CBAS__member__len)));
    if((long)(__CBAS____method_str_____isempty(((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_o))))){

    
    return 0ul;
    }

    ((((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_this))->__CBAS__member__len)) = ((((unsigned long)((((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_o))->__CBAS__member__len))) + ((unsigned long)((((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_this))->__CBAS__member__len)))) + (1ul));
    ((((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_this))->__CBAS__member__d)) = (__CBAS__realloc(((unsigned char*)((((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_this))->__CBAS__member__d))),((((unsigned long)((((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_this))->__CBAS__member__len))) + ((unsigned long)((((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_o))->__CBAS__member__len)))) + (1ul))));
    if(((unsigned char*)((((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_this))->__CBAS__member__d))) == ((unsigned char*)(0ul))){

    
    return 1ul;
    }

    __CBAS__mcpy((((unsigned char*)((((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_this))->__CBAS__member__d))) + ((long)(__cbas_local_variable_mangled_name_old_len))),((unsigned char*)((((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_o))->__CBAS__member__d))),(((unsigned long)((((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_o))->__CBAS__member__len))) + (1ul)));
    
    return 0ul;
    
}
static inline unsigned long __CBAS____method_pstr_____add(struct __CBAS__type__pstr* __cbas_local_variable_mangled_name_this,struct __CBAS__type__pstr* __cbas_local_variable_mangled_name_o)
{
    unsigned long __cbas_local_variable_mangled_name_old_len;
    (__cbas_local_variable_mangled_name_old_len) = ((unsigned long)((((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_this))->__CBAS__member__len)));
    if((long)(__CBAS____method_pstr_____isempty(((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_o))))){

    
    return 0ul;
    }

    ((((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_this))->__CBAS__member__len)) = (((unsigned long)((((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_o))->__CBAS__member__len))) + ((unsigned long)((((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_this))->__CBAS__member__len))));
    ((((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_this))->__CBAS__member__d)) = (__CBAS__realloc(((unsigned char*)((((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_this))->__CBAS__member__d))),(((unsigned long)((((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_this))->__CBAS__member__len))) + ((unsigned long)((((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_o))->__CBAS__member__len))))));
    if(((unsigned char*)((((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_this))->__CBAS__member__d))) == ((unsigned char*)(0ul))){

    
    return 1ul;
    }

    __CBAS__mcpy((((unsigned char*)((((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_this))->__CBAS__member__d))) + ((long)(__cbas_local_variable_mangled_name_old_len))),((unsigned char*)((((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_o))->__CBAS__member__d))),((unsigned long)((((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_o))->__CBAS__member__len))));
    
    return 0ul;
    
}
static inline unsigned long __CBAS____method_str_____addc(struct __CBAS__type__str* __cbas_local_variable_mangled_name_this,unsigned char* __cbas_local_variable_mangled_name_oo)
{
    unsigned long __cbas_local_variable_mangled_name_qq;
    unsigned long __cbas_local_variable_mangled_name_old_len;
    (__cbas_local_variable_mangled_name_qq) = (0ul);
    if(((unsigned char*)(__cbas_local_variable_mangled_name_oo)) == ((unsigned char*)(0ul))){

    
    return 0ul;
    }

    (__cbas_local_variable_mangled_name_qq) = (__CBAS__strlen(((unsigned char*)(__cbas_local_variable_mangled_name_oo))));
    if(((unsigned long)(__cbas_local_variable_mangled_name_qq)) == (0ul)){

    
    return 0ul;
    }

    (__cbas_local_variable_mangled_name_old_len) = ((unsigned long)((((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_this))->__CBAS__member__len)));
    ((((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_this))->__CBAS__member__len)) = ((((unsigned long)(__cbas_local_variable_mangled_name_qq)) + ((unsigned long)((((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_this))->__CBAS__member__len)))) + (1ul));
    ((((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_this))->__CBAS__member__d)) = (__CBAS__realloc(((unsigned char*)((((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_this))->__CBAS__member__d))),((((unsigned long)((((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_this))->__CBAS__member__len))) + ((unsigned long)(__cbas_local_variable_mangled_name_qq))) + (1ul))));
    if(((unsigned char*)((((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_this))->__CBAS__member__d))) == ((unsigned char*)(0ul))){

    
    return 1ul;
    }

    __CBAS__mcpy((((unsigned char*)((((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_this))->__CBAS__member__d))) + ((long)(__cbas_local_variable_mangled_name_old_len))),((unsigned char*)(__cbas_local_variable_mangled_name_oo)),(((unsigned long)(__cbas_local_variable_mangled_name_qq)) + (1ul)));
    
    return 0ul;
    
}
static inline unsigned long __CBAS____method_pstr_____addc(struct __CBAS__type__pstr* __cbas_local_variable_mangled_name_this,unsigned char* __cbas_local_variable_mangled_name_oo)
{
    unsigned long __cbas_local_variable_mangled_name_qq;
    unsigned long __cbas_local_variable_mangled_name_old_len;
    (__cbas_local_variable_mangled_name_qq) = (0ul);
    if(((unsigned char*)(__cbas_local_variable_mangled_name_oo)) == ((unsigned char*)(0ul))){

    
    return 0ul;
    }

    (__cbas_local_variable_mangled_name_qq) = (__CBAS__strlen(((unsigned char*)(__cbas_local_variable_mangled_name_oo))));
    if(((unsigned long)(__cbas_local_variable_mangled_name_qq)) == (0ul)){

    
    return 0ul;
    }

    (__cbas_local_variable_mangled_name_old_len) = ((unsigned long)((((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_this))->__CBAS__member__len)));
    ((((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_this))->__CBAS__member__len)) = (((unsigned long)(__cbas_local_variable_mangled_name_qq)) + ((unsigned long)((((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_this))->__CBAS__member__len))));
    ((((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_this))->__CBAS__member__d)) = (__CBAS__realloc(((unsigned char*)((((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_this))->__CBAS__member__d))),(((unsigned long)((((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_this))->__CBAS__member__len))) + ((unsigned long)(__cbas_local_variable_mangled_name_qq)))));
    if(((unsigned char*)((((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_this))->__CBAS__member__d))) == ((unsigned char*)(0ul))){

    
    return 1ul;
    }

    __CBAS__mcpy((((unsigned char*)((((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_this))->__CBAS__member__d))) + ((long)(__cbas_local_variable_mangled_name_old_len))),((unsigned char*)(__cbas_local_variable_mangled_name_oo)),((unsigned long)(__cbas_local_variable_mangled_name_qq)));
    
    return 0ul;
    
}
static inline struct __CBAS__type__str* __CBAS____method_str_____substr(struct __CBAS__type__str* __cbas_local_variable_mangled_name_this,unsigned long __cbas_local_variable_mangled_name_beginning,unsigned long __cbas_local_variable_mangled_name_howmuch)
{
    unsigned char* __cbas_local_variable_mangled_name_b;
    struct __CBAS__type__str* __cbas_local_variable_mangled_name_p;
    if((long)(((unsigned long)(((unsigned long)(__cbas_local_variable_mangled_name_beginning)) >= ((unsigned long)((((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_this))->__CBAS__member__len))))) || ((unsigned long)((((unsigned long)(__cbas_local_variable_mangled_name_beginning)) + ((unsigned long)(__cbas_local_variable_mangled_name_howmuch))) >= ((unsigned long)((((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_this))->__CBAS__member__len))))))){

    
    return (struct __CBAS__type__str*)(0ul);
    }

    if(((unsigned long)(__cbas_local_variable_mangled_name_howmuch)) == (0ul)){

    struct __CBAS__type__str* __cbas_local_variable_mangled_name_p2;
    (__cbas_local_variable_mangled_name_p2) = ((struct __CBAS__type__str*)(__CBAS__malloc((sizeof(struct __CBAS__type__str)))));
    if(((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_p2)) == ((struct __CBAS__type__str*)(0ul))){

    
    return (struct __CBAS__type__str*)(0ul);
    }

    __CBAS____method_str_____ctor(((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_p2)));
    
    return (struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_p2);
    }

    (__cbas_local_variable_mangled_name_b) = (__CBAS__malloc((((unsigned long)(__cbas_local_variable_mangled_name_howmuch)) + (1ul))));
    if(((unsigned char*)(__cbas_local_variable_mangled_name_b)) == ((unsigned char*)(0ul))){

    
    return (struct __CBAS__type__str*)(0ul);
    }

    __CBAS__mcpy(((unsigned char*)(__cbas_local_variable_mangled_name_b)),(((unsigned char*)((((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_this))->__CBAS__member__d))) + ((long)(__cbas_local_variable_mangled_name_beginning))),((unsigned long)(__cbas_local_variable_mangled_name_howmuch)));
    (((unsigned char*)(__cbas_local_variable_mangled_name_b))[(long)(__cbas_local_variable_mangled_name_howmuch)]) = ((unsigned char)(0ul));
    (__cbas_local_variable_mangled_name_p) = ((struct __CBAS__type__str*)(__CBAS__malloc((sizeof(struct __CBAS__type__str)))));
    if(((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_p)) == ((struct __CBAS__type__str*)(0ul))){

    __CBAS__free(((unsigned char*)(__cbas_local_variable_mangled_name_b)));
    
    return (struct __CBAS__type__str*)(0ul);
    }

    ((((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_p))->__CBAS__member__d)) = ((unsigned char*)(__cbas_local_variable_mangled_name_b));
    ((((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_p))->__CBAS__member__len)) = ((unsigned long)(__cbas_local_variable_mangled_name_howmuch));
    
    return (struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_p);
    
}
static inline struct __CBAS__type__pstr* __CBAS____method_pstr_____substr(struct __CBAS__type__pstr* __cbas_local_variable_mangled_name_this,unsigned long __cbas_local_variable_mangled_name_beginning,unsigned long __cbas_local_variable_mangled_name_howmuch)
{
    unsigned char* __cbas_local_variable_mangled_name_b;
    struct __CBAS__type__pstr* __cbas_local_variable_mangled_name_p;
    if((long)(((unsigned long)(((unsigned long)(__cbas_local_variable_mangled_name_beginning)) >= ((unsigned long)((((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_this))->__CBAS__member__len))))) || ((unsigned long)((((unsigned long)(__cbas_local_variable_mangled_name_beginning)) + ((unsigned long)(__cbas_local_variable_mangled_name_howmuch))) >= ((unsigned long)((((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_this))->__CBAS__member__len))))))){

    
    return (struct __CBAS__type__pstr*)(0ul);
    }

    if(((unsigned long)(__cbas_local_variable_mangled_name_howmuch)) == (0ul)){

    struct __CBAS__type__pstr* __cbas_local_variable_mangled_name_p2;
    (__cbas_local_variable_mangled_name_p2) = ((struct __CBAS__type__pstr*)(__CBAS__malloc((sizeof(struct __CBAS__type__pstr)))));
    if(((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_p2)) == ((struct __CBAS__type__pstr*)(0ul))){

    
    return (struct __CBAS__type__pstr*)(0ul);
    }

    __CBAS____method_pstr_____ctor(((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_p2)));
    
    return (struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_p2);
    }

    (__cbas_local_variable_mangled_name_b) = (__CBAS__malloc(((unsigned long)(__cbas_local_variable_mangled_name_howmuch))));
    if(((unsigned char*)(__cbas_local_variable_mangled_name_b)) == ((unsigned char*)(0ul))){

    
    return (struct __CBAS__type__pstr*)(0ul);
    }

    __CBAS__mcpy(((unsigned char*)(__cbas_local_variable_mangled_name_b)),(((unsigned char*)((((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_this))->__CBAS__member__d))) + ((long)(__cbas_local_variable_mangled_name_beginning))),((unsigned long)(__cbas_local_variable_mangled_name_howmuch)));
    (__cbas_local_variable_mangled_name_p) = ((struct __CBAS__type__pstr*)(__CBAS__malloc((sizeof(struct __CBAS__type__str)))));
    if(((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_p)) == ((struct __CBAS__type__pstr*)(0ul))){

    __CBAS__free(((unsigned char*)(__cbas_local_variable_mangled_name_b)));
    
    return (struct __CBAS__type__pstr*)(0ul);
    }

    ((((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_p))->__CBAS__member__d)) = ((unsigned char*)(__cbas_local_variable_mangled_name_b));
    ((((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_p))->__CBAS__member__len)) = ((unsigned long)(__cbas_local_variable_mangled_name_howmuch));
    
    return (struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_p);
    
}
static inline long __CBAS____method_str_____replace_first_from(struct __CBAS__type__str* __cbas_local_variable_mangled_name_this,struct __CBAS__type__str* __cbas_local_variable_mangled_name_what,struct __CBAS__type__str* __cbas_local_variable_mangled_name_with,unsigned long __cbas_local_variable_mangled_name_maxwhere)
{
    long __cbas_local_variable_mangled_name_where;
    unsigned long __cbas_local_variable_mangled_name_is_shorter;
    unsigned long __cbas_local_variable_mangled_name_is_equal;
    if((long)(__CBAS____method_str_____empty(((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_what))))){

    
    return -((long)(1ul));
    }

    if(((unsigned long)(__cbas_local_variable_mangled_name_maxwhere)) >= ((unsigned long)((((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_this))->__CBAS__member__len)))){

    
    return -((long)(1ul));
    }

    if((long)(__CBAS____method_str_____empty(((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_this))))){

    
    return (long)(0ul);
    }

    (__cbas_local_variable_mangled_name_where) = (__CBAS____method_str_____findfrom(((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_this)),((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_what)),((unsigned long)(__cbas_local_variable_mangled_name_maxwhere))));
    if(((long)(__cbas_local_variable_mangled_name_where)) == (-((long)(1ul)))){

    
    return -((long)(2ul));
    }

    (__cbas_local_variable_mangled_name_is_shorter) = ((unsigned long)(((unsigned long)((((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_with))->__CBAS__member__len))) < ((unsigned long)((((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_what))->__CBAS__member__len)))));
    (__cbas_local_variable_mangled_name_is_equal) = ((unsigned long)(((unsigned long)((((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_with))->__CBAS__member__len))) == ((unsigned long)((((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_what))->__CBAS__member__len)))));
    if((long)(__cbas_local_variable_mangled_name_is_equal)){

    __CBAS__mcpy((((unsigned char*)((((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_this))->__CBAS__member__d))) + ((long)(__cbas_local_variable_mangled_name_where))),((unsigned char*)((((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_with))->__CBAS__member__d))),((unsigned long)((((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_with))->__CBAS__member__len))));
    
    return (long)(__cbas_local_variable_mangled_name_where);
    }

    else if((long)(__cbas_local_variable_mangled_name_is_shorter)){

    __CBAS__mcpy((((unsigned char*)((((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_this))->__CBAS__member__d))) + ((long)(__cbas_local_variable_mangled_name_where))),((unsigned char*)((((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_with))->__CBAS__member__d))),((unsigned long)((((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_with))->__CBAS__member__len))));
    __CBAS__mmove(((((unsigned char*)((((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_this))->__CBAS__member__d))) + ((long)(__cbas_local_variable_mangled_name_where))) + ((long)((((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_with))->__CBAS__member__len)))),((((unsigned char*)((((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_this))->__CBAS__member__d))) + ((long)(__cbas_local_variable_mangled_name_where))) + ((long)((((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_what))->__CBAS__member__len)))),((unsigned long)((((long)((((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_this))->__CBAS__member__len))) - (((long)(__cbas_local_variable_mangled_name_where)) + ((long)((((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_what))->__CBAS__member__len))))) + ((long)(1ul)))));
    ((((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_this))->__CBAS__member__len)) = ((unsigned long)((((long)(__cbas_local_variable_mangled_name_where)) + ((long)((((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_with))->__CBAS__member__len)))) + (((long)((((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_this))->__CBAS__member__len))) - (((long)(__cbas_local_variable_mangled_name_where)) + ((long)((((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_what))->__CBAS__member__len)))))));
    
    return (long)(__cbas_local_variable_mangled_name_where);
    }

    else {
    unsigned char* __cbas_local_variable_mangled_name_resbuf;
    (__cbas_local_variable_mangled_name_resbuf) = (__CBAS__realloc(((unsigned char*)(0ul)),((((unsigned long)((((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_this))->__CBAS__member__len))) + (((unsigned long)((((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_with))->__CBAS__member__len))) - ((unsigned long)((((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_what))->__CBAS__member__len))))) + (1ul))));
    if(((unsigned char*)(__cbas_local_variable_mangled_name_resbuf)) == ((unsigned char*)(0ul))){

    
    return -((long)(1ul));
    }

    __CBAS__mcpy(((unsigned char*)(__cbas_local_variable_mangled_name_resbuf)),((unsigned char*)((((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_this))->__CBAS__member__d))),((unsigned long)(__cbas_local_variable_mangled_name_where)));
    __CBAS__mcpy((((unsigned char*)(__cbas_local_variable_mangled_name_resbuf)) + ((long)(__cbas_local_variable_mangled_name_where))),((unsigned char*)((((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_with))->__CBAS__member__d))),((unsigned long)((((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_with))->__CBAS__member__len))));
    __CBAS__mcpy(((((unsigned char*)(__cbas_local_variable_mangled_name_resbuf)) + ((long)(__cbas_local_variable_mangled_name_where))) + ((long)((((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_with))->__CBAS__member__len)))),((((unsigned char*)((((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_this))->__CBAS__member__d))) + ((long)(__cbas_local_variable_mangled_name_where))) + ((long)((((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_what))->__CBAS__member__len)))),((unsigned long)((((long)((((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_this))->__CBAS__member__len))) - (((long)(__cbas_local_variable_mangled_name_where)) + ((long)((((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_what))->__CBAS__member__len))))) + ((long)(1ul)))));
    __CBAS__free(((unsigned char*)((((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_this))->__CBAS__member__d))));
    ((((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_this))->__CBAS__member__d)) = ((unsigned char*)(__cbas_local_variable_mangled_name_resbuf));
    ((((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_this))->__CBAS__member__len)) = ((unsigned long)((((long)(__cbas_local_variable_mangled_name_where)) + ((long)((((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_with))->__CBAS__member__len)))) + (((long)((((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_this))->__CBAS__member__len))) - (((long)(__cbas_local_variable_mangled_name_where)) + ((long)((((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_what))->__CBAS__member__len)))))));
    
    return (long)(__cbas_local_variable_mangled_name_where);
    }

    
}
static inline long __CBAS____method_pstr_____replace_first_from(struct __CBAS__type__pstr* __cbas_local_variable_mangled_name_this,struct __CBAS__type__pstr* __cbas_local_variable_mangled_name_what,struct __CBAS__type__pstr* __cbas_local_variable_mangled_name_with,unsigned long __cbas_local_variable_mangled_name_maxwhere)
{
    long __cbas_local_variable_mangled_name_where;
    unsigned long __cbas_local_variable_mangled_name_is_shorter;
    unsigned long __cbas_local_variable_mangled_name_is_equal;
    if((long)(__CBAS____method_pstr_____empty(((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_what))))){

    
    return -((long)(1ul));
    }

    if(((unsigned long)(__cbas_local_variable_mangled_name_maxwhere)) >= ((unsigned long)((((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_this))->__CBAS__member__len)))){

    
    return -((long)(1ul));
    }

    if((long)(__CBAS____method_pstr_____empty(((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_this))))){

    
    return (long)(0ul);
    }

    (__cbas_local_variable_mangled_name_where) = (__CBAS____method_pstr_____findfrom(((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_this)),((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_what)),((unsigned long)(__cbas_local_variable_mangled_name_maxwhere))));
    if(((long)(__cbas_local_variable_mangled_name_where)) == (-((long)(1ul)))){

    
    return -((long)(2ul));
    }

    (__cbas_local_variable_mangled_name_is_shorter) = ((unsigned long)(((unsigned long)((((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_with))->__CBAS__member__len))) < ((unsigned long)((((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_what))->__CBAS__member__len)))));
    (__cbas_local_variable_mangled_name_is_equal) = ((unsigned long)(((unsigned long)((((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_with))->__CBAS__member__len))) == ((unsigned long)((((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_what))->__CBAS__member__len)))));
    if((long)(__cbas_local_variable_mangled_name_is_equal)){

    __CBAS__mcpy((((unsigned char*)((((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_this))->__CBAS__member__d))) + ((long)(__cbas_local_variable_mangled_name_where))),((unsigned char*)((((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_with))->__CBAS__member__d))),((unsigned long)((((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_with))->__CBAS__member__len))));
    
    return (long)(__cbas_local_variable_mangled_name_where);
    }

    else if((long)(__cbas_local_variable_mangled_name_is_shorter)){

    __CBAS__mcpy((((unsigned char*)((((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_this))->__CBAS__member__d))) + ((long)(__cbas_local_variable_mangled_name_where))),((unsigned char*)((((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_with))->__CBAS__member__d))),((unsigned long)((((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_with))->__CBAS__member__len))));
    __CBAS__mmove(((((unsigned char*)((((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_this))->__CBAS__member__d))) + ((long)(__cbas_local_variable_mangled_name_where))) + ((long)((((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_with))->__CBAS__member__len)))),((((unsigned char*)((((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_this))->__CBAS__member__d))) + ((long)(__cbas_local_variable_mangled_name_where))) + ((long)((((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_what))->__CBAS__member__len)))),((unsigned long)(((long)((((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_this))->__CBAS__member__len))) - (((long)(__cbas_local_variable_mangled_name_where)) + ((long)((((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_what))->__CBAS__member__len)))))));
    ((((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_this))->__CBAS__member__len)) = ((unsigned long)((((long)(__cbas_local_variable_mangled_name_where)) + ((long)((((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_with))->__CBAS__member__len)))) + (((long)((((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_this))->__CBAS__member__len))) - (((long)(__cbas_local_variable_mangled_name_where)) + ((long)((((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_what))->__CBAS__member__len)))))));
    
    return (long)(__cbas_local_variable_mangled_name_where);
    }

    else {
    unsigned char* __cbas_local_variable_mangled_name_resbuf;
    (__cbas_local_variable_mangled_name_resbuf) = (__CBAS__realloc(((unsigned char*)(0ul)),(((unsigned long)((((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_this))->__CBAS__member__len))) + (((unsigned long)((((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_with))->__CBAS__member__len))) - ((unsigned long)((((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_what))->__CBAS__member__len)))))));
    if(((unsigned char*)(__cbas_local_variable_mangled_name_resbuf)) == ((unsigned char*)(0ul))){

    
    return -((long)(1ul));
    }

    __CBAS__mcpy(((unsigned char*)(__cbas_local_variable_mangled_name_resbuf)),((unsigned char*)((((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_this))->__CBAS__member__d))),((unsigned long)(__cbas_local_variable_mangled_name_where)));
    __CBAS__mcpy((((unsigned char*)(__cbas_local_variable_mangled_name_resbuf)) + ((long)(__cbas_local_variable_mangled_name_where))),((unsigned char*)((((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_with))->__CBAS__member__d))),((unsigned long)((((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_with))->__CBAS__member__len))));
    __CBAS__mcpy(((((unsigned char*)(__cbas_local_variable_mangled_name_resbuf)) + ((long)(__cbas_local_variable_mangled_name_where))) + ((long)((((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_with))->__CBAS__member__len)))),((((unsigned char*)((((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_this))->__CBAS__member__d))) + ((long)(__cbas_local_variable_mangled_name_where))) + ((long)((((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_what))->__CBAS__member__len)))),((unsigned long)(((long)((((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_this))->__CBAS__member__len))) - (((long)(__cbas_local_variable_mangled_name_where)) + ((long)((((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_what))->__CBAS__member__len)))))));
    __CBAS__free(((unsigned char*)((((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_this))->__CBAS__member__d))));
    ((((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_this))->__CBAS__member__d)) = ((unsigned char*)(__cbas_local_variable_mangled_name_resbuf));
    ((((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_this))->__CBAS__member__len)) = ((unsigned long)((((long)(__cbas_local_variable_mangled_name_where)) + ((long)((((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_with))->__CBAS__member__len)))) + (((long)((((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_this))->__CBAS__member__len))) - (((long)(__cbas_local_variable_mangled_name_where)) + ((long)((((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_what))->__CBAS__member__len)))))));
    
    return (long)(__cbas_local_variable_mangled_name_where);
    }

    
}
static inline long __CBAS____method_str_____replace_first(struct __CBAS__type__str* __cbas_local_variable_mangled_name_this,struct __CBAS__type__str* __cbas_local_variable_mangled_name_what,struct __CBAS__type__str* __cbas_local_variable_mangled_name_with)
{
    
    return __CBAS____method_str_____replace_first_from(((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_this)),((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_what)),((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_with)),(0ul));
    
}
static inline long __CBAS____method_pstr_____replace_first(struct __CBAS__type__pstr* __cbas_local_variable_mangled_name_this,struct __CBAS__type__pstr* __cbas_local_variable_mangled_name_what,struct __CBAS__type__pstr* __cbas_local_variable_mangled_name_with)
{
    
    return __CBAS____method_pstr_____replace_first_from(((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_this)),((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_what)),((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_with)),(0ul));
    
}
static inline long __CBAS____method_str_____replace(struct __CBAS__type__str* __cbas_local_variable_mangled_name_this,struct __CBAS__type__str* __cbas_local_variable_mangled_name_what,struct __CBAS__type__str* __cbas_local_variable_mangled_name_with)
{
    long __cbas_local_variable_mangled_name_w;
    if((long)(__CBAS____method_str_____empty(((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_what))))){

    
    return -((long)(1ul));
    }

    if((long)(__CBAS____method_str_____empty(((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_this))))){

    
    return (long)(0ul);
    }

    (__cbas_local_variable_mangled_name_w) = ((long)(0ul));
    while((long)(1ul)){

    long __cbas_local_variable_mangled_name_w_new;
    (__cbas_local_variable_mangled_name_w_new) = (__CBAS____method_str_____replace_first_from(((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_this)),((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_what)),((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_with)),((unsigned long)(__cbas_local_variable_mangled_name_w))));
    if(((long)(__cbas_local_variable_mangled_name_w_new)) == (-((long)(1ul)))){

    
    return (long)(0ul);
    }

    if(((long)(__cbas_local_variable_mangled_name_w_new)) == (-((long)(2ul)))){

    
    return (long)(0ul);
    }

    if(((long)(__cbas_local_variable_mangled_name_w_new)) == ((long)(__cbas_local_variable_mangled_name_w))){

    
    return (long)(0ul);
    }

    (__cbas_local_variable_mangled_name_w) = (((long)(__cbas_local_variable_mangled_name_w_new)) + ((long)((((struct __CBAS__type__str*)(__cbas_local_variable_mangled_name_with))->__CBAS__member__len))));
    }

    
}
static inline long __CBAS____method_pstr_____replace(struct __CBAS__type__pstr* __cbas_local_variable_mangled_name_this,struct __CBAS__type__pstr* __cbas_local_variable_mangled_name_what,struct __CBAS__type__pstr* __cbas_local_variable_mangled_name_with)
{
    long __cbas_local_variable_mangled_name_w;
    if((long)(__CBAS____method_pstr_____empty(((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_what))))){

    
    return -((long)(1ul));
    }

    if((long)(__CBAS____method_pstr_____empty(((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_this))))){

    
    return (long)(0ul);
    }

    (__cbas_local_variable_mangled_name_w) = ((long)(0ul));
    while((long)(1ul)){

    long __cbas_local_variable_mangled_name_w_new;
    (__cbas_local_variable_mangled_name_w_new) = (__CBAS____method_pstr_____replace_first_from(((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_this)),((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_what)),((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_with)),((unsigned long)(__cbas_local_variable_mangled_name_w))));
    if(((long)(__cbas_local_variable_mangled_name_w_new)) == (-((long)(1ul)))){

    
    return (long)(0ul);
    }

    if(((long)(__cbas_local_variable_mangled_name_w_new)) == (-((long)(2ul)))){

    
    return (long)(0ul);
    }

    if(((long)(__cbas_local_variable_mangled_name_w_new)) == ((long)(__cbas_local_variable_mangled_name_w))){

    
    return (long)(0ul);
    }

    (__cbas_local_variable_mangled_name_w) = (((long)(__cbas_local_variable_mangled_name_w_new)) + ((long)((((struct __CBAS__type__pstr*)(__cbas_local_variable_mangled_name_with))->__CBAS__member__len))));
    }

    
}
static inline void __CBAS__utoa(unsigned char* __cbas_local_variable_mangled_name_dest,unsigned long __cbas_local_variable_mangled_name_value)
{
    if(((unsigned long)(__cbas_local_variable_mangled_name_value)) == (0ul)){

    (((unsigned char*)(__cbas_local_variable_mangled_name_dest))[(long)(0ul)]) = ((unsigned char)(48ul));
    (((unsigned char*)(__cbas_local_variable_mangled_name_dest))[(long)(1ul)]) = ((unsigned char)(0ul));
    
    return;
    }

    if((long)(1ul)){

    unsigned long __cbas_local_variable_mangled_name_pow;
    (__cbas_local_variable_mangled_name_pow) = (1ul);
    while((((unsigned long)(__cbas_local_variable_mangled_name_value)) / ((unsigned long)(__cbas_local_variable_mangled_name_pow))) >= (10ul)){

    (__cbas_local_variable_mangled_name_pow) = (((unsigned long)(__cbas_local_variable_mangled_name_pow)) * (10ul));
    }

    while((long)(__cbas_local_variable_mangled_name_pow)){

    unsigned long __cbas_local_variable_mangled_name_temp;
    (__cbas_local_variable_mangled_name_temp) = (((unsigned long)(__cbas_local_variable_mangled_name_value)) / ((unsigned long)(__cbas_local_variable_mangled_name_pow)));
    (((unsigned char*)(__cbas_local_variable_mangled_name_dest))[(long)(0ul)]) = ((unsigned char)(((unsigned long)(__cbas_local_variable_mangled_name_temp)) + (48ul)));
    (__cbas_local_variable_mangled_name_dest)++;
    (__cbas_local_variable_mangled_name_value) = (((unsigned long)(__cbas_local_variable_mangled_name_value)) - (((unsigned long)(__cbas_local_variable_mangled_name_temp)) * ((unsigned long)(__cbas_local_variable_mangled_name_pow))));
    (__cbas_local_variable_mangled_name_pow) = (((unsigned long)(__cbas_local_variable_mangled_name_pow)) / (10ul));
    }

    }

    __cbas_mangled_label__ending:1;/*Statement with no effect generated for label...*/
    (((unsigned char*)(__cbas_local_variable_mangled_name_dest))[(long)(0ul)]) = ((unsigned char)(0ul));
    
    return;
    
}
static inline void __CBAS__itoa(unsigned char* __cbas_local_variable_mangled_name_dest,long __cbas_local_variable_mangled_name_value)
{
    if(((long)(__cbas_local_variable_mangled_name_value)) >= ((long)(0ul))){

    __CBAS__utoa(((unsigned char*)(__cbas_local_variable_mangled_name_dest)),((unsigned long)(__cbas_local_variable_mangled_name_value)));
    }

    else {
    (((unsigned char*)(__cbas_local_variable_mangled_name_dest))[(long)(0ul)]) = ((unsigned char)(45ul));
    __CBAS__utoa((((unsigned char*)(__cbas_local_variable_mangled_name_dest)) + ((long)(1ul))),((unsigned long)(-((long)(__cbas_local_variable_mangled_name_value)))));
    }

    
}
static inline void __CBAS__ftoa_n(unsigned char* __cbas_local_variable_mangled_name_dest,double __cbas_local_variable_mangled_name_v,long __cbas_local_variable_mangled_name_ndigits)
{
    unsigned long __cbas_local_variable_mangled_name_is_neg;
    unsigned long __cbas_local_variable_mangled_name_dg;
    long __cbas_local_variable_mangled_name_pow10;
    unsigned long __cbas_local_variable_mangled_name_dgi;
    double __cbas_local_variable_mangled_name_ftoa_errmargin;
    (__cbas_local_variable_mangled_name_is_neg) = (0ul);
    (__cbas_local_variable_mangled_name_dg) = (0ul);
    (__cbas_local_variable_mangled_name_pow10) = ((long)(0ul));
    (__cbas_local_variable_mangled_name_dgi) = (0ul);
    (__cbas_local_variable_mangled_name_ndigits)++;
    if(((long)(__cbas_local_variable_mangled_name_ndigits)) <= ((long)(0ul))){

    (((unsigned char*)(__cbas_local_variable_mangled_name_dest))[(long)(0ul)]) = ((unsigned char)(69ul));
    (((unsigned char*)(__cbas_local_variable_mangled_name_dest))[(long)(1ul)]) = ((unsigned char)(82ul));
    (((unsigned char*)(__cbas_local_variable_mangled_name_dest))[(long)(2ul)]) = ((unsigned char)(82ul));
    (((unsigned char*)(__cbas_local_variable_mangled_name_dest))[(long)(3ul)]) = ((unsigned char)(0ul));
    }

    if(__CBAS__signbit(((double)(__cbas_local_variable_mangled_name_v)))){

    (__cbas_local_variable_mangled_name_is_neg) = (1ul);
    (((unsigned char*)(__cbas_local_variable_mangled_name_dest))[(long)(0ul)]) = ((unsigned char)(45ul));
    (__cbas_local_variable_mangled_name_dest)++;
    (__cbas_local_variable_mangled_name_v) = (-((double)(__cbas_local_variable_mangled_name_v)));
    }

    if(__CBAS__isnan(((double)(__cbas_local_variable_mangled_name_v)))){

    (((unsigned char*)(__cbas_local_variable_mangled_name_dest))[(long)(0ul)]) = ((unsigned char)(78ul));
    (((unsigned char*)(__cbas_local_variable_mangled_name_dest))[(long)(1ul)]) = ((unsigned char)(65ul));
    (((unsigned char*)(__cbas_local_variable_mangled_name_dest))[(long)(2ul)]) = ((unsigned char)(78ul));
    (((unsigned char*)(__cbas_local_variable_mangled_name_dest))[(long)(3ul)]) = ((unsigned char)(0ul));
    
    return;
    }

    if((long)(!(__CBAS__isfinite(((double)(__cbas_local_variable_mangled_name_v)))))){

    (((unsigned char*)(__cbas_local_variable_mangled_name_dest))[(long)(0ul)]) = ((unsigned char)(73ul));
    (((unsigned char*)(__cbas_local_variable_mangled_name_dest))[(long)(1ul)]) = ((unsigned char)(78ul));
    (((unsigned char*)(__cbas_local_variable_mangled_name_dest))[(long)(2ul)]) = ((unsigned char)(70ul));
    (((unsigned char*)(__cbas_local_variable_mangled_name_dest))[(long)(3ul)]) = ((unsigned char)(0ul));
    
    return;
    }

    if(((double)(__cbas_local_variable_mangled_name_v)) == ((double)(0ul))){

    (((unsigned char*)(__cbas_local_variable_mangled_name_dest))[(long)(0ul)]) = ((unsigned char)(48ul));
    (((unsigned char*)(__cbas_local_variable_mangled_name_dest))[(long)(1ul)]) = ((unsigned char)(0ul));
    
    return;
    }

    (__cbas_local_variable_mangled_name_pow10) = ((long)(__CBAS__log10(((double)(__cbas_local_variable_mangled_name_v)))));
    if(((long)(__cbas_local_variable_mangled_name_pow10)) >= (((long)(__cbas_local_variable_mangled_name_ndigits)) + ((long)(1ul)))){

    goto __cbas_mangled_label__science;
    }

    else if(((long)(__cbas_local_variable_mangled_name_pow10)) <= (-(((long)(__cbas_local_variable_mangled_name_ndigits)) + ((long)(1ul))))){

    goto __cbas_mangled_label__science;
    }

    else {
    while(((long)(__cbas_local_variable_mangled_name_pow10)) >= ((long)(0ul))){

    long __cbas_local_variable_mangled_name_temp;
    (__cbas_local_variable_mangled_name_temp) = ((long)(((double)(__cbas_local_variable_mangled_name_v)) / (__CBAS__pow(((double)(10ul)),((double)(__cbas_local_variable_mangled_name_pow10))))));
    (((unsigned char*)(__cbas_local_variable_mangled_name_dest))[(long)(0ul)]) = ((unsigned char)(((long)(__cbas_local_variable_mangled_name_temp)) + ((long)(48ul))));
    (__cbas_local_variable_mangled_name_dest)++;
    (__cbas_local_variable_mangled_name_ndigits)--;
    (__cbas_local_variable_mangled_name_v) = (((double)(__cbas_local_variable_mangled_name_v)) - (((double)(__cbas_local_variable_mangled_name_temp)) * (__CBAS__pow(((double)(10ul)),((double)(__cbas_local_variable_mangled_name_pow10))))));
    (__cbas_local_variable_mangled_name_pow10)--;
    }

    if((long)(((unsigned long)(((double)(__cbas_local_variable_mangled_name_v)) == ((double)(0ul)))) || ((unsigned long)(((long)(__cbas_local_variable_mangled_name_ndigits)) == ((long)(0ul)))))){

    (((unsigned char*)(__cbas_local_variable_mangled_name_dest))[(long)(0ul)]) = ((unsigned char)(46ul));
    (__cbas_local_variable_mangled_name_dest)++;
    (((unsigned char*)(__cbas_local_variable_mangled_name_dest))[(long)(0ul)]) = ((unsigned char)(48ul));
    (__cbas_local_variable_mangled_name_dest)++;
    (((unsigned char*)(__cbas_local_variable_mangled_name_dest))[(long)(0ul)]) = ((unsigned char)(0ul));
    
    return;
    }

    (((unsigned char*)(__cbas_local_variable_mangled_name_dest))[(long)(0ul)]) = ((unsigned char)(46ul));
    (__cbas_local_variable_mangled_name_dest)++;
    while(((long)(__cbas_local_variable_mangled_name_ndigits)) > ((long)(0ul))){

    long __cbas_local_variable_mangled_name_temp;
    (__cbas_local_variable_mangled_name_v) = (((double)(__cbas_local_variable_mangled_name_v)) * ((double)(10ul)));
    (__cbas_local_variable_mangled_name_temp) = ((long)(__cbas_local_variable_mangled_name_v));
    (((unsigned char*)(__cbas_local_variable_mangled_name_dest))[(long)(0ul)]) = ((unsigned char)(((long)(__cbas_local_variable_mangled_name_temp)) + ((long)(48ul))));
    (__cbas_local_variable_mangled_name_dest)++;
    (__cbas_local_variable_mangled_name_v) = (((double)(__cbas_local_variable_mangled_name_v)) - ((double)(__cbas_local_variable_mangled_name_temp)));
    (__cbas_local_variable_mangled_name_ndigits)--;
    }

    (((unsigned char*)(__cbas_local_variable_mangled_name_dest))[(long)(0ul)]) = ((unsigned char)(0ul));
    
    return;
    }

    
    return;
    __cbas_mangled_label__science:1;/*Statement with no effect generated for label...*/
    if((long)(1ul)){

    long __cbas_local_variable_mangled_name_dgc;
    (__cbas_local_variable_mangled_name_dgc) = ((long)(0ul));
    (__cbas_local_variable_mangled_name_v) = (((double)(__cbas_local_variable_mangled_name_v)) / (__CBAS__pow(((double)(10ul)),((double)(__cbas_local_variable_mangled_name_pow10)))));
    while(((long)(__cbas_local_variable_mangled_name_ndigits)) > ((long)(0ul))){

    long __cbas_local_variable_mangled_name_temp;
    (__cbas_local_variable_mangled_name_temp) = ((long)(__cbas_local_variable_mangled_name_v));
    (((unsigned char*)(__cbas_local_variable_mangled_name_dest))[(long)(0ul)]) = ((unsigned char)(((long)(__cbas_local_variable_mangled_name_temp)) + ((long)(48ul))));
    (__cbas_local_variable_mangled_name_dest)++;
    (__cbas_local_variable_mangled_name_v) = (((double)(__cbas_local_variable_mangled_name_v)) - ((double)(__cbas_local_variable_mangled_name_temp)));
    (__cbas_local_variable_mangled_name_ndigits)--;
    if((long)(((unsigned long)(((long)(__cbas_local_variable_mangled_name_dgc)) == ((long)(0ul)))) && ((unsigned long)(((long)(__cbas_local_variable_mangled_name_ndigits)) != ((long)(0ul)))))){

    (((unsigned char*)(__cbas_local_variable_mangled_name_dest))[(long)(0ul)]) = ((unsigned char)(46ul));
    (__cbas_local_variable_mangled_name_dest)++;
    }

    (__cbas_local_variable_mangled_name_dgc)++;
    (__cbas_local_variable_mangled_name_v) = (((double)(__cbas_local_variable_mangled_name_v)) * ((double)(10ul)));
    }

    (((unsigned char*)(__cbas_local_variable_mangled_name_dest))[(long)(0ul)]) = ((unsigned char)(69ul));
    (__cbas_local_variable_mangled_name_dest)++;
    __CBAS__itoa(((unsigned char*)(__cbas_local_variable_mangled_name_dest)),((long)(__cbas_local_variable_mangled_name_pow10)));
    
    return;
    }

    
}
static inline void __CBAS__ftoa(unsigned char* __cbas_local_variable_mangled_name_dest,double __cbas_local_variable_mangled_name_v)
{
    __CBAS__ftoa_n(((unsigned char*)(__cbas_local_variable_mangled_name_dest)),((double)(__cbas_local_variable_mangled_name_v)),((long)(54ul)));
    
}
static void __CBAS____method_bbb2_____ctor(struct __CBAS__type__bbb2* __cbas_local_variable_mangled_name_this)
{
    (((&(((struct __CBAS__type__bbb2*)(__cbas_local_variable_mangled_name_this))->__CBAS__member__mymember))->__CBAS__member__a)) = ((long)(3ul));
    (((&(((struct __CBAS__type__bbb2*)(__cbas_local_variable_mangled_name_this))->__CBAS__member__mymember))->__CBAS__member__b)) = ((float)(3ul));
    
}
static void __CBAS____method_membuf_____ctor(struct __CBAS__type__membuf* __cbas_local_variable_mangled_name_this)
{
    ((((struct __CBAS__type__membuf*)(__cbas_local_variable_mangled_name_this))->__CBAS__member__d)) = ((unsigned char*)(0ul));
    
}
static void __CBAS____method_membuf_____dtor(struct __CBAS__type__membuf* __cbas_local_variable_mangled_name_this)
{
    if(((unsigned char*)((((struct __CBAS__type__membuf*)(__cbas_local_variable_mangled_name_this))->__CBAS__member__d))) != ((unsigned char*)(0ul))){

    __CBAS__free(((unsigned char*)((((struct __CBAS__type__membuf*)(__cbas_local_variable_mangled_name_this))->__CBAS__member__d))));
    }

    
}
static void __CBAS____method_membuf_____req(struct __CBAS__type__membuf* __cbas_local_variable_mangled_name_this,unsigned long __cbas_local_variable_mangled_name_sz)
{
    ((((struct __CBAS__type__membuf*)(__cbas_local_variable_mangled_name_this))->__CBAS__member__d)) = (__CBAS__malloc(((unsigned long)(__cbas_local_variable_mangled_name_sz))));
    if(((unsigned char*)((((struct __CBAS__type__membuf*)(__cbas_local_variable_mangled_name_this))->__CBAS__member__d))) == ((unsigned char*)(0ul))){

    __CBAS__println((((unsigned char*)"malloc failed!")));
    __CBAS__sys_exit(((long)(1ul)));
    }

    
}
static inline long __CBAS____method_bbb_____do_stuff(union __CBAS__type__bbb* __cbas_local_variable_mangled_name_this,long __cbas_local_variable_mangled_name_incr)
{
    float* __cbas_local_variable_mangled_name_b_ptr;
    long* __cbas_local_variable_mangled_name_a_ptr;
    ((((union __CBAS__type__bbb*)(__cbas_local_variable_mangled_name_this))->__CBAS__member__a)) = (((long)((((union __CBAS__type__bbb*)(__cbas_local_variable_mangled_name_this))->__CBAS__member__a))) + ((long)(__cbas_local_variable_mangled_name_incr)));
    (__cbas_local_variable_mangled_name_a_ptr) = (&(((union __CBAS__type__bbb*)(__cbas_local_variable_mangled_name_this))->__CBAS__member__a));
    (__cbas_local_variable_mangled_name_b_ptr) = (&(((union __CBAS__type__bbb*)(__cbas_local_variable_mangled_name_this))->__CBAS__member__b));
    if(((long*)(__cbas_local_variable_mangled_name_a_ptr)) != ((long*)(__cbas_local_variable_mangled_name_b_ptr))){

    

    }

    else {
    
    return (long)((((union __CBAS__type__bbb*)(__cbas_local_variable_mangled_name_this))->__CBAS__member__a));
    }

    
    return ((long)((((union __CBAS__type__bbb*)(__cbas_local_variable_mangled_name_this))->__CBAS__member__a))) - ((long)(1ul));
    
}
static long __CBAS__myFunction(long __cbas_local_variable_mangled_name_arg1,long __cbas_local_variable_mangled_name_arg2,long __cbas_local_variable_mangled_name_arg3)
{
    
    return (((long)(__cbas_local_variable_mangled_name_arg1)) + ((long)(__cbas_local_variable_mangled_name_arg2))) + ((long)(__cbas_local_variable_mangled_name_arg3));
    
}
static inline long __CBAS____method_bbb_____do_stuff2(union __CBAS__type__bbb* __cbas_local_variable_mangled_name_this,long __cbas_local_variable_mangled_name_incr,long __cbas_local_variable_mangled_name_incr2,long __cbas_local_variable_mangled_name_incr3)
{
    float* __cbas_local_variable_mangled_name_b_ptr;
    long* __cbas_local_variable_mangled_name_a_ptr;
    ((((union __CBAS__type__bbb*)(__cbas_local_variable_mangled_name_this))->__CBAS__member__a)) = (((((long)((((union __CBAS__type__bbb*)(__cbas_local_variable_mangled_name_this))->__CBAS__member__a))) + ((long)(__cbas_local_variable_mangled_name_incr))) + ((long)(__cbas_local_variable_mangled_name_incr2))) + ((long)(__cbas_local_variable_mangled_name_incr3)));
    (__cbas_local_variable_mangled_name_a_ptr) = (&(((union __CBAS__type__bbb*)(__cbas_local_variable_mangled_name_this))->__CBAS__member__a));
    (__cbas_local_variable_mangled_name_b_ptr) = (&(((union __CBAS__type__bbb*)(__cbas_local_variable_mangled_name_this))->__CBAS__member__b));
    if(((long*)(__cbas_local_variable_mangled_name_a_ptr)) != ((long*)(__cbas_local_variable_mangled_name_b_ptr))){

    

    }

    else {
    
    return (long)((((union __CBAS__type__bbb*)(__cbas_local_variable_mangled_name_this))->__CBAS__member__a));
    }

    
    return ((long)((((union __CBAS__type__bbb*)(__cbas_local_variable_mangled_name_this))->__CBAS__member__a))) - ((long)(1ul));
    
}
static void __CBAS____method_bbb_____init(union __CBAS__type__bbb* __cbas_local_variable_mangled_name_this)
{
    ((((union __CBAS__type__bbb*)(__cbas_local_variable_mangled_name_this))->__CBAS__member__a)) = ((long)(0ul));
    
}
static inline void __CBAS__mutoa(unsigned char* __cbas_local_variable_mangled_name_dest,unsigned long __cbas_local_variable_mangled_name_value)
{
    float* __cbas_local_variable_mangled_name_ppp;
    if(((unsigned long)(__cbas_local_variable_mangled_name_value)) == (0ul)){

    (((unsigned char*)(__cbas_local_variable_mangled_name_dest))[(long)(0ul)]) = ((unsigned char)(48ul));
    (__cbas_local_variable_mangled_name_dest)++;
    goto __cbas_mangled_label__ending;
    }

    if((long)(1ul)){

    unsigned long __cbas_local_variable_mangled_name_pow;
    (__cbas_local_variable_mangled_name_pow) = (1ul);
    while((((unsigned long)(__cbas_local_variable_mangled_name_value)) / ((unsigned long)(__cbas_local_variable_mangled_name_pow))) > (9ul)){

    (__cbas_local_variable_mangled_name_pow) = (((unsigned long)(__cbas_local_variable_mangled_name_pow)) * (10ul));
    }

    while((long)(__cbas_local_variable_mangled_name_pow)){

    unsigned long __cbas_local_variable_mangled_name_temp;
    (__cbas_local_variable_mangled_name_temp) = (((unsigned long)(__cbas_local_variable_mangled_name_value)) / ((unsigned long)(__cbas_local_variable_mangled_name_pow)));
    (((unsigned char*)(__cbas_local_variable_mangled_name_dest))[(long)(0ul)]) = ((unsigned char)(((unsigned long)(__cbas_local_variable_mangled_name_temp)) + (48ul)));
    (__cbas_local_variable_mangled_name_dest)++;
    (__cbas_local_variable_mangled_name_value) = (((unsigned long)(__cbas_local_variable_mangled_name_value)) - (((unsigned long)(__cbas_local_variable_mangled_name_temp)) * ((unsigned long)(__cbas_local_variable_mangled_name_pow))));
    (__cbas_local_variable_mangled_name_pow) = (((unsigned long)(__cbas_local_variable_mangled_name_pow)) / (10ul));
    }

    }

    __cbas_mangled_label__ending:1;/*Statement with no effect generated for label...*/
    (((unsigned char*)(__cbas_local_variable_mangled_name_dest))[(long)(0ul)]) = ((unsigned char)(0ul));
    
    return;
    (__cbas_local_variable_mangled_name_ppp) = (__CBAS__myfloats);
    
}
static inline void __CBAS____method_this_type_has_a_destructor_____dtor(struct __CBAS__type__this_type_has_a_destructor* __cbas_local_variable_mangled_name_this)
{
    if((long)(1ul)){

    unsigned char __cbas_local_variable_mangled_name___pprint_local_buf[250l];
    __CBAS__println((((unsigned char*)"Goodbye mom!")));
    __CBAS__println((((unsigned char*)"My a was:")));
    __CBAS__mutoa((__cbas_local_variable_mangled_name___pprint_local_buf),((unsigned long)((((struct __CBAS__type__this_type_has_a_destructor*)(__cbas_local_variable_mangled_name_this))->__CBAS__member__a))));
    __CBAS__println((__cbas_local_variable_mangled_name___pprint_local_buf));
    }

    if(((unsigned char*)((((struct __CBAS__type__this_type_has_a_destructor*)(__cbas_local_variable_mangled_name_this))->__CBAS__member__qq))) != ((unsigned char*)(0ul))){

    __CBAS__free(((unsigned char*)((((struct __CBAS__type__this_type_has_a_destructor*)(__cbas_local_variable_mangled_name_this))->__CBAS__member__qq))));
    }

    
}
static inline void __CBAS____method_this_type_has_a_destructor_____ctor(struct __CBAS__type__this_type_has_a_destructor* __cbas_local_variable_mangled_name_this)
{
    __CBAS__println((((unsigned char*)"Hello Mom!")));
    ((((struct __CBAS__type__this_type_has_a_destructor*)(__cbas_local_variable_mangled_name_this))->__CBAS__member__a)) = ((long)(5ul));
    ((((struct __CBAS__type__this_type_has_a_destructor*)(__cbas_local_variable_mangled_name_this))->__CBAS__member__b)) = ((long)(7ul));
    ((((struct __CBAS__type__this_type_has_a_destructor*)(__cbas_local_variable_mangled_name_this))->__CBAS__member__qq)) = (__CBAS__malloc((20ul)));
    if(((unsigned char*)((((struct __CBAS__type__this_type_has_a_destructor*)(__cbas_local_variable_mangled_name_this))->__CBAS__member__qq))) == ((unsigned char*)(0ul))){

    __CBAS__println((((unsigned char*)"malloc failed!")));
    }

    
}
static void __CBAS__whosa_yousa(long __cbas_local_variable_mangled_name_a)
{
    if((long)(1ul)){

    unsigned char __cbas_local_variable_mangled_name___pprint_local_buf[250l];
    __CBAS__println((((unsigned char*)"Whosa programmer? Yousa programmer!")));
    __CBAS__println((((unsigned char*)"Here's that integer...")));
    __CBAS__mutoa((__cbas_local_variable_mangled_name___pprint_local_buf),((unsigned long)(__cbas_local_variable_mangled_name_a)));
    __CBAS__println((__cbas_local_variable_mangled_name___pprint_local_buf));
    }

    
}
static void __CBAS__wadda(long __cbas_local_variable_mangled_name_iters)
{
    unsigned char __cbas_local_variable_mangled_name_buf[50l];
    long __cbas_local_variable_mangled_name_i;
    long __cbas_local_variable_mangled_name_j;
    for((__cbas_local_variable_mangled_name_i) = ((long)(0ul));((long)(__cbas_local_variable_mangled_name_i)) <= ((long)(__cbas_local_variable_mangled_name_iters));(__cbas_local_variable_mangled_name_i)++){

    __CBAS__mutoa((__cbas_local_variable_mangled_name_buf),((unsigned long)(((long)(__cbas_local_variable_mangled_name_i)) * ((long)(__cbas_local_variable_mangled_name_i)))));
    for((__cbas_local_variable_mangled_name_j) = ((long)(0ul));((long)(__cbas_local_variable_mangled_name_j)) < ((long)(12ul));(__cbas_local_variable_mangled_name_j)++){

    __CBAS__println((__cbas_local_variable_mangled_name_buf));
    if(((long)(__cbas_local_variable_mangled_name_j)) == ((long)(3ul))){

    ((__cbas_local_variable_mangled_name_buf)[(long)(0ul)]) = ((unsigned char)(113ul));
    __CBAS__println((__cbas_local_variable_mangled_name_buf));
    break;
    }

    }

    if(((long)(__cbas_local_variable_mangled_name_i)) == ((long)(13ul))){

    break;
    }

    continue;
    }

    
    __CBAS__whosa_yousa((__cbas_local_variable_mangled_name_iters));return;
    
}
void __CBAS__inf_loop()
{
    
    __CBAS__inf_loop();return;
    
}
long __CBAS__inf_loop2()
{
    
    return __CBAS__inf_loop2();
    
}
static long __CBAS__inf_loop3(long __cbas_local_variable_mangled_name_a)
{
    (__cbas_local_variable_mangled_name_a) = (((long)(__cbas_local_variable_mangled_name_a)) + ((long)(1ul)));
    if((((long)(__cbas_local_variable_mangled_name_a)) % ((long)(5ul))) != ((long)(0ul))){

    
    return __CBAS__inf_loop3((__cbas_local_variable_mangled_name_a));
    }

    
    return (long)(__cbas_local_variable_mangled_name_a);
    
}
long main()
{
    unsigned char* __cbas_local_variable_mangled_name_p;
    union __CBAS__type__bbb __cbas_local_variable_mangled_name_my_bbb;
    union __CBAS__type__bbb __cbas_local_variable_mangled_name_my_bbb_2;
    long __cbas_local_variable_mangled_name_i;
    __CBAS__println((((unsigned char*)"Hello World, from CBAS!")));
    (__cbas_local_variable_mangled_name_p) = ((unsigned char*)&__CBAS__wadda);
    ((void(*)(long))((unsigned char*)(__cbas_local_variable_mangled_name_p)))(((long)(30ul)));
    if((long)(0ul)){

    __CBAS__println((((unsigned char*)"a")));
    }

    else if(__CBAS__impl_streq((((unsigned char*)"eq")),(((unsigned char*)"neq")))){

    __CBAS__println((((unsigned char*)"b")));
    }

    else {
    __CBAS__println((((unsigned char*)"Quality!")));
    }

    if((long)(1ul)){

    const void* const __cbas_dispatchtable_switch__0_____[4ul] = {
    &&__cbas_mangled_label__aa, &&__cbas_mangled_label__bb, &&__cbas_mangled_label__cc, &&__cbas_mangled_label__dd
    };
    goto *__cbas_dispatchtable_switch__0_____[(long)(2ul)];;
    goto *__cbas_dispatchtable_switch__0_____[(long)(3ul)];;
    goto *__cbas_dispatchtable_switch__0_____[(long)(3ul)];;
    goto *__cbas_dispatchtable_switch__0_____[(long)(3ul)];;
    goto *__cbas_dispatchtable_switch__0_____[(long)(3ul)];;
    __cbas_mangled_label__aa:1;/*Statement with no effect generated for label...*/
    __CBAS__println((((unsigned char*)"Got a 0!")));
    goto __cbas_mangled_label__ee;
    __cbas_mangled_label__bb:1;/*Statement with no effect generated for label...*/
    __CBAS__println((((unsigned char*)"Got a 1!")));
    goto __cbas_mangled_label__ee;
    __cbas_mangled_label__cc:1;/*Statement with no effect generated for label...*/
    __CBAS__println((((unsigned char*)"Got a 2!")));
    goto __cbas_mangled_label__ee;
    __cbas_mangled_label__dd:1;/*Statement with no effect generated for label...*/
    __CBAS__println((((unsigned char*)"Got a 3!")));
    goto __cbas_mangled_label__ee;
    __cbas_mangled_label__ee:1;/*Statement with no effect generated for label...*/
    if((long)(1ul)){

    unsigned char __cbas_local_variable_mangled_name___pprint_local_buf[250l];
    __CBAS__println((((unsigned char*)"Goodbye!")));
    __CBAS__println((((unsigned char*)"This should be 8:")));
    __CBAS__mutoa((__cbas_local_variable_mangled_name___pprint_local_buf),((1ul) << (3ul)));
    __CBAS__println((__cbas_local_variable_mangled_name___pprint_local_buf));
    }

    }

    for((__CBAS__my_integer) = ((long)(0ul));((long)(__CBAS__my_integer)) < ((long)(500ul));(__CBAS__my_integer)++){

    if((((long)(__CBAS__my_integer)) % ((long)(10ul))) == ((long)(0ul))){

    if((long)(1ul)){

    unsigned char __cbas_local_variable_mangled_name___pprint_local_buf[250l];
    __CBAS__println((((unsigned char*)"Printing atomic variable my_integer")));
    __CBAS__mutoa((__cbas_local_variable_mangled_name___pprint_local_buf),((unsigned long)(__CBAS__my_integer)));
    __CBAS__println((__cbas_local_variable_mangled_name___pprint_local_buf));
    }

    }

    }

    __CBAS____method_bbb_____init(((&__cbas_local_variable_mangled_name_my_bbb)));
    for((__cbas_local_variable_mangled_name_i) = ((long)(0ul));((long)(__cbas_local_variable_mangled_name_i)) < ((long)(10ul));(__cbas_local_variable_mangled_name_i)++){

    if((long)(1ul)){

    unsigned char __cbas_local_variable_mangled_name___pprint_local_buf[250l];
    __CBAS__println((((unsigned char*)"Another iteration?")));
    __CBAS__mutoa((__cbas_local_variable_mangled_name___pprint_local_buf),((unsigned long)(__CBAS____method_bbb_____do_stuff2(((&__cbas_local_variable_mangled_name_my_bbb)),((long)(12ul)),((long)(12ul)),((long)(12ul))))));
    __CBAS__println((__cbas_local_variable_mangled_name___pprint_local_buf));
    }

    }

    ((((&__cbas_local_variable_mangled_name_my_bbb_2))->__CBAS__member__a)) = ((long)(0ul));
    ((((&__cbas_local_variable_mangled_name_my_bbb_2))->__CBAS__member__b)) = ((float)(0ul));
    (*((&__cbas_local_variable_mangled_name_my_bbb_2)) = *((&__cbas_local_variable_mangled_name_my_bbb)));
    if((long)(1ul)){

    unsigned char __cbas_local_variable_mangled_name___pprint_local_buf[250l];
    __CBAS__println((((unsigned char*)"my_bbb_2.a = ")));
    __CBAS__mutoa((__cbas_local_variable_mangled_name___pprint_local_buf),((unsigned long)((((&__cbas_local_variable_mangled_name_my_bbb_2))->__CBAS__member__a))));
    __CBAS__println((__cbas_local_variable_mangled_name___pprint_local_buf));
    __CBAS__println((((unsigned char*)"my_bbb.a = ")));
    __CBAS__mutoa((__cbas_local_variable_mangled_name___pprint_local_buf),((unsigned long)((((&__cbas_local_variable_mangled_name_my_bbb))->__CBAS__member__a))));
    __CBAS__println((__cbas_local_variable_mangled_name___pprint_local_buf));
    }

    if((long)(1ul)){

    unsigned char __cbas_local_variable_mangled_name___pprint_local_buf[250l];
    __CBAS__println((((unsigned char*)"This is what I get for inf_loop3:")));
    __CBAS__mutoa((__cbas_local_variable_mangled_name___pprint_local_buf),((unsigned long)(__CBAS__inf_loop3(((long)(23ul))))));
    __CBAS__println((__cbas_local_variable_mangled_name___pprint_local_buf));
    __CBAS__println((((unsigned char*)"As you can see, it wasn't really an infinite loop after all!")));
    __CBAS__println((((unsigned char*)"This is intentional...")));
    }

    ((((__CBAS__my_bbbs) + ((long)(23ul)))->__CBAS__member__a)) = ((long)(17ul));
    ((((__CBAS__my_bbbs) + ((long)(24ul)))->__CBAS__member__a)) = ((long)(97.2999999999999971578290569595992565155029296875000000000000000000));
    __CBAS____method_bbb_____do_stuff(((__CBAS__my_bbbs) + ((long)(23ul))),((long)(1ul)));
    if((long)(1ul)){

    unsigned char __cbas_local_variable_mangled_name___pprint_local_buf[250l];
    __CBAS__println((((unsigned char*)"Here's what happened to my_bbbs+23:")));
    __CBAS__mutoa((__cbas_local_variable_mangled_name___pprint_local_buf),((unsigned long)((((__CBAS__my_bbbs) + ((long)(23ul)))->__CBAS__member__a))));
    __CBAS__println((__cbas_local_variable_mangled_name___pprint_local_buf));
    __CBAS__println((((unsigned char*)"And +24:")));
    __CBAS__mutoa((__cbas_local_variable_mangled_name___pprint_local_buf),((unsigned long)((((__CBAS__my_bbbs) + ((long)(24ul)))->__CBAS__member__a))));
    __CBAS__println((__cbas_local_variable_mangled_name___pprint_local_buf));
    __CBAS__println((((unsigned char*)"Furthermore, if we use getglobalptr and get the 24th element...")));
    __CBAS__mutoa((__cbas_local_variable_mangled_name___pprint_local_buf),((unsigned long)((((__CBAS__my_bbbs) + ((long)(24ul)))->__CBAS__member__a))));
    __CBAS__println((__cbas_local_variable_mangled_name___pprint_local_buf));
    __CBAS__println((((unsigned char*)"Interpreted as a float, this is...")));
    __CBAS__mutoa((__cbas_local_variable_mangled_name___pprint_local_buf),((unsigned long)((((__CBAS__my_bbbs) + ((long)(24ul)))->__CBAS__member__b))));
    __CBAS__println((__cbas_local_variable_mangled_name___pprint_local_buf));
    }

    ((((__CBAS__my_bbbs) + ((long)(24ul)))->__CBAS__member__b)) = ((float)(25.3000000000000007105427357601001858711242675781250000000000000000));
    if((long)(1ul)){

    unsigned char __cbas_local_variable_mangled_name___pprint_local_buf[250l];
    __CBAS__println((((unsigned char*)"I've assigned the float with the value 25.3. Here it is multiplied by 10 as an int:")));
    __CBAS__mutoa((__cbas_local_variable_mangled_name___pprint_local_buf),((unsigned long)(((float)((((__CBAS__my_bbbs) + ((long)(24ul)))->__CBAS__member__b))) * ((float)(10ul)))));
    __CBAS__println((__cbas_local_variable_mangled_name___pprint_local_buf));
    __CBAS__println((((unsigned char*)"And here is the integer interpretation:")));
    __CBAS__mutoa((__cbas_local_variable_mangled_name___pprint_local_buf),((unsigned long)((((__CBAS__my_bbbs) + ((long)(24ul)))->__CBAS__member__a))));
    __CBAS__println((__cbas_local_variable_mangled_name___pprint_local_buf));
    __CBAS__println((((unsigned char*)"And the size of my_bbb is...")));
    __CBAS__println((((unsigned char*)"~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~")));
    __CBAS__mutoa((__cbas_local_variable_mangled_name___pprint_local_buf),(sizeof(union __CBAS__type__bbb)));
    __CBAS__println((__cbas_local_variable_mangled_name___pprint_local_buf));
    __CBAS__println((((unsigned char*)"~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~")));
    }

    if((long)(1ul)){

    struct __CBAS__type__this_type_has_a_destructor __cbas_local_variable_mangled_name_qq;
    struct __CBAS__type__this_type_has_a_destructor __cbas_local_variable_mangled_name_qq2;
    long __cbas_local_variable_mangled_name_ffff;
    __CBAS____method_this_type_has_a_destructor_____ctor(((&__cbas_local_variable_mangled_name_qq)));
    __CBAS____method_this_type_has_a_destructor_____ctor(((&__cbas_local_variable_mangled_name_qq2)));
    for((__cbas_local_variable_mangled_name_ffff) = ((long)(0ul));((long)(__cbas_local_variable_mangled_name_ffff)) < ((long)(10ul));(__cbas_local_variable_mangled_name_ffff)++){

    struct __CBAS__type__this_type_has_a_destructor __cbas_local_variable_mangled_name_a;
    struct __CBAS__type__this_type_has_a_destructor __cbas_local_variable_mangled_name_b;
    struct __CBAS__type__this_type_has_a_destructor __cbas_local_variable_mangled_name_c;
    long __cbas_local_variable_mangled_name_quiet;
    __CBAS____method_this_type_has_a_destructor_____ctor(((&__cbas_local_variable_mangled_name_a)));
    __CBAS____method_this_type_has_a_destructor_____ctor(((&__cbas_local_variable_mangled_name_b)));
    __CBAS____method_this_type_has_a_destructor_____ctor(((&__cbas_local_variable_mangled_name_c)));
    ((((&__cbas_local_variable_mangled_name_a))->__CBAS__member__a)) = ((long)(1ul));
    ((((&__cbas_local_variable_mangled_name_b))->__CBAS__member__a)) = ((long)(2ul));
    ((((&__cbas_local_variable_mangled_name_c))->__CBAS__member__a)) = ((long)(3ul));
    for((__cbas_local_variable_mangled_name_quiet) = ((long)(0ul));((long)(__cbas_local_variable_mangled_name_quiet)) < ((long)(3ul));(__cbas_local_variable_mangled_name_quiet)++){

    if((long)(1ul)){

    unsigned char __cbas_local_variable_mangled_name___pprint_local_buf[250l];
    __CBAS__println((((unsigned char*)"Do do do...")));
    __CBAS__mutoa((__cbas_local_variable_mangled_name___pprint_local_buf),((unsigned long)(((((long)(__cbas_local_variable_mangled_name_quiet)) * ((long)(__cbas_local_variable_mangled_name_quiet))) * ((long)(__cbas_local_variable_mangled_name_quiet))) * ((long)(__cbas_local_variable_mangled_name_quiet)))));
    __CBAS__println((__cbas_local_variable_mangled_name___pprint_local_buf));
    }

    }

    if(((long)(__cbas_local_variable_mangled_name_ffff)) == ((long)(3ul))){

    __CBAS____method_this_type_has_a_destructor_____dtor(((&__cbas_local_variable_mangled_name_c)));
    __CBAS____method_this_type_has_a_destructor_____dtor(((&__cbas_local_variable_mangled_name_b)));
    __CBAS____method_this_type_has_a_destructor_____dtor(((&__cbas_local_variable_mangled_name_a)));
    break;
    }

    if(((long)(__cbas_local_variable_mangled_name_ffff)) == ((long)(9ul))){

    __CBAS____method_this_type_has_a_destructor_____dtor(((&__cbas_local_variable_mangled_name_c)));
    __CBAS____method_this_type_has_a_destructor_____dtor(((&__cbas_local_variable_mangled_name_b)));
    __CBAS____method_this_type_has_a_destructor_____dtor(((&__cbas_local_variable_mangled_name_a)));
    __CBAS____method_this_type_has_a_destructor_____dtor(((&__cbas_local_variable_mangled_name_qq2)));
    __CBAS____method_this_type_has_a_destructor_____dtor(((&__cbas_local_variable_mangled_name_qq)));
    goto __cbas_mangled_label__real_end;
    }

    if((long)(0ul)){

    __CBAS____method_this_type_has_a_destructor_____dtor(((&__cbas_local_variable_mangled_name_c)));
    __CBAS____method_this_type_has_a_destructor_____dtor(((&__cbas_local_variable_mangled_name_b)));
    __CBAS____method_this_type_has_a_destructor_____dtor(((&__cbas_local_variable_mangled_name_a)));
    __CBAS____method_this_type_has_a_destructor_____dtor(((&__cbas_local_variable_mangled_name_qq2)));
    __CBAS____method_this_type_has_a_destructor_____dtor(((&__cbas_local_variable_mangled_name_qq)));
    
    return (long)(0ul);
    __CBAS__println((((unsigned char*)"This should never print!")));
    }

    __CBAS____method_this_type_has_a_destructor_____dtor(((&__cbas_local_variable_mangled_name_c)));
    __CBAS____method_this_type_has_a_destructor_____dtor(((&__cbas_local_variable_mangled_name_b)));
    __CBAS____method_this_type_has_a_destructor_____dtor(((&__cbas_local_variable_mangled_name_a)));
    }

    __CBAS____method_this_type_has_a_destructor_____dtor(((&__cbas_local_variable_mangled_name_qq2)));
    __CBAS____method_this_type_has_a_destructor_____dtor(((&__cbas_local_variable_mangled_name_qq)));
    }

    if((long)(1ul)){

    struct __CBAS__type__membuf __cbas_local_variable_mangled_name_buffer;
    __CBAS____method_membuf_____ctor(((&__cbas_local_variable_mangled_name_buffer)));
    __CBAS____method_membuf_____req(((&__cbas_local_variable_mangled_name_buffer)),((1ul) << (10ul)));
    __CBAS__println((((unsigned char*)"Enter 10 characters")));
    __CBAS__fread((__CBAS__get_stdin_handle()),((unsigned char*)((((&__cbas_local_variable_mangled_name_buffer))->__CBAS__member__d))),(10ul));
    __CBAS__println((((unsigned char*)"Here are your characters back:")));
    __CBAS__fwrite((__CBAS__get_stdout_handle()),((unsigned char*)((((&__cbas_local_variable_mangled_name_buffer))->__CBAS__member__d))),(10ul));
    __CBAS____method_membuf_____dtor(((&__cbas_local_variable_mangled_name_buffer)));
    }

    __cbas_mangled_label__real_end:1;/*Statement with no effect generated for label...*/
    
}
/*
Blessed be the Lord Jesus Christ of Nazareth, forever. Amen.
*/
