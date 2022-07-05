#include <unistd.h>
#include <sys/types.h>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUF_SIZE 4096

static void recursivePrint(char *root, int indent)
{
    for(int i = 0; i < indent; ++i){
            printf(" ");
    }
    printf("%s\n", root);

    DIR *dir = opendir(root);
    if(dir == NULL){
        return;
    }

    struct dirent* entry;
    while((entry = readdir(dir)) != NULL){
        if(strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0){
            continue;
        }
        char newRoot[BUF_SIZE];
        strcpy(newRoot, root);
        strcat(newRoot, "/");
        strcat(newRoot, entry->d_name);
        recursivePrint(newRoot, indent + 1);
    }
}

int main(int argc, char *argv[])
{
    if(argc > 2){
        printf("usage: Recursive (beginPath)\n");
        exit(-1);
    }

    char root[BUF_SIZE];
    if(argc == 1){
        if(getcwd(root, BUF_SIZE) == NULL){
            printf("root path too long\n");
            exit(-1);
        }
        recursivePrint(root, 0);
    }else if(argc == 2){
        recursivePrint(argv[1], 0);
    }
}