#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>

int main(int argc, char *argv[]){
    int x = 100;
    int rc = fork();
    if(rc < 0){
        fprintf(stderr, "fork failed\n");
    }else if(rc == 0){
        printf("x in child process %d\n", x);
        x = 200;
        printf("child process change x to %d\n", x);
    }else{
        printf("x in parent process %d\n", x);
        x = 300;
        printf("parent process change x to %d\n",x);
    }
    return 0;
}