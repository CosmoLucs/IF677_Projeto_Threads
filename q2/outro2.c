#define _XOPEN_SOURCE 600
#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>

pthread_barrier_t barreira;

struct ArrayInfo{
    int *array;
    int tam;
};
struct threadInfo{
    int threadId;
    int nThreads;
    int begin, end;
    struct ArrayInfo *vetInfo;
    int *subArray;
    int tamSub;
};
