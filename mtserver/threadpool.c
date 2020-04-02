/**
 * threadpool.c
 *
 * This file will contain your implementation of a threadpool.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

#include "threadpool.h"

// _threadpool is the internal threadpool structure that is
// cast to type "threadpool" before it given out to callers

typedef struct _threadpool_function_st{
  void (*routine)(void *);          
  void *arg;                        
  struct _threadpool_function_st *next; 
} threadpool_function;

typedef struct _threadpool_st {
   // you should fill in this structure with whatever you need
  int threadsNumber;
  int jobNumber;
  pthread_t *threads;
  threadpool_function *first;
  threadpool_function *last;
  pthread_mutex_t lock;
  pthread_cond_t isEmpty;
  pthread_cond_t empty;
  int down;
  int busy;   
} _threadpool;


void *worker_thread(threadpool p) {
  _threadpool *pool = (_threadpool *)p;
  threadpool_function *function;
  while (1) {
    // wait for a signal
    // l
    // mark itself as busy
    // run a given function
    //
    pool->jobNumber = pool->jobNumber;
    pthread_mutex_lock(&(pool->lock));
    while(pool->jobNumber == 0){
      if(pool->down){
      pthread_mutex_unlock(&(pool->lock));
      pthread_exit(NULL);
      }
      pthread_mutex_unlock(&(pool->lock));                 
      pthread_cond_wait(&(pool->isEmpty), &(pool->lock)); 
      if(pool->down){
        pthread_mutex_unlock(&(pool->lock));
        pthread_exit(NULL);
      }
    }
    function = pool->first;
    pool->jobNumber--;
    if(pool->jobNumber == 0){
        pool->first = NULL;
        pool->last = NULL;
    }else{
      pool->first = function->next;
    }
    if(pool->jobNumber == 0 && !pool->down){
      pthread_cond_signal(&(pool->empty));
    }
    pthread_mutex_unlock(&(pool->lock));
    (function->routine)(function->arg);
    free(function);
  }
}

threadpool create_threadpool(int num_threads_in_pool) {
_threadpool *pool;
  // sanity check the argument
  if ((num_threads_in_pool <= 0) || (num_threads_in_pool > MAXT_IN_POOL))
    return NULL;

  pool = (_threadpool *) malloc(sizeof(_threadpool));
  if (pool == NULL) {
    fprintf(stderr, "Out of memory creating a new threadpool!\n");
    return NULL;
  }

  // add your code here to initialize the newly created threadpool
  pool->threads = (pthread_t *)malloc(sizeof(pthread_t) * num_threads_in_pool);
  if(!pool->threads){
        printf("Out of memory creating a new threadpool!\n");
        return NULL;
    }
    pool->threadsNumber = num_threads_in_pool;
    pool->jobNumber = 0;
    pool->first = NULL;
    pool->last = NULL;
    pool->down = 0;
    pool->busy = 0;
    if(pthread_mutex_init(&pool->lock, NULL)){
      printf("Mutex initiation error!\n");
      return NULL;
    }
    if(pthread_cond_init(&(pool->empty), NULL)){
      printf("CV initiation error!\n");
      return NULL;
    }
    if(pthread_cond_init(&(pool->isEmpty), NULL)){
      printf("CV initiation error!\n");
      return NULL;
    }
    for(int i = 0; i < num_threads_in_pool; i++){
      if(pthread_create(&(pool->threads[i]), NULL, worker_thread, pool)){
          printf("Thread initiation error!\n");
          return NULL;
      }
    }
  return (threadpool) pool;
}


void dispatch(threadpool from_me, dispatch_fn dispatch_to_here,
        void *arg) {
  _threadpool *pool = (_threadpool *) from_me;

  // add your code here to dispatch a thread
  threadpool_function *function;
  function = (threadpool_function *)malloc(sizeof(threadpool_function));
  if(function == NULL){
      printf("Out of memory creating a work struct!\n");
      return;
  }
  function->routine = dispatch_to_here;
  function->arg = arg;
  function->next = NULL;
  pthread_mutex_lock(&(pool->lock));
  if(pool->busy){
    free(function);
    return;
  }
  if(pool->jobNumber == 0){
    pool->first = function;
    pool->last = function;
    pthread_cond_signal(&(pool->isEmpty));
  }else{
    pool->last->next = function;
    pool->last = function;
  }
  pool->jobNumber++;
  pthread_mutex_unlock(&(pool->lock));
}

void destroy_threadpool(threadpool destroyme) {
  _threadpool *pool = (_threadpool *) destroyme;

  // add your code here to kill a threadpool
  pthread_mutex_lock(&(pool->lock));
  pool->busy = 1;
  while(pool->jobNumber != 0){
    pthread_cond_wait(&(pool->empty), &(pool->lock));
  }
  pool->down = 1;                            
  pthread_cond_broadcast(&(pool->isEmpty));
  pthread_mutex_unlock(&(pool->lock));
  for(int i = 0; i < pool->threadsNumber; i++){
    pthread_cond_broadcast(&(pool->isEmpty));
    pthread_join(pool->threads[i], NULL);
  }
  free(pool->threads);
  pthread_mutex_destroy(&(pool->lock));
  pthread_cond_destroy(&(pool->empty));
  pthread_cond_destroy(&(pool->isEmpty));
  free(pool);
}

