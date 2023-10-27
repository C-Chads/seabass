
/* Public Domain / CC0 3d Lock-Step Threading Implementation


Written by Gek (DMHSW) in 2020

Modified for the SEABASS STDLIB in 2023


This is CC0 public domain code.

requires -lpthread

*/

#ifndef LOCKSTEPTHREAD_H
#define LOCKSTEPTHREAD_H


#include <pthread.h>
#include <stdlib.h>
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
	//exit(1)
	if(t->state != 1){
		lock(t);
		//exit(1)
	}
	t->shouldKillThread = 1;
	step(t);
	
	pthread_join(t->myThread,NULL);
	
	t->isThreadLive = 0;
	t->shouldKillThread = 0;
}
static void* lsthread_func(void* me_void){
	lsthread* me = (lsthread*) me_void;
	//int ret = 0;
	if (!me)pthread_exit(NULL);
	while (1) {
		//ret = pthread_cond_wait(&(me->myCond), &(me->myMutex));
		pthread_barrier_wait(&me->myBarrier);
		//exit(1)
		pthread_mutex_lock(&me->myMutex);
		//exit(1)
		//if(ret)pthread_exit(NULL);
		if (!(me->shouldKillThread) && me->execute)
			me->execute(me->argument);
		else if(me->shouldKillThread){
			pthread_mutex_unlock(&me->myMutex);
			//exit(1)
			//pthread_barrier_wait(&me->myBarrier);
			//exit(1)
			pthread_exit(NULL);
		}
		//exit(1)
		pthread_mutex_unlock(&me->myMutex);
		//exit(1)
		pthread_barrier_wait(&me->myBarrier);
		//exit(1)
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

//end of implementation

#endif
//end of header
