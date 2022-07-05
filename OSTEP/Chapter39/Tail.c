#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static long int getFileSize(char *filePath)
{
    struct stat s;
    stat(filePath, &s);
    return s.st_size;
}

int main(int argc, char *argv[])
{
    if(argc != 3){
        printf("usage: Tail -n file (n is the number of lines at the end of the file to print)\n");
        exit(-1);
    }

    long int fileSize = getFileSize(argv[2]);
    int n = atoi((argv[1] + 1));
    int fd = open(argv[2], O_RDONLY);
    int offset = fileSize - 1;
    char buf[1];
    while(n != 0){
        lseek(fd, offset, SEEK_SET);
        read(fd, buf, 1);
        --offset;
        if(strcmp(buf, "\n") == 0){
            n--;
        }
    }
    offset += 2;

    lseek(fd, offset, SEEK_SET);
    while((read(fd, buf, 1)) != 0){
        printf("%s", buf);
    }
    close(fd);
}
