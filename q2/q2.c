#define _XOPEN_SOURCE 600
#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>


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

    //printando os subArrays
    for(int t=0; t<Nthreads; t++){
        for(int i=threadArg[t].begin; i<threadArg[t].end; i++){
            printf("%d ", threadArg[t].subArray[i]);
        }
    }
    printf("\n");


    //sincronizando a main e o sort
    pthread_barrier_init(&barreira, NULL, 2);
    //thread que junta tudo 
    pthread_t threadSort;
    pthread_create(&threadSort, NULL, sortThread, (void*) &threadArg);

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
    int tamSub = info[0].tamSub;
    int tamVet = info[0].vetInfo->tam;
    int nThreads = info[0].nThreads;
    int novoArray[tamVet];
    for(int k=0; k<tamVet; k++){
        int escolhido = __INT_MAX__;
        int indEscolhido = 0;
        //procura o menor entre os subArrays
        for(int i=0; i<nThreads; i++){
            //comparar qual o menor valor em uma posição de cada subArray
            int ind1 = info[i].begin; 
            if(ind1 == info[i].end){
                //nada
            }
            else if(escolhido > info[i].vetInfo->array[ind1]){
                escolhido = info[i].vetInfo->array[ind1];
                indEscolhido = i;
            }
        }
        //colocando o valor escolhido
        novoArray[k] = escolhido;
        info[indEscolhido].begin++;
    }

    for(int i=0; i<tamVet; i++){
        info[0].vetInfo->array[i] = novoArray[i];
    }

    pthread_barrier_wait(&barreira);
    pthread_exit(NULL);
}

