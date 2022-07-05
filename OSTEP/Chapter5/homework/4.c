#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

int main(int argc, char *argv[]){
    int rc = fork();
    if(rc < 0){
        fprintf(stderr, "fork failed\n");
    }else if(rc == 0){
        printf("ls from child process\n");
        execl("/bin/ls", "ls", NULL);
    }else{
        printf("ls from parent process\n");
        char *args[] = {"ls", NULL};
        execvp("/bin/ls", args);
    }
    return 0;
}