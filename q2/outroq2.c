#define _XOPEN_SOURCE 600
#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>


//====================================================
#include "heap.c"
//====================================================


pthread_barrier_t barreiraT, barreiraM;

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

//===========================================================

void *bubbleThread(void* arg);

void *sortThread(void* arg);

//============================================================


int main(){
    struct ArrayInfo vetCoisas;
    int Nthreads;

    printf("Qual o tamanho do array e a quantidade de particoes?\n");
    scanf("%d %d", &vetCoisas.tam, &Nthreads);
    printf("Qual os elementos do array?\n");
    vetCoisas.array = (int*)malloc(sizeof(int)*vetCoisas.tam);
    for(int i=0; i<vetCoisas.tam; i++)
        scanf("%d", &vetCoisas.array[i]);
    //qtd de threads deve ser menor que a qtd de partições
    if(vetCoisas.tam < Nthreads) return 1;

    pthread_barrier_init(&barreiraT, NULL, Nthreads);

    struct threadInfo thread[Nthreads];
    pthread_t threadVar[Nthreads];

    int cont=0;

    int quant = vetCoisas.tam / Nthreads;

    for(int t=0; t<Nthreads; t++){
        thread[t].threadId = t;
        thread[t].nThreads = Nthreads;
        thread[t].tamSub = quant;
        thread[t].subArray = (int*)malloc(sizeof(int)*vetCoisas.tam);
        thread[t].vetInfo = &vetCoisas;

        thread[t].begin = cont;
        cont += quant;
        thread[t].end = cont;
        pthread_create(&threadVar[t], NULL, bubbleThread, (void *) &thread[t]);
    }
    for(int t=0; t<Nthreads; t++)
        pthread_join(threadVar[t], NULL);
    
    pthread_barrier_wait(&barreiraT);
    pthread_barrier_destroy(&barreiraT);
    
    // sincronizando a main e o sort
    pthread_barrier_init(&barreiraM, NULL, 1);
    // thread que junta tudo
    pthread_t threadSort;
    pthread_create(&threadSort, NULL, sortThread, (void*) &thread);
    //pthread_join(threadSort, NULL);

    pthread_barrier_wait(&barreiraM);
    pthread_barrier_destroy(&barreiraM);

    //array ordenado
    for(int i=0; i<vetCoisas.tam; i++)
        printf("%d", vetCoisas.array[i]);
    printf("\n");

    //free
    free(vetCoisas.array);
    for(int t=0; t<Nthreads; t++)
        free(thread[t].subArray);

    pthread_exit(NULL);

}

//===========================================================

void *bubbleThread(void* arg){
    struct threadInfo info = (*(struct threadInfo*) arg);
    for(int i=info.begin; i<info.end; i++){
        for(int j=i; j<info.end-1; j++){
            if(info.subArray[j] > info.subArray[j+1]){
                printf("Thread %d trocou\n", info.threadId);
                int temp = info.subArray[j];
                info.subArray[j] = info.subArray[j+1];
                info.subArray[j+1] = temp;
            }
        }
    }
    pthread_barrier_wait(&barreiraT);
    pthread_exit(NULL);
}

void *sortThread(void* arg){
    struct threadInfo *info = (struct threadInfo*) arg;
    //comparar qual o menor valor em determinada posição de cada subArray
    //colocá-los no "novo" array de maneira crescente
    //fazer isso com cada posicão
    //usar uma heap?

    int k=0;
    minHeap *heap = _minHeapCreate(info[0].vetInfo->tam);
    for(int i=0; i<info[0].tamSub; i++){
        for(int j=0; j<info[0].nThreads; j++){
            int subI = info[j].begin+i; 
            _minHeapInserir(heap, info[j].subArray[subI]); 
        }
        for(int j=0; j<info[0].nThreads; j++){
            info[0].vetInfo->array[k] = _minHeapPop(heap); 
            k++; 
        }
    }

    // pthread_barrier_wait(&barreiraM);
    pthread_exit(NULL);
}
