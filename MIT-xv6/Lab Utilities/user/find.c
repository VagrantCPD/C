#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/fs.h"
#include "user/user.h"

#define MAX_LEN 512

// compare path with given name
int compare(char *path, char *target)
{
    char *p;

    // Find first character after last slash.
    for(p=path+strlen(path); p >= path && *p != '/'; p--)
        ;
    p++;

    char *q = target;
    for(; *p != 0 && *q != 0; ++p, ++q){
        if(*p != *q){
            return -1;
        }
    }

    if(*p != 0 || *q != 0){
        return -1;
    }

    return 0;
}

void find(char *root, char *target)
{
    struct dirent de;
    struct stat st;
    char newRoot[MAX_LEN];
    char *p;
    int fd;

    if((fd = open(root, 0)) < 0){
        printf("cannot open file %s\n", root);
        exit(-1);
    }

    if(fstat(fd, &st) < 0){
        printf("cannot stat file %s\n", root);
        exit(-1);
    }

    switch (st.type)
    {
    case T_FILE:
        if(compare(root, target) == 0){
            printf("%s\n", root);
        }
        break;
    case T_DIR:
        strcpy(newRoot, root);
        p = newRoot + strlen(newRoot);
        *p++ = '/';
        while(read(fd, &de, sizeof(de)) == sizeof(de)){
            if(de.inum == 0 || strcmp(de.name, ".") == 0 || strcmp(de.name, "..") == 0){
                continue;
            }
            memmove(p, de.name, DIRSIZ);
            p[DIRSIZ] = 0;
            find(newRoot, target);
        }
        break;
    default:
        break;
    }

    close(fd);
}

int main(int argc, char *argv[])
{
    char root[MAX_LEN];
    char target[MAX_LEN];

    if(argc != 3){
        printf("usage: find root target\n");
        exit(-1);
    }
    strcpy(root, argv[1]);
    strcpy(target, argv[2]);
    find(root, target);
    exit(0);
}