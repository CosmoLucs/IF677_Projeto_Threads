#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <pthread.h>

#define PEDIDOS_SIZE 3 
#define MONEY_INICIAL 5 
#define MAX_ARQ_NAME 50 

typedef struct node Node;

typedef struct Pedidos{
    int id; 
    int money; 
    int OP; 
    bool valid; 
} Pedidos; 

typedef struct node{
    Pedidos element;
    Node* next;
} Node;

typedef struct queue{
    Node* front;
    Node* rear;
    int size;
}Queue;

Node* create_node(Pedidos new, Node* nextval);
Queue* create_queue();
void clear_queue(Queue* q);
void enqueue(Queue* q, Pedidos new);
Pedidos dequeue(Queue* q);
Pedidos frontValue(Queue* q);

//=============================================================

typedef struct Conta{
    int id; 
    int money; 
} Conta; 

typedef struct fileThings{
    int Ncliente;
    int Ncontas;
    char** nomes;
} fileThings;

typedef struct InfoCliente{
    int tid;
    fileThings dataC;
} InfoCliente;

typedef struct InfoBanco{
    int Ncliente;
    int Ncontas;
    Pedidos v;
} InfoBanco;

//===========================================================================

//recurso para variavel p
pthread_mutex_t mutexPedidos = PTHREAD_MUTEX_INITIALIZER;
//recurso da variavel ClientesTerminaram
pthread_mutex_t mutexSomar = PTHREAD_MUTEX_INITIALIZER;
//recurso da fila de pedidos
pthread_mutex_t mutexLista = PTHREAD_MUTEX_INITIALIZER; 
//recurso de printar no terminal
pthread_mutex_t mutexPrint = PTHREAD_MUTEX_INITIALIZER;

//indicar para os clientes para fazerem pedidos 
pthread_cond_t empty = PTHREAD_COND_INITIALIZER; 
//indicar para o banco completar os pedidos 
pthread_cond_t fill = PTHREAD_COND_INITIALIZER; 

//fila de pedidos global
Queue* fila = NULL;
//lista de contas global também
Conta* contas = NULL;

//indica a quantidade de pedidos feitos
int p=0;
//indica quantos clientes terminaram suas solicitações
int ClientesTerminaram=0;


//===========================================================================

void put(Pedidos newPedido); //para os produtores
Pedidos get(InfoBanco* info); //para o consumidor

void *producerClientes(void* arg); //o numero de clientes
void *consumerBanco(void* arg); //apenas um

fileThings* abreArquivo();
void pedidoPrint(char* nome, int id, Pedidos* ped);

//=============================================================




int main(){
    int i=0;
    fileThings* infoGeral = abreArquivo();

    //inicializando variaveis globais
    fila = create_queue();
    contas = (Conta*)malloc(sizeof(Conta)*infoGeral->Ncontas);
    for(i=0; i < infoGeral->Ncontas; i++){
        contas[i].id = i; 
        contas[i].money = MONEY_INICIAL;
    }

    //--------------------------------------------------------

    pthread_t proThread[infoGeral->Ncliente];
    pthread_t coThread;

    InfoCliente proInfo[infoGeral->Ncliente];
    InfoBanco coInfo;

    //--------------------------------------------------------

    for(i=0; i<infoGeral->Ncliente; i++){
        proInfo[i].tid = i;
        proInfo[i].dataC.Ncliente = infoGeral->Ncliente;
        proInfo[i].dataC.Ncontas = infoGeral->Ncontas;
        proInfo[i].dataC.nomes = &infoGeral->nomes[i];
    }

    coInfo.Ncliente = infoGeral->Ncliente;
    coInfo.Ncontas = infoGeral->Ncontas;

    //--------------------------------------------------------

    for(i=0; i<infoGeral->Ncliente; i++){
        pthread_create(&proThread[i], NULL, producerClientes, (void*)&proInfo[i]);
    }

    pthread_create(&coThread, NULL, consumerBanco, (void*)&coInfo);

    //--------------------------------------------------------

    for(i=0; i<infoGeral->Ncliente; i++)
        pthread_join(proThread[i], NULL);
    
    pthread_join(coThread, NULL);

    //---------------------------------------------------------

    for(i=0; i<infoGeral->Ncliente; i++){
        free(infoGeral->nomes[i]);
    }
    free(infoGeral->nomes);
    free(infoGeral);

    free(contas);

    clear_queue(fila);

    pthread_exit(NULL);

}




//=============================================================

fileThings* abreArquivo(){
    fileThings* arqCoisas = (fileThings*)malloc(sizeof(fileThings));
    FILE* arqEntrada = fopen("pc_q3entrada.txt","r");
    if(arqEntrada == NULL) exit(1);
    
    fscanf(arqEntrada, " %d", &arqCoisas->Ncontas);
    fscanf(arqEntrada, " %d", &arqCoisas->Ncliente);

    arqCoisas->nomes = (char**) malloc(sizeof(char*)*arqCoisas->Ncliente);

    for(int i=0; i<arqCoisas->Ncliente; i++){ 
        arqCoisas->nomes[i] = (char*)malloc(sizeof(char)*MAX_ARQ_NAME);
        fscanf(arqEntrada, " %[^\n]s", arqCoisas->nomes[i]);
    }

    fclose(arqEntrada);
    return arqCoisas;
}
void pedidoPrint(char* nome, int id, Pedidos* ped){
    printf("\t%s %d\n", nome, id);
    printf("\t\tOperacao %d, Valido %d\n", ped->OP, ped->valid);
    printf("\t\tConta %d: R$%d\n", ped->id, ped->money);
}
void debugCoisas(void* arg){
    printf("- Info 1: %d\n", (int)arg);
}

//=============================================================

void* producerClientes(void* arg){
    InfoCliente* info = ((InfoCliente*) arg);
    pthread_mutex_lock(&mutexPrint);
    printf("Cliente %d: %s\n", info->tid, (*info->dataC.nomes)); 
    pthread_mutex_unlock(&mutexPrint);

    FILE* arq = fopen((*info->dataC.nomes), "r");
    //nao conseguiu abrir o arquivo
    if(arq == NULL) { 
        pthread_mutex_lock(&mutexSomar);
        ClientesTerminaram++; 
        pthread_mutex_unlock(&mutexSomar);
        pthread_exit(NULL);
    }

    //limitar a leitura do arquivo
    int qtd=0;
    fscanf(arq, " %d", &qtd);
    pthread_mutex_lock(&mutexPedidos);
    p+=qtd;
    pthread_mutex_unlock(&mutexPedidos);

    Pedidos novoPedido;
    for(int i=0; i<qtd && !feof(arq); i++){
        // printf("Qual operação deseja Realizar?\n1...Depósito\n2...Saque\n3...Consulta de Saldo\n");
        // printf("Qual o id da conta que você quer acessar?\n"); 
        // printf("Quanto Deseja depositar/sacar/ver?\n"); 

        //adapta isso para que funcione da forma que for escrita no arquivo de cada Cliente
        fscanf(arq, " %d %d %d", &novoPedido.OP, &novoPedido.id, &novoPedido.money);
        //o pedido so vai ser valido se o cliente dizer um id de conta valido
        novoPedido.valid = (novoPedido.id < info->dataC.Ncontas) ? true : false;

        pthread_mutex_lock(&mutexPrint);
        pedidoPrint("Cliente", info->tid, &novoPedido);
        pthread_mutex_unlock(&mutexPrint);
        //poe na fila de Pedidos
        put(novoPedido);
    }

    fclose(arq);
    //para que os produtores nao somem sobrepondo
    pthread_mutex_lock(&mutexSomar);
    ClientesTerminaram++;
    pthread_mutex_unlock(&mutexSomar);


    pthread_mutex_lock(&mutexPrint);
    printf("Cliente %d terminou\n", info->tid);
    pthread_mutex_unlock(&mutexPrint);

    pthread_exit(NULL);
}

void put(Pedidos newPedido){
    pthread_mutex_lock(&mutexLista);
    while((fila->size) == PEDIDOS_SIZE){
        pthread_mutex_lock(&mutexPrint);
        printf("buffer cheio\n");
        pthread_mutex_unlock(&mutexPrint);
        pthread_cond_wait(&empty, &mutexLista);
    }

    enqueue(fila, newPedido);

    if((fila->size) == 1) { pthread_cond_signal(&fill); }

    pthread_mutex_unlock(&mutexLista);
}

//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
//nao está terminando//AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
//esta terminando sem entrar no while (antes que os produtores tenham feito algo)
void* consumerBanco(void* arg){
    InfoBanco* info = ((InfoBanco*) arg);

    int id=0;
    pthread_mutex_lock(&mutexPrint);
    printf("Banco iniciou \n");
    pthread_mutex_unlock(&mutexPrint);

    /* enquanto ainda houver clientes para receber pedidos 
        ou tiver pedidos na lista o consumidor continua */ 

    while((ClientesTerminaram < info->Ncliente || (fila->size) > 0)){
        info->v = get(info);
        id = info->v.id;
        //mexendo na variavel global conta
        if(!info->v.valid) { }
        else {
            switch (info->v.OP)
            {
            case 1:
                contas[id].money += info->v.money; 
                break;
            case 2:
                if(info->v.money > contas[id].money) {}
                else { contas[id].money -= info->v.money; }
                break;
            default:
                break;
            }
        }

        //print das informacoes modificadas
        pthread_mutex_lock(&mutexPrint); 
        printf("Banco realizou a opearação\n");
        printf("\tConta %d :", id); 
        if(!info->v.valid) { 
            printf("Operação não realizada, informação não são válidas...\n");
        }
        else {
            switch (info->v.OP)
            {
            case 1:
                printf("Deposito de R$%d Realizado\n", info->v.money);
                break;
            case 2:
                if(info->v.money > contas[id].money) 
                    printf("Saldo insuficiente para Saque\n"); 
                else {
                    printf("Saque de R$%d Realizado\n", info->v.money); 
                }
                break;
            case 3:
                printf("R$%d disponíveis\n", contas[id].money);
                break;
            default:
                //
                break;
            }
        }
        pedidoPrint("Pedido", id, &info->v);
        pthread_mutex_unlock(&mutexPrint);
    }
    
    pthread_mutex_lock(&mutexPrint);
    printf("Banco terminou \n");
    pthread_mutex_unlock(&mutexPrint);

    pthread_exit(NULL);
}

Pedidos get(InfoBanco* info){
    
    pthread_mutex_lock(&mutexLista);
    Pedidos result; result.valid = false;

    while((fila->size) == 0){
        pthread_mutex_lock(&mutexPrint);
        printf("buffer vazio\n");
        pthread_mutex_unlock(&mutexPrint);
        pthread_cond_wait(&fill, &mutexLista);
    }

    if(ClientesTerminaram < info->Ncliente || (fila->size)>0)
        result = dequeue(fila);

    if((fila->size) == PEDIDOS_SIZE-1) { pthread_cond_broadcast(&empty); }
    pthread_mutex_unlock(&mutexLista);
    return result;
}

//=============================================================

Node* create_node(Pedidos new, Node* nextval){
    Node* node = (Node*) malloc(sizeof(Node));
    node->element = new; 
    node->next = nextval;
    return node;
}

Queue* create_queue(){
    Queue* q = (Queue*) malloc(sizeof(Queue));
    Pedidos r;
    r.id = 0;
    r.OP = 0;
    r.money = 0;
    r.valid = 0;

    q->rear = create_node(r, NULL);
    q->front = q->rear;
    q->size = 0;
    return q;
}

void enqueue(Queue* q, Pedidos new){
    q->rear->next = create_node(new, NULL);
    q->rear = q->rear->next;
    (q->size)++;
}

Pedidos dequeue(Queue* q){
    if(q->size == 0){
        Pedidos r;
        r.id = -1;
        r.valid = false;
        return r;
    }
    Pedidos rem = q->front->next->element;
    Node* temp = q->front->next;
    if(q->front->next == NULL){
        q->rear = q->front;
    }
    q->front->next = q->front->next->next;
    (q->size)--;
    free(temp);
    return rem;
}

Pedidos frontValue(Queue* q){
    Pedidos i = q->front->next->element;
    return i;
}

void clear_queue(Queue* q){
    Pedidos pe;
    while(q->size > 0){
        pe = dequeue(q);
    }
    free(q->front);
    free(q);
}

//=============================================================

