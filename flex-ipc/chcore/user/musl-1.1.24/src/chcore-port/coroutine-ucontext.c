// #include <chcore/coroutine-ucontext.h>

// scheduler_t *create_scheduler()
// {
//     scheduler_t *scheduler = (scheduler_t *) malloc(sizeof(scheduler_t));
//     scheduler->running_coroutine = -1;
//     scheduler->coroutine_count = 0;
//     scheduler->coroutines = (coroutine_t **) malloc(sizeof(coroutine_t *) * MAX_COROUTINE);
//     memset(scheduler->coroutines, 0, sizeof(coroutine_t *) * MAX_COROUTINE);
//     // init_list_head(&wait_queue);
// 	// init_list_head(&ready_queue);
//     return scheduler;
// }

// // the function to execute coroutine
// static void main_function(scheduler_t *scheduler)
// {
//     int coroutine_id = scheduler->running_coroutine;
//     if(coroutine_id != -1){
//         coroutine_t *coroutine = scheduler->coroutines[coroutine_id];
//         if(coroutine->state != DEAD){
//             coroutine->state = RUNNING;
//             coroutine->func(coroutine->args);
//             coroutine->state = DEAD;
//             scheduler->running_coroutine = -1;
//         }
//     }
// }

// int create_coroutine(scheduler_t *scheduler, target_func func, void *args)
// {
//     coroutine_t *coroutine = NULL;
//     int i = 0;
//     for(; i < scheduler->coroutine_count; ++i){
//         if(scheduler->coroutines[i]->state == DEAD){
//             break;
//         }
//     }
//     if(i == scheduler->coroutine_count){
//         scheduler->coroutines[i] = (coroutine_t *) malloc(sizeof(coroutine_t));
//         scheduler->coroutine_count += 1;
//     }

//     coroutine = scheduler->coroutines[i];
//     coroutine->state = IDLE;
//     coroutine->args = args;
//     coroutine->func = func;
//     getcontext(&(coroutine->context));
//     coroutine->context.uc_stack.ss_size = STACK_SIZE;
//     coroutine->context.uc_stack.ss_flags = 0;
//     coroutine->context.uc_stack.ss_sp = coroutine->stack;
//     coroutine->context.uc_link = &(scheduler->context);
//     makecontext(&(coroutine->context), (void (*)(void))main_function, 1, scheduler);

//     return i;
// }

// void run_coroutine(scheduler_t *scheduler, int coroutine_id)
// {
//     if(coroutine_id < 0 || coroutine_id >= scheduler->coroutine_count){
//         return;
//     }

//     scheduler->running_coroutine = coroutine_id;
//     swapcontext(&(scheduler->context), &(scheduler->coroutines[coroutine_id]->context));
// }

// void yield_coroutine(scheduler_t *scheduler)
// {
//     int coroutine_id = scheduler->running_coroutine;
//     if(coroutine_id == -1){
//         return;
//     }
//     coroutine_t *coroutine = scheduler->coroutines[coroutine_id];
//     coroutine->state = WAITING;
//     scheduler->running_coroutine = -1;
//     swapcontext(&(coroutine->context), &(scheduler->context));
// }

// int get_coroutine_id(scheduler_t *scheduler)
// {
//     return scheduler->running_coroutine;
// }

// int is_scheduler_finished(scheduler_t *scheduler)
// {
//     if(scheduler->coroutine_count == 0){
//         return 1;
//     }

//     for(int i = 0; i < scheduler->coroutine_count; ++i){
//         if(scheduler->coroutines[i]->state != DEAD){
//             return 0;
//         }
//     }

//     return 1;
// }

// void close_scheduler(scheduler_t *scheduler)
// {
//     for(int i = 0; i < scheduler->coroutine_count; ++i){
//         free(scheduler->coroutines[i]);
//         scheduler->coroutines[i] = NULL;
//     }

//     free(scheduler->coroutines);
//     scheduler->coroutines = NULL;
//     free(scheduler);
//     scheduler = NULL;
// }