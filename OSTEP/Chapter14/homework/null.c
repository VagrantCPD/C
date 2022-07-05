#include <stdlib.h>
#include <stdio.h>

#pragma region 1

void one()
{
    int *p = NULL;
    free(p);
}

#pragma endregion

#pragma region 4

void four()
{
    int *p = (int *) malloc(sizeof(int));
    *p = 10;
}

#pragma endregion

#pragma region 5

void five()
{
    int *data = (int *) malloc(100 * sizeof(int));
    data[100] = 0;
    free(data);
}

#pragma endregion

#pragma region 6

void six()
{
    int *data = (int *) malloc(100 * sizeof(int));
    data[50] = 50;
    free(data);
    printf("access inavailable data %d\n", data[50]);
}

#pragma endregion

#pragma region 7

void seven()
{
    int *data = (int *) malloc(100 * sizeof(int));
    data[50] = 50;
    free((data + 20));
    printf("access inavailable data %d\n", data[50]);
}

#pragma endregion

int main(int argc, char *argv[])
{
    // one();
    // four();
    // five();
    // six();
    seven();
    return 0;    
}