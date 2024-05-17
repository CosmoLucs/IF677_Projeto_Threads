#ifndef _HEAP_
#define _HEAP_

#include <stdio.h>
#include <stdlib.h>


typedef struct minHeap{
    int *vetor;
    int size, cnt;
} minHeap;

minHeap* _minHeapCreate(int size);
minHeap* _minHeapDestroy(minHeap* H);

void _minHeapfyTop(minHeap* H);
void _minHeapfyBot(minHeap* H);

void _minHeapInserir(minHeap* H, int value);
int _minHeapPop(minHeap* H);

#endif