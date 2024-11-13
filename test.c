#include "thread_pool.h"
#include <unistd.h>

int num_processors;
ThreadPool* pool = NULL;

void example_task(void* arg) {
    int* num = (int*)arg;
    printf("Executing task with number: %d\n", *num);
    printf("max_tasks:%d\n",pool->max_tasks);
    sleep(3); // Simulate work
}

int main() {
    num_processors = sysconf(_SC_NPROCESSORS_ONLN);
    pool = create_thread_pool(num_processors);
    printf("CPU cero number: %d\n", num_processors);

    for (int i = 0; i < 15; i++) {
        int* num = malloc(sizeof(int));
        *num = i;
        add_task(pool, example_task, (void*)num);
    }

    // Sleep to allow tasks to be processed
    sleep(5);

    // Shutdown thread pool gracefully
    // shutdown_thread_pool(pool);

    // Force shutdown thread pool
    shutdown_thread_pool(pool);

    destroy_thread_pool(pool);

    return 0;
}