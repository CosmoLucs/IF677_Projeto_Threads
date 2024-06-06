#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <pthread.h>

#define NUM_CONTAS 5 
#define NUM_CLIENTES 5 
#define PEDIDOS_SIZE 7 

//numero máximo de leitura de pedidos que o banco vai fazer
#define MAX_PEDIDOS 16

#define MONEY_INICIAL 5 
#define MAX_ARQ_NAME 50 

typedef struct node Node;

typedef struct Pedidos{
    int id; 
    int money; 
    int OP; 
    bool valid; 
} Pedidos; 

//=============================================================

typedef struct Conta{
    int id; 
    int money; 
} Conta; 

typedef struct InfoCliente{
    int tid;
    char* nome;
    Conta* zerada;
} InfoCliente;


//===========================================================================

//recurso da fila de pedidos
pthread_mutex_t mutexLista = PTHREAD_MUTEX_INITIALIZER; 
//recurso de printar no terminal
pthread_mutex_t mutexPrint = PTHREAD_MUTEX_INITIALIZER;

//recurso para variavel p
pthread_mutex_t mutexPedidos = PTHREAD_MUTEX_INITIALIZER;
//indica a quantidade de pedidos feitos
int pTotal=0; 
int p=0; 

//recurso da variavel ClientesTerminaram
pthread_mutex_t mutexTermino = PTHREAD_MUTEX_INITIALIZER;
//indica quantos clientes terminaram suas solicitações
int ClientesTerminaram=0;

//indicar para os clientes para fazerem pedidos 
pthread_cond_t empty = PTHREAD_COND_INITIALIZER; 
//indicar para o banco completar os pedidos 
pthread_cond_t fill = PTHREAD_COND_INITIALIZER; 
int items=0;
int first=0;
int last=0;

//lista de contas global também
InfoCliente clientes[NUM_CLIENTES];
Pedidos pedidos[PEDIDOS_SIZE];
Conta contas[NUM_CONTAS];



//===========================================================================

void put(Pedidos newPedido); //para os produtores
Pedidos get(); //para o consumidor

void *producerClientes(void* arg); //o numero de clientes
void *consumerBanco(void* arg); //apenas um

char** abreArquivo();
void pedidoPrint(char* nome, char* id, Pedidos* ped);

//=============================================================




int main(){
    int i=0;
    char** nomes = abreArquivo();

    //inicializando variaveis globais
    for(i=0; i < NUM_CONTAS; i++){
        contas[i].id = i; 
        contas[i].money = MONEY_INICIAL;
    }

    //--------------------------------------------------------

    pthread_t proThread[NUM_CLIENTES];
    pthread_t coThread;
    
    //--------------------------------------------------------

    for(i=0; i<NUM_CLIENTES; i++){
        pthread_create(&proThread[i], NULL, producerClientes, (void*) nomes[i]);
    }
    pthread_create(&coThread, NULL, consumerBanco, NULL);

    //--------------------------------------------------------

    for(i=0; i<NUM_CLIENTES; i++){
        pthread_join(proThread[i], NULL);
    }
    pthread_join(coThread, NULL);

    //---------------------------------------------------------

    for(i=0; i<NUM_CLIENTES; i++){
        free(nomes[i]);
    }
    free(nomes);

    pthread_exit(NULL);

}




//=============================================================

char** abreArquivo(){
    char** arqNomes = (char**)malloc(sizeof(char*));
    FILE* arqEntrada = fopen("entrada.txt","r");
    if(arqEntrada == NULL) exit(1);
    
    arqNomes = (char**) malloc(sizeof(char*)*NUM_CLIENTES);

    for(int i=0; i<NUM_CLIENTES; i++){ 
        arqNomes[i] = (char*)malloc(sizeof(char)*MAX_ARQ_NAME);
        fscanf(arqEntrada, " %[^\n]s", arqNomes[i]);
    }

    fclose(arqEntrada);
    return arqNomes;
}
void pedidoPrint(char* nome, char* id, Pedidos* ped){
    printf("\t%s %s\n", nome, id);
    printf("\t\tOperacao %d, Valido %d\n", ped->OP, ped->valid);
    printf("\t\tConta %d: R$%d\n", ped->id, ped->money);
}

//=============================================================

void* producerClientes(void* arg){
    char* arqNome = ((char*) arg); 
    char* nome = (char*)malloc(sizeof(char)*MAX_ARQ_NAME); 
    int cnt=0;

    //printar bonitinho o nome de cada cliente
    pthread_mutex_lock(&mutexPrint); 
    printf("Cliente "); 
    for(int i = 4; i < ((strlen(arqNome)) - 4); i++){
        nome[cnt] = arqNome[i];
        cnt++;
    }
    nome[cnt+1] = '\0';
    printf("%s iniciou\n", nome);
    pthread_mutex_unlock(&mutexPrint);

    FILE* arq = fopen((arqNome), "r");
    //nao conseguiu abrir o arquivo
    if(arq == NULL) { 
        pthread_mutex_lock(&mutexTermino); 
        ClientesTerminaram++; 
        pthread_mutex_unlock(&mutexTermino); 
        pthread_exit(NULL);
    }

    //limitar a leitura do arquivo
    int qtd=0;
    fscanf(arq, " %d", &qtd); 

    //calcular o total de Pedidos
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
        novoPedido.valid = (novoPedido.id < NUM_CLIENTES) ? true : false;

        // pthread_mutex_lock(&mutexPrint);
        // pedidoPrint("Cliente", nome, &novoPedido);
        // pthread_mutex_unlock(&mutexPrint);
        //poe na fila de Pedidos
        put(novoPedido);
    }

    fclose(arq);

    //para que os produtores nao somem sobrepondo
    pthread_mutex_lock(&mutexTermino);
    ClientesTerminaram++;
    pthread_mutex_unlock(&mutexTermino);

    pthread_mutex_lock(&mutexPrint);
    printf("Cliente %s terminou\n", nome);
    pthread_mutex_unlock(&mutexPrint);

    free(nome);
    pthread_exit(NULL);
}

void put(Pedidos newPedido){
    pthread_mutex_lock(&mutexLista);
    while(items == PEDIDOS_SIZE){
        // pthread_mutex_lock(&mutexPrint);
        // printf("buffer cheio\n");
        // pthread_mutex_unlock(&mutexPrint);
        pthread_cond_wait(&empty, &mutexLista);
    }

    pedidos[last] = newPedido;
    char id = last;
    pthread_mutex_lock(&mutexPrint);
    pedidoPrint("teste", &id, &pedidos[last]);
    pthread_mutex_unlock(&mutexPrint);

    items++;
    last++;
    if(last==PEDIDOS_SIZE) { last = 0; } 
    if(items == 1) { pthread_cond_signal(&fill); }

    pthread_mutex_unlock(&mutexLista);
}

void* consumerBanco(void* arg){ 
    int id=0; 
    pthread_mutex_lock(&mutexPrint); 
    printf("Banco iniciou \n"); 
    pthread_mutex_unlock(&mutexPrint); 

    pthread_mutex_lock(&mutexTermino);
    bool certeza = (ClientesTerminaram < NUM_CLIENTES);
    pthread_mutex_unlock(&mutexTermino);

    //modificar a condicao de parada 
    for(int i=0; i<MAX_PEDIDOS; i++){
        //deixa de receber pedidos se n tiver mais nenhum pedido e nenhum cliente para adcicionar pedidos
        pthread_mutex_lock(&mutexTermino);
        if(ClientesTerminaram == NUM_CLIENTES && items==0) break;
        pthread_mutex_unlock(&mutexTermino);

        Pedidos v = get(); 
        id = v.id; 
        //mexendo na variavel global conta 
        if(v.valid) {
            switch (v.OP)
            {
            case 1:
                contas[id].money += v.money; 
                break;
            case 2:
                if(v.money > contas[id].money) {}
                else { contas[id].money -= v.money; }
                break;
            default:
                break;
            }
        }

        //print das informacoes modificadas 
        pthread_mutex_lock(&mutexPedidos);
            int falta = p - pTotal;
        pthread_mutex_unlock(&mutexPedidos);

        pthread_mutex_lock(&mutexPrint); 
        printf("Banco está realizando a opearação %d\nFaltam %d pedidos e %d clientes\n", 
                pTotal+1, falta, NUM_CLIENTES-ClientesTerminaram); 
        pTotal++; 
        printf("\tConta %d :", id); 
        if(!v.valid) { 
            printf("Operação não realizada, informação não são válidas...\n");
        } 
        else { 
            switch (v.OP)
            { 
            case 1: 
                printf("Deposito de R$%d Realizado\n", v.money);
                break;
            case 2: 
                if(v.money > contas[id].money) 
                    printf("Saldo insuficiente para Saque\n"); 
                else 
                    printf("Saque de R$%d Realizado\n", v.money); 
                break;
            case 3: 
                printf("R$%d disponíveis\n", contas[id].money);
                break;
            default:
                break;
            }
        }
        char num = id;
        pedidoPrint("Pedido", &num, &v); 
        pthread_mutex_unlock(&mutexPrint); 

        //verificar se ainda precisa esperar
        pthread_mutex_lock(&mutexTermino); 
        certeza = (ClientesTerminaram < NUM_CLIENTES); 
        pthread_mutex_unlock(&mutexTermino); 
    }
    
    pthread_mutex_lock(&mutexPrint);
    printf("Banco terminou \n");
    pthread_mutex_unlock(&mutexPrint);

    pthread_exit(NULL);
}

Pedidos get(){
    pthread_mutex_lock(&mutexLista);
    Pedidos result; result.valid = false;

    while(items == 0){
        // pthread_mutex_lock(&mutexPrint);
        // printf("buffer vazio\n");
        // pthread_mutex_unlock(&mutexPrint);
        pthread_cond_wait(&fill, &mutexLista);
    }

    result = pedidos[first];

    items--;
    first++;
    if(first == PEDIDOS_SIZE) { first = 0; }
    if(items == PEDIDOS_SIZE-1) { pthread_cond_broadcast(&empty); }
    pthread_mutex_unlock(&mutexLista);

    return result;
}

//=============================================================
