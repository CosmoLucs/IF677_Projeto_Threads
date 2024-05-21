#define _XOPEN_SOURCE 600
#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>

//====================================================
#include "heap.h"
//====================================================

/*
Nthreads é o N, a quantidade de threads e a quantidade que vai particionar o array.
*/

pthread_barrier_t barreira;

struct ArrayInfo{
    int tam;
    int *array;
};
struct threadInfo{
    int threadId;
    int nThreads;
    int begin, end;
    int tamSub;
    int *subArray;
    struct ArrayInfo *vetInfo;
};

//====================================================

void *bubbleThread(void *arg);
void *sortThread(void *arg);

void *sort2Thread(void* arg);

//====================================================

int main(){
    //struct para as informações do array
    struct ArrayInfo vetorCoisas;
    int Nthreads;

    //únicas entradas do código
    printf("Qual o tamanho do array e a quantidade de particoes?\n");
    scanf("%d %d", &vetorCoisas.tam, &Nthreads);
    printf("Quais os elementos do array?\n");
    vetorCoisas.array = (int*) malloc(sizeof(int)*vetorCoisas.tam);
    for(int i=0; i<vetorCoisas.tam; i++){ 
        scanf("%d", &vetorCoisas.array[i]);
    }
    //a qtd de threads deve ser menor que a qtd de elementos do array
    if(vetorCoisas.tam < Nthreads) return 1;

    //barreira iniciada
    pthread_barrier_init(&barreira, NULL, Nthreads+1);

    //struct que vai ser mandada para cada thread
    struct threadInfo threadArg[Nthreads];
    pthread_t threadVar[Nthreads];

    //variavel para indicar os indices do array
    int cont=0;
    //quantidade de elementos do array colocado em cada thread
    int quant = vetorCoisas.tam / Nthreads;

    for(int t=0; t<Nthreads; t++){
        //todas as threads apontam para a posição em que as informações do array está
        threadArg[t].threadId = t;
        threadArg[t].nThreads = Nthreads;
        threadArg[t].tamSub = quant;
        threadArg[t].subArray = (int*)malloc(sizeof(int)*vetorCoisas.tam);
        threadArg[t].subArray = vetorCoisas.array;
        threadArg[t].vetInfo = &vetorCoisas;
        //colocando as partes do array em cada thread 
        threadArg[t].begin = cont;
        cont += quant;
        threadArg[t].end = cont;
        pthread_create(&threadVar[t], NULL, bubbleThread, (void*) &threadArg[t]);
    }
    
    pthread_barrier_wait(&barreira);
    //barreira finalizada
    pthread_barrier_destroy(&barreira);

    //sincronizando a main e o sort
    pthread_barrier_init(&barreira, NULL, 2);
    //thread que junta tudo 
    pthread_t threadSort;
    pthread_create(&threadSort, NULL, sort2Thread, (void*) &threadArg);

    pthread_barrier_wait(&barreira);
    pthread_barrier_destroy(&barreira);

    //array ordenado
    for(int i=0; i<vetorCoisas.tam; i++){
        printf("%d ", vetorCoisas.array[i]);
    }
    printf("\n");

    free(vetorCoisas.array);
    //quando eu tento dar free nisso aqui, dá um erro chamado "double free"
    // for(int i=0; i<Nthreads; i++)
    //     free(threadArg[i].subArray);
    pthread_exit(NULL);
}



//====================================================

void *bubbleThread(void *arg){
    struct threadInfo info = (*(struct threadInfo *) arg);
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
    pthread_barrier_wait(&barreira);
    pthread_exit(NULL);
}

void *sortThread(void *arg){
    //todas as threads e seus subArrays
    struct threadInfo *info = (struct threadInfo*) arg;
    //comparar qual o menor valor em determinada posição de cada subArray
    //colocá-los no "novo" array de maneira crescente
    //fazer isso com cada posicão

    int k=1;
    int ultimo=__INT_MAX__;
    for(int j=0; j<info[0].tamSub && k<info[0].vetInfo->tam; j++){
        for(int i=0; i<info[0].nThreads; i++){
            int subI=info[i].begin+j; 
            //int subF=info[i].end;
            //o ultimo adicionado eh maior do que o atual //trocam de pos
            if(ultimo > info[i].subArray[subI]){
                //a pos anterior recebe o atual
                info[i].vetInfo->array[k-1] = info[i].subArray[subI];
                //a pos atual recebe o ultimo
                info[i].vetInfo->array[k] = ultimo;
                //o atual vira o novo ultimo
                ultimo = info[i].subArray[subI];
                k++;
            }
            //o ultimo nao é maior ent só adiciona o valor atual na pos atual
            else {
                info[i].vetInfo->array[k] = info[i].subArray[subI];
                k++;
            }
        }
    }

    pthread_exit(NULL);
}

//====================================================

void *sort2Thread(void* arg){
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
    heap = _minHeapDestroy(heap);
    pthread_barrier_wait(&barreira);
    pthread_exit(NULL);
}

//====================================================

minHeap* _minHeapCreate(int size){
    minHeap *H = (minHeap*)malloc(sizeof(minHeap));
    H->vetor = (int*)malloc(sizeof(int)*size+1);
    H->cnt=0;
    H->size=size;
    return H;
}
minHeap* _minHeapDestroy(minHeap* H){
    free(H->vetor);
    free(H);
    return H;
}

void _minHeapfyTop(minHeap* H){
    if(H->cnt/2 >= 1){
        int k=1;
        int v = H->vetor[k];
        bool heap = false;
        while(!heap && 2*k <= H->cnt){
            int j=2*k;
            if(j<H->cnt)
                if(H->vetor[j] > H->vetor[j+1])
                    j=j+1;
            if(v <= H->vetor[j])
                heap = true;
            else{
                H->vetor[k] = H->vetor[j];
                k = j;
            }
            H->vetor[k] = v;
        }
    }
}
void _minHeapfyBot(minHeap* H){
    for(int i = H->cnt/2; i>=1; i--){
        int k = i;
        int v = H->vetor[k];
        bool heap = false;
        while(!heap && 2*k <= H->cnt){
            int j=2*k;
            if(j<H->cnt)
                if(H->vetor[j] > H->vetor[j+1])
                    j++;
            if(v <= H->vetor[j])
                heap = true;
            else{
                H->vetor[k] = H->vetor[j];
                k =j;
            }
            H->vetor[k] = v;
        }
    }
}

void _minHeapInserir(minHeap* H, int value){
    if(H->size == H->cnt){
        //alocar mais memoria
        return;
    }
    H->cnt++;
    H->vetor[H->cnt] = value;
}
int _minHeapPop(minHeap* H){
    if(H->cnt==0) return 0;

    int sub = H->vetor[1];
    H->vetor[1] = H->vetor[H->cnt];
    H->vetor[H->cnt] = sub;

    H->cnt--;
    _minHeapfyTop(H);
    return sub;
}
