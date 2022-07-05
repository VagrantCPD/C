#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/wait.h>

int main(int argc, char *argv[]){
    int fd[2];
    char buffer[100];
    
    // msg write into fd[1] can be read from fd[0]
    if(pipe(fd) < 0){
        fprintf(stderr, "pipe failed\n");
        exit(1);
    }

    int pid1 = fork();
    if(pid1 < 0){
        fprintf(stderr, "fork1 failed\n");
    }else if(pid1 == 0){
        char *msg = "hello from child process1\n";
        strcpy(buffer, msg);
        write(fd[1], buffer, strlen(msg) + 1);
        exit(0);
    }

    int pid2 = fork();
    if(pid2 < 0){
        fprintf(stderr, "fork2 failed\n");
    }else if(pid2 == 0){
        read(fd[0], buffer, 100);
        printf("msg in child process2: %s", buffer);
        exit(0);
    }

    int wait1 = waitpid(pid1, NULL, 0);
    int wait2 = waitpid(pid2, NULL, 0);
    return 0;
}