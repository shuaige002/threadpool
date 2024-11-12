#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#ifndef _THREADPOOL_H_
#define _THREADPOOL_H_

#define QUEUE_SIZE 1000

typedef struct {
    void (*func)(void*);
    void* arg;
} Task;

typedef struct {
    pthread_mutex_t lock;
    pthread_cond_t cond;
    pthread_t* threads;
    Task tasks[QUEUE_SIZE];
    int task_count;
    int task_head;
    int task_tail;
    bool shutdown;
    bool force_shutdown;
    int max_threads;
} ThreadPool;

ThreadPool* create_thread_pool(int max_threads);
void add_task(ThreadPool* pool, void (*func)(void*), void* arg);
void shutdown_thread_pool(ThreadPool* pool);
void force_shutdown_thread_pool(ThreadPool* pool);
void destroy_thread_pool(ThreadPool* pool);

#endif