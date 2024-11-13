
#include "thread_pool.h"


static void* thread_function(void* arg) {
    ThreadPool* pool = (ThreadPool*)arg;
    Task task;

    while (true) {
        pthread_mutex_lock(&pool->lock);

        while (pool->task_count == 0 && !pool->shutdown) {
            pthread_cond_wait(&pool->cond, &pool->lock);
        }

        if (pool->shutdown) {
            if (pool->force_shutdown || pool->task_count == 0) {
                pthread_mutex_unlock(&pool->lock);
                break;
            }
        }

        task = pool->tasks[pool->task_head];
        pool->task_head = (pool->task_head + 1) % pool->max_tasks;
        pool->task_count--;

        pthread_mutex_unlock(&pool->lock);

        if (task.func != NULL) {
            task.func(task.arg);
        }
    }

    return NULL;
}

ThreadPool* create_thread_pool(int max_threads) {
    ThreadPool* pool = (ThreadPool*)malloc(sizeof(ThreadPool));
    if (pool == NULL) {
        perror("Failed to allocate memory for ThreadPool");
        return NULL;
    }
 
    pool->task_count = 0;
    pool->task_head = 0;
    pool->task_tail = 0;
    pool->shutdown = false;
    pool->force_shutdown = false;
    pool->max_threads = max_threads;
    pool->max_tasks = (max_threads<<3);
 
    if (pthread_mutex_init(&pool->lock, NULL) != 0) {
        perror("Failed to initialize mutex");
        free(pool);
        return NULL;
    }
 
    if (pthread_cond_init(&pool->cond, NULL) != 0) {
        perror("Failed to initialize condition variable");
        pthread_mutex_destroy(&pool->lock);
        free(pool);
        return NULL;
    }
 
    pool->tasks = (Task*)malloc((max_threads<<3) * sizeof(Task));
    pool->threads = (pthread_t*)malloc(max_threads * sizeof(pthread_t));

    if (pool->tasks == NULL) {
    perror("Failed to allocate memory for tasks");
    pthread_mutex_destroy(&pool->lock);
    pthread_cond_destroy(&pool->cond);
    free(pool->threads);
    free(pool);
    return NULL;
    }


    if (pool->threads == NULL) {
        perror("Failed to allocate memory for threads");
        pthread_mutex_destroy(&pool->lock);
        pthread_cond_destroy(&pool->cond);
        free(pool->tasks);
        free(pool);
        return NULL;
    }
 
    for (int i = 0; i < max_threads; i++) {
        if (pthread_create(&pool->threads[i], NULL, thread_function, (void*)pool) != 0) {
            perror("Failed to create thread");
 
            // 释放线程数组已分配的内存
            for (int j = 0; j < i; j++) {
                pthread_cancel(pool->threads[j]); // 尝试取消已创建的线程（注意：这可能需要适当的处理）
                pthread_join(pool->threads[j], NULL); // 等待线程结束
            }
            free(pool->threads);
 
            // 销毁互斥锁和条件变量
            pthread_mutex_destroy(&pool->lock);
            pthread_cond_destroy(&pool->cond);
 
            // 释放ThreadPool结构体
            free(pool);
 
            return NULL;
        }
    }
 
    return pool;
}

void add_task(ThreadPool* pool, void (*func)(void*), void* arg) {
    pthread_mutex_lock(&pool->lock);

    if (pool->task_count == (pool->max_tasks)) {
        pthread_mutex_unlock(&pool->lock);
        fprintf(stderr, "Task queue is full!\n");
        return;
    }

    pool->tasks[pool->task_tail].func = func;
    pool->tasks[pool->task_tail].arg = arg;
    pool->task_tail = (pool->task_tail + 1) % pool->max_tasks;
    pool->task_count++;

    pthread_cond_signal(&pool->cond);
    pthread_mutex_unlock(&pool->lock);
}

void shutdown_thread_pool(ThreadPool* pool) {
    pthread_mutex_lock(&pool->lock);
    pool->shutdown = true;
    pthread_cond_broadcast(&pool->cond);
    pthread_mutex_unlock(&pool->lock);

    for (int i = 0; i < pool->max_threads; i++) {
        pthread_join(pool->threads[i], NULL);
    }
}

void force_shutdown_thread_pool(ThreadPool* pool) {
    pthread_mutex_lock(&pool->lock);
    pool->shutdown = true;
    pool->force_shutdown = true;
    pthread_cond_broadcast(&pool->cond);
    pthread_mutex_unlock(&pool->lock);

    for (int i = 0; i < pool->max_threads; i++) {
        pthread_cancel(pool->threads[i]);
        pthread_join(pool->threads[i], NULL);
    }
}

void destroy_thread_pool(ThreadPool* pool) {
    if (pool != NULL) {
        pthread_mutex_destroy(&pool->lock);
        pthread_cond_destroy(&pool->cond);
 
        // 释放线程数组的内存
        if (pool->threads != NULL) {
            free(pool->threads);
        }
        // 释放任务数组的内存
        if (pool->tasks != NULL) {
            free(pool->tasks);
        }
 
        free(pool);
    }
}