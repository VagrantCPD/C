#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

int main(int argc, char *argv[]){
    int fd = open("test.txt", O_WRONLY | O_RDONLY | O_CREAT, S_IRWXU | S_IRWXO);
    int rc = fork();
    if(rc < 0){
        fprintf(stderr, "fork failed\n");
    }else if(rc == 0){
        printf("fd in child process %d\n", fd);
        char *buf = "hello in child process\n";
        write(fd, buf, strlen(buf));
    }else{
        printf("fd in parent process %d\n", fd);
        char *buf = "hello in parent process\n";
        write(fd, buf, strlen(buf));
    }
    return 0;
}