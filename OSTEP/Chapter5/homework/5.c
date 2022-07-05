#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>

int main(int argc, char *argv[]){
    int rc = fork();
    int rc_wait;
    if(rc < 0){
        fprintf(stderr, "fork failed\n");
    }else if(rc == 0){
        rc_wait = wait(NULL);
        printf("wait return %d in child process (pid:%d)\n", rc_wait, (int) getpid());
    }else{
        rc_wait = wait(NULL);
        printf("wait return %d in parent process(pid:%d)\n", rc_wait, (int) getpid());
    }
    return 0;
}