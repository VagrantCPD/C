#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/wait.h>

int main(int argc, char *argv[]){
    int rc = fork();
    if(rc < 0){
        fprintf(stderr, "fork failed\n");
    }else if(rc == 0){
        close(STDOUT_FILENO);
    }else{
        wait(NULL);
        printf("hello from parent process\n");
    }
    return 0;
}