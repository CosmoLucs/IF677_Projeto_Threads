#define _XOPEN_SOURCE 600
#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>


//====================================================

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

//====================================================

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
    struct threadInfo info = (*(struct threadInfo*) arg);
    //comparar qual o menor valor em determinada posição de cada subArray
    //colocá-los no "novo" array de maneira crescente
    //fazer isso com cada posicão
    //aaa

    pthread_barrier_wait(&barreiraM);
}

//====================================================


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

    pthread_barrier_init(&barreiraT, NULL, Nthreads+1);

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
    
    //sincronizando a main e o sort
    pthread_barrier_init(&barreiraM, NULL, 2);
    //thread que junta tudo
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
