// #pragma once

// #include <ucontext.h>
// #include <stdlib.h>
// #include <string.h>

// // struct list_head wait_queue;
// // struct list_head ready_queue;

// #define STACK_SIZE  (1024 * 128)
// #define MAX_COROUTINE   1024

// enum COROUTINE_STATE
// {
//     IDLE,
//     RUNNING,
//     WAITING,
//     DEAD
// };

// struct scheduler;

// typedef void* (*target_func) (void *);

// typedef struct coroutine
// {
//     enum COROUTINE_STATE state;
//     void *args;
//     target_func func;
//     ucontext_t context;
//     char stack[STACK_SIZE];
// } coroutine_t;

// typedef struct scheduler
// {
//     int running_coroutine;
//     int coroutine_count;
//     coroutine_t **coroutines;
//     ucontext_t context;
// } scheduler_t;

// scheduler_t * create_scheduler();
// int create_coroutine(scheduler_t *scheduler, target_func func, void *args);
// void run_coroutine(scheduler_t *scheduler, int coroutine_id);
// void yield_coroutine(scheduler_t *scheduler);
// int get_coroutine_id(scheduler_t *scheduler);
// int is_scheduler_finished(scheduler_t *scheduler);
// void close_scheduler(scheduler_t *scheduler);