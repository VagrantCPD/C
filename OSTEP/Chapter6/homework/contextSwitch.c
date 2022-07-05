 #define _GNU_SOURCE 
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include <pthread.h>
#include <sys/wait.h>
#include <sched.h>

// 通过管道来测量进程上下文切换的开销
int pipes[2];
char buffer[10];
int count = 100000;
int CPU_ID = 0;
cpu_set_t mask;

int main(int argc, char *argv[])
{
    if(pipe(pipes) < 0){
        fprintf(stderr, "pipe failed\n");
        exit(1);
    }

    CPU_ZERO(&mask);
    int rc = fork();
    if(rc < 0){
        fprintf(stderr, "fork failed\n");
        exit(2);
    }else {
        struct timeval start, end;
        if(rc == 0){
            // 绑定进程运行的CPU
            CPU_SET(CPU_ID, &mask);
            if(sched_setaffinity(getpid(), sizeof(mask), &mask) < 0){
                fprintf(stderr, "set affinity failed in child process\n");
                exit(3);
            }
            for(int i = 0; i < count; ++i){
                read(pipes[0], buffer, 10);
                write(pipes[0], buffer, 10);
            }
        }
        else{
             // 绑定进程运行的CPU
            CPU_SET(CPU_ID, &mask);
            if(sched_setaffinity(getpid(), sizeof(mask), &mask) < 0){
                fprintf(stderr, "set affinity failed in parent process\n");
                exit(3);
            }
            gettimeofday(&start, NULL);

            for(int i = 0; i < count; ++i){
                write(pipes[1], buffer, 10);
                read(pipes[1], buffer, 10);
            }

            gettimeofday(&end, NULL);
            double average_context_switch_time = (double) (1000000 * (end.tv_sec - start.tv_sec) + end.tv_usec - start.tv_usec) / count / 2;
            printf("average context switch time is %6.6f\n", average_context_switch_time);
        }
    }

    return 0;
}