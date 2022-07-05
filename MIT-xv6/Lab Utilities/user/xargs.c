#include "kernel/types.h"
#include "kernel/param.h"
#include "user/user.h"

int main(int argc, char *argv[])
{
    if(argc < 2){
        printf("too few arguments\n");
        exit(-1);
    }

    char *args[MAXARG];
    int argNum = argc - 1;
    for(int i = 1; i < argc; ++i){
        args[i - 1] = argv[i];
    }

    char buf[100];
    char *p = buf;
    while(read(0, p, 1) != 0){
        if(*p == '\n'){
            // mark the end of the buffer
            *p = 0;
            if(fork() == 0){
                args[argNum] = buf;
                exec(args[0], args);
            }else{
                wait(0);
            }
            p = buf;
        }else{
            ++p;
        }
    }
    exit(0);
}