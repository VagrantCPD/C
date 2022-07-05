#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int main(int argc, char *argv[])
{
    if(argc != 1){
        printf("usage: pingpong\n");
        exit(-1);
    }

    // pipe[0] is fd for read, pipe[1] is fd for write
    int pipe1[2];
    int pipe2[2];

    if(pipe(pipe1) < 0 || pipe(pipe2) < 0){
        printf("pipe error\n");
        exit(-1);
    }

    int pid = fork();
    
    if(pid < 0){
        printf("fork error\n");
        exit(-1);
    }

    if(pid == 0){
        close(pipe1[0]);
        close(pipe2[1]);
        char buf[1];
        read(pipe2[0], buf, 1);
        printf("%d: received ping\n", getpid());
        write(pipe1[1], buf, 1);
        close(pipe1[1]);
        close(pipe2[0]);
        exit(0);
    }else{
        close(pipe1[1]);
        close(pipe2[0]);
        char buf[1];
        write(pipe2[1], "a", 1);
        read(pipe1[0], buf, 1);
        printf("%d: received pong\n", getpid());
        close(pipe1[0]);
        close(pipe2[1]);
        exit(0);
    }
}