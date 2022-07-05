#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

int main(int argc, char *argv[]){
    int rc = fork();
    if(rc < 0){
        fprintf(stderr, "fork failed\n");
    }else if(rc == 0){
        printf("hello from child process\n");
    }else{
        sleep(3);
        printf("hello from parent process\n");
    }
    return 0;
}