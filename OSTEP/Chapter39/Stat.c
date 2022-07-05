#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

int main(int argc, char *argv[])
{
    if(argc != 2){
        printf("usage: ./Stat filePath\n");
        return -1;
    }

    char *path = argv[1];
    struct stat s;
    stat(path, &s);
    printf("file size: %ld\n", s.st_size);
    printf("block num: %ld\n", s.st_blocks);
    printf("reference count: %ld\n", s.st_nlink);
}