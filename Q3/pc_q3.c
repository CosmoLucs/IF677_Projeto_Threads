
//produtor: clientes //consumidor: Banco
#define _XOPEN_SOURCE 600
#include <pthread.h>
#include <stdio.h>
#include <math.h>
#include <stdbool.h>

//===========================================================================

typedef struct Conta{
    int id;
    int money;
} Conta; 

typedef struct Pedidos{
    int qtd;
    int OP;
    Conta* Zerada;
} Pedidos;

pthread_mutex_t mutexScanf = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t empty = PTHREAD_COND_INITIALIZER;
pthread_cond_t fill = PTHREAD_COND_INITIALIZER;

#define MAXpedidos 10

Pedidos listaPedidos[MAXpedidos];
int items = 0;
int first = 0;
int last = 0;


typedef struct ConsumerInfoThread{
    bool valid;
    int tid;
    int Nclients;
    Pedidos* listaPedidos;
} ConsumerInfoThread;

typedef struct ProducerInfoThread{
    int tid;
    int Nclients;
    Pedidos* listaPedidos;
}ProducerInfoThread;

//===========================================================================

void put(int i){
    pthread_mutex_lock(&mutex);
    while(items == MAXpedidos) {
        pthread_cond_wait(&empty, &mutex);
    }
    
    listaPedidos[last].qtd = i;
    //printf("pos %d: ", last);

    items++; 
    last++;
    if(last==MAXpedidos) { last = 0; } 
    if(items == 1) { 
        pthread_cond_signal(&fill); 
    }
    
  pthread_mutex_unlock(&mutex); 
}

//===========================================================================

int get() {
    int result;
    pthread_mutex_lock(&mutex); 
    while(items == 0){  
        pthread_cond_wait(&fill, &mutex); 
    }
    
    result = listaPedidos[first].qtd;
    
    //  printf("pos %d: ", first);
    
    items--;  
    first++;
    
    if(first==MAXpedidos) {
        first = 0; 
    }
    if(items == MAXpedidos - 1){ 
        pthread_cond_broadcast(&empty); 
    }
    
    pthread_mutex_unlock(&mutex);
    return result;
}

//===========================================================================

void *producerClientes(void* arg); //o numero de clientes
void *consumerBanco(void* arg); //apenas um

//===========================================================================

int main(){


    pthread_exit(NULL);
}

//===========================================================================


void *producerClientes(void* arg){
    ProducerInfoThread* dados = ((ProducerInfoThread*) arg);
    int i = 0;
    printf("Produtor\n");
    int amount = MAXpedidos * dados->Nclients;
    // printf("Produtor iniciou\n");
    for(i=0;i<amount; i++) {
        put(i);
        // printf("Produzi %d \n",i);
    }
    // printf("Produtor terminou\n");
    pthread_exit(NULL);
}

//===========================================================================

void *consumerBanco(void* arg){
    int i,v;
    // printf("Consumidor %d iniciou \n", *((int*) threadid));
    for (i=0;i<MAXpedidos;i++) {
        v = get();
        // printf("Consumidor %d consumiu %d \n", *((int*) threadid),v);
    }
    // printf("Consumidor %d terminou \n", *((int*) threadid));
    pthread_exit(NULL);
}

//===========================================================================

