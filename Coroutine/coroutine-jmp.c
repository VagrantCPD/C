#include <coroutine-jmp.h>
#include <unistd.h>

static scheduler_t* coroutine_scheduler = NULL;

//  default shceduler policy: choose next coroutine in wait queue
//  return NULL means all coroutines are done
static coroutine_t* default_sched_policy(void)
{ 
    coroutine_t *coroutine = NULL;

    for_each_in_list(coroutine, coroutine_t, wait_queue_node, &(coroutine_scheduler->wait_queue)){
        if(coroutine->state == RUNNING || coroutine->state == IDLE){
            list_del(&(coroutine->wait_queue_node));
            list_append(&(coroutine->wait_queue_node), &(coroutine_scheduler->wait_queue));
            return coroutine;
        }
	}

    return NULL;
}

void create_scheduler()
{
    coroutine_scheduler = (scheduler_t *) malloc(sizeof(scheduler_t));
    coroutine_scheduler->running_coroutine = NULL;
    coroutine_scheduler->coroutine_count = 0;
    coroutine_scheduler->coroutines = (coroutine_t **) malloc(sizeof(coroutine_t *) * MAX_COROUTINE);
    init_list_head(&(coroutine_scheduler->wait_queue));
    memset(coroutine_scheduler->coroutines, 0, sizeof(coroutine_t *) * MAX_COROUTINE);
}

int get_running_coroutine()
{
    return coroutine_scheduler->running_coroutine->id;
}

int create_coroutine(target_func func, void *args)
{
    coroutine_t *coroutine = NULL;
    int i = 0;
    for(; i < coroutine_scheduler->coroutine_count; ++i){
        if(coroutine_scheduler->coroutines[i]->state == DEAD){
            break;
        }
    }
    if(i == coroutine_scheduler->coroutine_count){
        coroutine_scheduler->coroutines[i] = (coroutine_t *) malloc(sizeof(coroutine_t));
        coroutine_scheduler->coroutine_count += 1;
    }

    coroutine = coroutine_scheduler->coroutines[i];
    coroutine->state = IDLE;
    coroutine->args = args;
    coroutine->func = func;
    coroutine->stack_bottom = malloc(STACK_SIZE);
    coroutine->stack_top = (void *)((u64) coroutine->stack_bottom + STACK_SIZE);
    coroutine->id = i;
    list_append(&(coroutine->wait_queue_node), &(coroutine_scheduler->wait_queue));
    return i;
}

void run_scheduler()
{
    switch(setjmp(coroutine_scheduler->context))
    {
        case COROUTINE_EXIT:
        case COROUTINE_INIT:
        case COROUTINE_SCHEDULE:
            schedule();
            return;
        default:
            return;
    }
}

void exit_coroutine()
{
    coroutine_t *coroutine = coroutine_scheduler->running_coroutine;
    coroutine->state = DEAD;
    list_del(&(coroutine->wait_queue_node));
    longjmp(coroutine_scheduler->context, COROUTINE_EXIT);
}

void yield_coroutine()
{
    if(setjmp(coroutine_scheduler->running_coroutine->context)){
        return;
    }else{
        longjmp(coroutine_scheduler->context, COROUTINE_SCHEDULE);
    }
}

void schedule()
{
    coroutine_t *next_coroutine = default_sched_policy();
    if(next_coroutine == NULL){
        return;
    }

    coroutine_scheduler->running_coroutine = next_coroutine;
    if(next_coroutine->state == IDLE){
        // set separate sp for each coroutine
        register void *stack_top = next_coroutine->stack_top;

#ifdef ARCH_X86_64
        asm volatile(
            "mov %[rs], %%rsp \n"
            ::[ rs ] "+r" (stack_top):
        );
#else
        __asm__ volatile(
            "mov sp, %[tos] \n"
            ::[tos] "r" (stack_top):
        );
#endif

        next_coroutine->state = RUNNING;
        next_coroutine->func(next_coroutine->args);        
        exit_coroutine();
    }else{
        longjmp(next_coroutine->context, 1);
    }
}

void close_scheduler()
{
    for(int i = 0; i < coroutine_scheduler->coroutine_count; ++i){
        coroutine_t *coroutine = coroutine_scheduler->coroutines[i];
        free(coroutine->stack_bottom);
        free(coroutine);
    }

    free(coroutine_scheduler->coroutines);
    free(coroutine_scheduler);
    coroutine_scheduler = NULL;
}
