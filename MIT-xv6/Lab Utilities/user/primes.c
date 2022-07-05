#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

// https://swtch.com/~rsc/thread/
void process(int pipes[])
{
    int beginPrime;
    close(pipes[1]);
    read(pipes[0], &beginPrime, 4);
    printf("prime %d\n", beginPrime);
    int forkChild = 0;
    int _pipes[2];

    int i;
    while(read(pipes[0], &i, 4) != 0){
        if(i % beginPrime != 0){
            if(forkChild == 0){
                forkChild = 1;
                pipe(_pipes);
                if(fork() == 0){
                    process(_pipes);
                }else{
                    close(_pipes[0]);
                }
            }
            write(_pipes[1], &i, 4);
        }
    }
    close(pipes[0]);
    close(_pipes[1]);
    wait(0);
}

int main(int argc, char *argv[])
{
    int beginPrime = 2;
    printf("prime %d\n", beginPrime);
    int forkChild = 0;
    int pipes[2];

    for(int i = beginPrime + 1; i <= 35; ++i){
        if(i % beginPrime != 0){
            if(forkChild == 0){
                forkChild = 1;
                pipe(pipes);
                if(fork() == 0){
                    process(pipes);
                }else{
                    close(pipes[0]);
                }
            }
            write(pipes[1], &i, 4);
        }
    }
    close(pipes[1]);
    wait(0);
    exit(0);
}