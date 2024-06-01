#define _XOPEN_SOURCE 600
#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define NUM_THREADS 4
#define COISA 2
#define NEGOCIO 4

#define NAO_EXECUTADO 0
#define EXECUTADO 1
#define INTERROMPIDO 2
#define EXECUTANDO 3

//o mutex precisa ser global?

typedef struct infoMain{
    int a_rec0, a_rec1, a_rec2;
    pthread_mutex_t *mutex;
    pthread_barrier_t barreira;
} infoMain;

typedef struct infoThread{
    int tid;
    int estado; 
    pthread_t tvar;
    infoMain* infoM;
} infoThread;

//testando os mutex e ocorrencias do deadlock
//exclusão mútua, posse e espera, não preempção, espera circular

//teste 1
//recurso 1 e 2 compartilhado para 4 threads
//thread 1 quer o recurso 1 e 2
//thread 2 quer o recurso 1
//thread 3 quer o recurso 1 e 2
//thread 4 quer o recurso 2


//====================================================================
void *primeiraThread(void*arg){
    infoThread dados = (*(infoThread*) arg);

    pthread_mutex_lock(&dados.infoM->mutex[0]);
    pthread_mutex_lock(&dados.infoM->mutex[1]);
    pthread_mutex_lock(&dados.infoM->mutex[2]);
    printf("Thread %d comecou a executar\n", dados.tid);

    dados.infoM->a_rec0 = COISA+1;
    dados.infoM->a_rec1 = NEGOCIO+1;
    dados.infoM->a_rec2 = NEGOCIO+1;

    printf("Thread %d terminou de executar\n", dados.tid);
    pthread_mutex_unlock(&dados.infoM->mutex[2]);
    pthread_mutex_unlock(&dados.infoM->mutex[1]);
    pthread_mutex_unlock(&dados.infoM->mutex[0]);
    pthread_barrier_wait(&dados.infoM->barreira);
    pthread_exit(NULL);
}
//====================================================================
void *segundaThread(void*arg){
    infoThread dados = (*(infoThread*) arg);
    
    pthread_mutex_lock(&dados.infoM->mutex[0]);
    pthread_mutex_lock(&dados.infoM->mutex[2]);
    printf("Thread %d comecou a executar\n", dados.tid);

    dados.infoM->a_rec0 = NEGOCIO+2;
    dados.infoM->a_rec2 = COISA+2;

    printf("Thread %d terminou de executar\n", dados.tid);
    pthread_mutex_unlock(&dados.infoM->mutex[2]);
    pthread_mutex_unlock(&dados.infoM->mutex[0]);
    pthread_barrier_wait(&dados.infoM->barreira);
    pthread_exit(NULL);
}
//====================================================================
void *terceiraThread(void*arg){
    infoThread dados = (*(infoThread*) arg);

    pthread_mutex_lock(&dados.infoM->mutex[1]);
    pthread_mutex_lock(&dados.infoM->mutex[2]);
    pthread_mutex_lock(&dados.infoM->mutex[0]);
    printf("Thread %d comecou a executar\n", dados.tid);

    dados.infoM->a_rec0 = COISA-1;
    dados.infoM->a_rec1 = NEGOCIO-1;
    dados.infoM->a_rec2 = NEGOCIO-1;

    printf("Thread %d terminou de executar\n", dados.tid);
    pthread_mutex_unlock(&dados.infoM->mutex[0]);
    pthread_mutex_unlock(&dados.infoM->mutex[2]);
    pthread_mutex_unlock(&dados.infoM->mutex[1]);
    pthread_barrier_wait(&dados.infoM->barreira);
    pthread_exit(NULL);
}
//====================================================================
void *quartaThread(void*arg){
    infoThread dados = (*(infoThread*) arg);

    pthread_mutex_lock(&dados.infoM->mutex[1]);
    pthread_mutex_lock(&dados.infoM->mutex[2]);
    printf("Thread %d comecou a executar\n", dados.tid);

    dados.infoM->a_rec1 = NEGOCIO-2;
    dados.infoM->a_rec2 = NEGOCIO-2;

    printf("Thread %d terminou de executar\n", dados.tid);
    pthread_mutex_unlock(&dados.infoM->mutex[2]);
    pthread_mutex_unlock(&dados.infoM->mutex[1]);
    pthread_barrier_wait(&dados.infoM->barreira);
    pthread_exit(NULL);
}


//====================================================================

int main(){
    infoMain infoM;
    infoM.mutex = (pthread_mutex_t*) malloc(sizeof(pthread_mutex_t)*3);
    
    //strcut das threads
    infoThread threads[4];
    for(int i=0; i<4; i++){ 
        threads[i].tid = i;
        threads[i].estado = NAO_EXECUTADO;
        threads[i].infoM = &infoM;
    }

    pthread_barrier_init(&infoM.barreira, NULL, NUM_THREADS+1);


    //terceira thread
    pthread_create(&threads[2].tvar, NULL, terceiraThread, (void*)&threads[2]);
    //primeira thread
    pthread_create(&threads[0].tvar, NULL, primeiraThread, (void*)&threads[0]);
    //segunda thread
    pthread_create(&threads[1].tvar, NULL, segundaThread, (void*)&threads[1]);
    //quarta thread
    pthread_create(&threads[3].tvar, NULL, quartaThread, (void*)&threads[3]);


    //barreira ou o join?
    pthread_barrier_wait(&infoM.barreira);
    pthread_barrier_destroy(&infoM.barreira);

    printf("a_rec0: %d \na_rec1: %d \na_rec2: %d\n", infoM.a_rec0, infoM.a_rec1, infoM.a_rec2);

    pthread_exit(NULL);

    return 0;
}

