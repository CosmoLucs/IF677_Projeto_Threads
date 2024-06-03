
//produtor: clientes //consumidor: Banco
#define _XOPEN_SOURCE 600
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <pthread.h>
#include <stdbool.h>

#define MAX_PEDIDOS 10 
#define PEDIDOS_SIZE 10 

//===========================================================================

typedef struct Conta{
    int id;
    int money;
} Conta; 

typedef struct Pedidos{
    bool valid;
    int qtd;
    int OP;
    Conta* Zerada;
} Pedidos;

typedef struct fileThings{
    int Ncliente;
    char** arqDosClientes;
} fileThings;

typedef struct InfoThread{
    int tid;
    int Nclients;
    char** arqCliente;
} InfoThread;

Pedidos listaPedidos[PEDIDOS_SIZE];
int items = 0;
int first = 0;
int last = 0;

//===========================================================================

pthread_barrier_t barreira;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER; 
pthread_cond_t empty = PTHREAD_COND_INITIALIZER; 
pthread_cond_t fill = PTHREAD_COND_INITIALIZER; 

//===========================================================================

void put(int i); //para o produtor
int get(); //para o consumidor

void *producerClientes(void* arg); //o numero de clientes
void *consumerBanco(void* arg); //apenas um

fileThings* abreArquivo();

//===========================================================================

int main(){
    fileThings* infoClientes = abreArquivo();

    pthread_barrier_init(&barreira, NULL, infoClientes->Ncliente+2);

    pthread_t producer_threads[infoClientes->Ncliente];
    pthread_t consumer_thread;

    InfoThread* prod_threads = (InfoThread*)malloc(sizeof(InfoThread)*infoClientes->Ncliente);
    InfoThread* cons_thread = (InfoThread*)malloc(sizeof(InfoThread));

    //consumidor
    cons_thread->tid = 0;
    cons_thread->Nclients = infoClientes->Ncliente;
    cons_thread->arqCliente; //o produtor n precisa do nome dos Clientes
    pthread_create(&cons_thread, NULL, consumerBanco, (void*) &cons_thread);

    //produtores
    for(int i=0; i<infoClientes->Ncliente; i++){
        prod_threads[i].tid = i+1; 
        prod_threads[i].Nclients = infoClientes->Ncliente; 
        prod_threads[i].arqCliente = &infoClientes->arqDosClientes[i]; 
        pthread_create(&producer_threads[i], NULL, producerClientes, (void*)&prod_threads[i]);
    }

    //esperando as threads acabarem
    pthread_barrier_wait(&barreira);
    pthread_barrier_destroy(&barreira);

    //desalocando
    free(cons_thread);
    free(prod_threads);

    pthread_exit(NULL);
}

//===========================================================================

fileThings* abreArquivo(){
    fileThings* arqCoisas = (fileThings*)malloc(sizeof(fileThings));
    FILE* arqEntrada = fopen("pc_q3entrada.txt","r");
    fscanf(arqEntrada, "%d\n", &arqCoisas->Ncliente);

    arqCoisas->arqDosClientes = (char**) malloc(sizeof(char*)*arqCoisas->Ncliente);

    for(int i=0; i<arqCoisas->Ncliente; i++){
        //ver se isso tá certo
        fscanf(arqEntrada, "%c[^\n]", arqCoisas->arqDosClientes[i]);
    }

    fclose(arqEntrada);
    return arqCoisas;
}

//===========================================================================

void *producerClientes(void* arg){
    // ProducerInfoThread* dados = ((ProducerInfoThread*) arg);
    int i = 0;
    printf("Produtor\n");
    /*Cada thread irá pegar as informações de seus arquivos 
    e tentar colocar cada pedido na listaPedidos por meio da função put(i)*/

    for (i=0;i<MAX_PEDIDOS;i++) {
    //printf("Produtor iniciou\n");
        put(i);
        //printf("Produzi %d \n",i); 
    }
    //printf("Produtor terminou\n");
    pthread_barrier_wait(&barreira);
    pthread_exit(NULL);
}

//===========================================================================

void *consumerBanco(void* arg){
    int i,v;
    /*Essa thread irá pegar os pedidos, 
    realizá-los e tirá-los da listaPedidos*/

    // printf("Consumidor %d iniciou \n", *((int*) threadid));
    // int amount = MAX_PEDIDOS * dados->Nclients;
    // for(i=0;i<amount; i++) {
        v = get();
        // printf("Consumidor %d consumiu %d \n", *((int*) threadid),v);
    // }
    // printf("Consumidor %d terminou \n", *((int*) threadid));
    pthread_barrier_wait(&barreira);
    pthread_exit(NULL);
}

//===========================================================================

void put(int i){
    pthread_mutex_lock(&mutex);
    while(items == MAX_PEDIDOS) {
        pthread_cond_wait(&empty, &mutex);
    }

    //colocar pedidos dos clientes
    listaPedidos[last].qtd = i;
    //printf("pos %d: ", last);

    items++; 
    last++;
    if(last==MAX_PEDIDOS) { last = 0; } 
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

    //realizar pedidos dos clientes
    result = listaPedidos[first].qtd;
    
    //  printf("pos %d: ", first);
    
    items--;  
    first++;
    
    if(first==MAX_PEDIDOS) {
        first = 0; 
    }
    if(items == MAX_PEDIDOS - 1){ 
        pthread_cond_broadcast(&empty); 
    }
    
    pthread_mutex_unlock(&mutex);
    return result;
}

//===========================================================================
