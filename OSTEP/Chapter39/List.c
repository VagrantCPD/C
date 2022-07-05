#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void statFile(char *filePath)
{
    struct stat s;
    stat(filePath, &s);
    printf("device ID: %ld\n", s.st_dev);
    printf("inode number: %ld\n", s.st_ino);
    printf("file type: %d\n", s.st_mode);
    printf("reference count: %ld\n", s.st_nlink);
    printf("user ID of owner: %d\n", s.st_uid);
    printf("group ID of owner: %d\n", s.st_gid);
    printf("size: %ld\n", s.st_size);
    printf("block size: %ld\n", s.st_blksize);
    printf("block count: %ld\n", s.st_blocks);
}

static void printDirEntry(char *dirPath, int level)
{
    DIR *dir = opendir(dirPath);
    if(dir == NULL){
        printf("cannot find directory %s\n", dirPath);
        exit(-1);
    }
    struct dirent* entry;
    
    while((entry = readdir(dir)) != NULL){
        printf("%s\n", entry->d_name);
        if(level == 1){
            char filePath[100];
            strcpy(filePath, dirPath);
            strcat(filePath, "/");
            strcat(filePath, entry->d_name);
            statFile(filePath);
        }
    }       
}

int main(int argc, char *argv[])
{
    if(argc > 3){
        printf("usage: List (-l) (directory)\n");
        exit(-1);
    }

    char dirPath[100];

    if(argc == 1){
        if(getcwd(dirPath, 100) == NULL){
            printf("directory path too long\n");
            exit(-1);
        }
        printDirEntry(dirPath, 0);
    }else if(argc == 2){
        if(strcmp(argv[1], "-l") == 0){
            if(getcwd(dirPath, 100) == NULL){
                printf("directory path too long\n");
                exit(-1);
            } 
            printDirEntry(dirPath, 1);  
        }else{
            printDirEntry(argv[1], 0);
        }
    }else if(argc == 3){
        if(strcmp(argv[1], "-l") == 0){
            printDirEntry(argv[2], 1);
        }else if(strcmp(argv[2], "-l") == 0){
            printDirEntry(argv[1], 1);
        }else{
            printf("usage: List (-l) (directory)\n");
            exit(-1);
        }
    }
}
