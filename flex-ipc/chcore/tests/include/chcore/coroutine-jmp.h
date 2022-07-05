#pragma once

#include <setjmp.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <chcore/type.h>
#include <chcore/container/list.h>

#define STACK_SIZE  (4096 * 2)
#define MAX_COROUTINE   1024

enum COROUTINE_STATE
{
    IDLE,
    RUNNING,
    DEAD
};

enum JUMP_STATE
{
    COROUTINE_INIT=0,
    COROUTINE_SCHEDULE,
    COROUTINE_EXIT
};

struct scheduler;

typedef void* (*target_func) (void *);

typedef struct coroutine
{
    enum COROUTINE_STATE state;
    void *args;
    target_func func;
    jmp_buf context;
    void *stack_top;
    void *stack_bottom;
    struct list_head wait_queue_node;
    int id;
} coroutine_t;

typedef struct scheduler
{
    coroutine_t *running_coroutine;
    int coroutine_count;
    coroutine_t **coroutines;
    jmp_buf context;
    struct list_head wait_queue;
} scheduler_t;

void create_scheduler();
int create_coroutine(target_func func, void *args);
void run_scheduler();
void exit_coroutine();
void yield_coroutine();
void schedule();
void close_scheduler();
int get_running_coroutine();