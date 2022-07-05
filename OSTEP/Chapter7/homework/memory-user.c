#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
    if(argc != 2){
        fprintf(stderr, "usage: ./memory-user N\n");
        exit(1);
    }

    int size = atoi(argv[1]) * 1e6;

    int *p = (int *) malloc(size);
    while(1)
    {
        for(int i = 0; i < (size / sizeof(int)); ++i)
        {
            p[i] = i;
        }
        printf("memory user is running\n");
    }
    return 0;
}