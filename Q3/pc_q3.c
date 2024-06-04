
//produtor: clientes //consumidor: Banco
#define _XOPEN_SOURCE 600
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <pthread.h>
#include <stdbool.h>

#define PEDIDOS_SIZE 10 
#define MONEY_INICIAL 5
#define MAX_ARQ_NAME 50

//===========================================================================

typedef struct Conta{
    int id;
    int money;
} Conta; 

typedef struct Pedidos{
    bool valid;
    int OP;
    Conta* Zerada;
} Pedidos;

typedef struct fileThings{
    int Ncliente;
    int Ncontas;
    char** arqDosClientes;
} fileThings;

typedef struct InfoThread{
    int tid;
    int Nclients;
    int Ncontas;
    char** arqCliente;
} InfoThread;

typedef struct InfoThreadBanco{
    int Nclients;
    int Ncontas;
    Conta* contas;
} InfoThreadBanco;

Pedidos listaPedidos[PEDIDOS_SIZE];
int items = 0;
int first = 0;
int last = 0;
int ClientesTerminaram=0;

//===========================================================================

pthread_barrier_t barreira;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER; 
pthread_mutex_t mutexPrint = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t empty = PTHREAD_COND_INITIALIZER; 
pthread_cond_t fill = PTHREAD_COND_INITIALIZER; 

//===========================================================================

void put(Pedidos newPedido); //para o produtor
Pedidos* get(InfoThreadBanco* info); //para o consumidor

void *producerClientes(void* arg); //o numero de clientes
void *consumerBanco(void* arg); //apenas um

fileThings* abreArquivo();
void pedidoPrint(int id, Pedidos* ped);

//===========================================================================

int main(){
    fileThings* infoClientes = abreArquivo();

    //arrumando a variavel Global listaPedidos
    for(int i=0; i<PEDIDOS_SIZE; i++){
        listaPedidos[i].OP=0;
        listaPedidos[i].valid=false;
        listaPedidos[i].Zerada = (Conta*)malloc(sizeof(Conta));
        listaPedidos[i].Zerada->id = 0;
        listaPedidos[i].Zerada->money = 0;
    }

    pthread_barrier_init(&barreira, NULL, infoClientes->Ncliente+2);

    pthread_t producer_threads[infoClientes->Ncliente];
    pthread_t consumer_thread;

    InfoThread* prod_infos = (InfoThread*)malloc(sizeof(InfoThread)*infoClientes->Ncliente);
    InfoThreadBanco* cons_info = (InfoThreadBanco*)malloc(sizeof(InfoThreadBanco));

    //produtores
    for(int i=0; i<infoClientes->Ncliente; i++){
        prod_infos[i].tid = i; 
        prod_infos[i].Nclients = infoClientes->Ncliente; 
        prod_infos[i].Ncontas = infoClientes->Ncontas; 
        prod_infos[i].arqCliente = &infoClientes->arqDosClientes[i]; 
        pthread_create(&producer_threads[i], NULL, producerClientes, (void*)&prod_infos[i]);
    }

    //consumidor
    cons_info->Nclients = infoClientes->Ncliente; 
    cons_info->Ncontas = infoClientes->Ncontas;
    cons_info->contas = (Conta*)malloc(sizeof(Conta)*infoClientes->Ncontas); 
    for(int i=0; i<cons_info->Ncontas; i++){
        cons_info->contas[i].id = i; 
        cons_info->contas[i].money = MONEY_INICIAL;
    }
    pthread_create(&consumer_thread, NULL, consumerBanco, (void*) &cons_info);

    //esperando as threads acabarem
    pthread_barrier_wait(&barreira);
    pthread_barrier_destroy(&barreira);

    //desalocando
    for(int i=0; i<infoClientes->Ncliente; i++)
        free(infoClientes->arqDosClientes[i]);
    free(infoClientes->arqDosClientes);
    free(cons_info->contas);
    free(cons_info);
    free(prod_infos);
    for(int i=0; i<PEDIDOS_SIZE; i++)
        free(listaPedidos[i].Zerada);

    pthread_exit(NULL);
}

//===========================================================================

fileThings* abreArquivo(){
    fileThings* arqCoisas = (fileThings*)malloc(sizeof(fileThings));
    FILE* arqEntrada = fopen("pc_q3entrada.txt","r");
    if(arqEntrada == NULL) exit(1);
    
    fscanf(arqEntrada, " %d", &arqCoisas->Ncontas);
    fscanf(arqEntrada, " %d", &arqCoisas->Ncliente);

    arqCoisas->arqDosClientes = (char**) malloc(sizeof(char*)*arqCoisas->Ncliente);

    for(int i=0; i<arqCoisas->Ncliente; i++){ 
        arqCoisas->arqDosClientes[i] = (char*)malloc(sizeof(char)*MAX_ARQ_NAME);
        fscanf(arqEntrada, " %[^\n]s", arqCoisas->arqDosClientes[i]);
    }

    fclose(arqEntrada);
    return arqCoisas;
}

void pedidoPrint(int id, Pedidos* ped){
    pthread_mutex_lock(&mutexPrint);
    printf("Pedido %d\n", id);
    printf("\tOperacao %d, Valido %d\n", ped->OP, ped->valid);
    printf("\tConta %d: R$%d\n", ped->Zerada->id, ped->Zerada->money);
    pthread_mutex_unlock(&mutexPrint);
}

//===========================================================================

void *producerClientes(void* arg){
    /*Cada thread irá pegar as informações de seus arquivos 
    e tentar colocar cada pedido na listaPedidos por meio da função put(i)*/
    InfoThread* info = ((InfoThread*) arg); 
    pthread_mutex_lock(&mutexPrint);
    printf("Cliente %d: %s\n", info->tid, (*info->arqCliente)); 
    pthread_mutex_unlock(&mutexPrint);

    FILE* arq = fopen((*info->arqCliente), "r");
    //nao conseguiu abrir o arquivo
    if(arq == NULL) { 
        ClientesTerminaram++;
        pthread_exit(NULL);
    }

    int buffer; 
    Pedidos pedidoNovo; 
    pedidoNovo.Zerada = (Conta*)malloc(sizeof(Conta));
    // printf("Qual operação deseja Realizar?\n1...Depósito\n2...Saque\n3...Consulta de Saldo\n");
    while(fscanf(arq, " %d", &buffer) != EOF){
        //o primeiro valor eh a operação
        pedidoNovo.OP = buffer;
        pedidoNovo.valid = false; 
        // printf("Qual o id da conta que você quer acessar?\n");
        fscanf(arq, " %d", &buffer);
        pedidoNovo.Zerada->id = buffer; 
        switch (pedidoNovo.OP){
            case 1: 
                // printf("Quanto Deseja depositar?\n"); 
                fscanf(arq, " %d", &pedidoNovo.Zerada->money);
                if(buffer < info->Nclients){ 
                    pedidoNovo.valid = 1; 
                } else { 
                    pedidoNovo.valid = 0; 
                    // printf("Esse não é um número de conta válido\n"); 
                }
                break;
            case 2: 
                // printf("Quanto Deseja Sacar?\n");
                fscanf(arq, " %d", &pedidoNovo.Zerada->money);
                if(buffer < info->Nclients){
                    pedidoNovo.valid = 1;
                } else {
                    pedidoNovo.valid = 0;
                    // printf("Esse não é um número de conta válido\n"); 
                }
                break;
            case 3:
                pedidoNovo.Zerada->money = 0;
                if(buffer < info->Nclients){
                    pedidoNovo.valid = 1;
                } else {
                    pedidoNovo.valid = 0;
                    // printf("Esse não é um número de conta válido\n"); 
                }
                break;
            default: 
                pedidoNovo.valid = 0;
                // printf("Não é um número válido, tente novamente\n");
                break;
        }
        //poe na lista de Pedidos
        pedidoPrint(info->tid, &pedidoNovo);
        put(pedidoNovo);
    }
    
    free(pedidoNovo.Zerada);
    fclose(arq);
    ClientesTerminaram++;

    pthread_mutex_lock(&mutexPrint);
    printf("Cliente %i terminou\n", info->tid);
    pthread_mutex_unlock(&mutexPrint);

    pthread_barrier_wait(&barreira);
    pthread_exit(NULL);
}

//===========================================================================

void put(Pedidos newPedido){
    pthread_mutex_lock(&mutex);
    while(items == PEDIDOS_SIZE) {
        pthread_cond_wait(&empty, &mutex);
    }

    //colocar pedidos dos clientes
    listaPedidos[last].OP = newPedido.OP;
    listaPedidos[last].valid = newPedido.valid; 
    listaPedidos[last].Zerada = newPedido.Zerada; 
    //printf("pos %d: ", last);

    items++; 
    last++;
    if(last==PEDIDOS_SIZE) { last = 0; } 
    if(items == 1) { 
        pthread_cond_signal(&fill); 
    }
    
    pthread_mutex_unlock(&mutex); 
}

//===========================================================================

void *consumerBanco(void* arg){
    InfoThreadBanco* info = (InfoThreadBanco*) arg;
    /*Essa thread irá pegar os pedidos, 
    realizá-los e tirá-los da listaPedidos*/
    Pedidos* v;
    int id=0;
    pthread_mutex_lock(&mutexPrint);
    printf("Banco iniciou \n");
    pthread_mutex_unlock(&mutexPrint);
    //vai consumir até que os clientes nao tenham mais nada para pedir
    bool end = false;
    while(!end){
        v = get(info);
        pedidoPrint(id, v);
        id = v->Zerada->id;
        pthread_mutex_lock(&mutexPrint);
        printf("Conta %d :", id); 
        if(v->valid) { 
            switch (v->OP) {
                { 
                case 1:
                    info->contas[id].money += v->Zerada->money; 
                    printf("Deposito Realizado\n");
                    break; 
                case 2: 
                    if(v->Zerada->money > info->contas[id].money) 
                        printf("Saldo insuficiente para Saque\n"); 
                    else {
                        info->contas[id].money -= v->Zerada->money; 
                        printf("Saque Realizado\n"); 
                    }
                    break;
                case 3:
                    printf("R$%d disponíveis\n", info->contas[id].money);
                    break;
                default: {}          
                break;
                }
            }
        } 
        else {
            printf("Operação não realizada, informação não são válidas...\n");
        }

        printf("Banco realizou a opearação\n");
        free(v->Zerada);
        free(v);
        pthread_mutex_unlock(&mutexPrint);
        end = (ClientesTerminaram < info->Nclients) ? false : true;
    }

    pthread_mutex_lock(&mutexPrint);
    printf("Banco terminou \n");
    pthread_mutex_unlock(&mutexPrint);

    pthread_barrier_wait(&barreira);
    pthread_exit(NULL);
}

//===========================================================================

Pedidos* get(InfoThreadBanco* info) {
    pthread_mutex_lock(&mutex); 
    while(items == 0){  
        pthread_cond_wait(&fill, &mutex); 
    }

    //realizar pedidos dos clientes
    Pedidos* result = (Pedidos*) malloc(sizeof(Pedidos));
    (*result) = listaPedidos[first];
    (*result).Zerada = (Conta*) malloc(sizeof(Conta));
    (*(*result).Zerada) = (*listaPedidos[first].Zerada);

    //  printf("pos %d: ", first);
    items--;  
    first++; 
    
    if(first==PEDIDOS_SIZE) { first = 0; }
    if(items == PEDIDOS_SIZE - 1){ 
        pthread_cond_broadcast(&empty); 
    }
    
    pthread_mutex_unlock(&mutex);
    //terminou a tarefa
    return result;
}

//===========================================================================
