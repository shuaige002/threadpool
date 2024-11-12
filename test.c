#include "thread_pool.h"
#include <unistd.h>

void example_task(void* arg) {
    int* num = (int*)arg;
    printf("Executing task with number: %d\n", *num);
    sleep(1); // Simulate work
}

int main() {
    ThreadPool* pool = create_thread_pool(2);

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