//andrey's version
//ignore
#define _XOPEN_SOURCE 600
#include <pthread.h>
#include <stdio.h>
#include <math.h>
#include <stdbool.h>

//===========================================================================

#define MAXpedidos 100
 
typedef struct {
    int id;
    int Money;
    pthread_mutex_t mutex; 
} Conta;

typedef struct {
    bool valid;
    int Qtd;
    int ThreadId;
    int OP;
    Conta *Zerada; 
} ThreadInfo;

pthread_barrier_t barreira;


//===========================================================================

void* ResolveIsso(void *arg); 

//===========================================================================

int main (int argc, char *argv[]) { 
    int Ncliente, i;
    scanf("%d", &Ncliente);
    

    //declaração
    ThreadInfo Info[Ncliente]; 
    pthread_t Thread[Ncliente]; 
    Conta Pessoa[Ncliente]; 

    pthread_barrier_init(&barreira, NULL, Ncliente+1); 

    for(i = 0; i < Ncliente; i++) {
        pthread_mutex_init(&Pessoa[i].mutex, NULL);
        Pessoa[i].Money = 5;
        Pessoa[i].id = i; 
    }

    int buffer=0;
    for(i = 0; i < Ncliente; i++) {
        // printf("Qual operação deseja Realizar?\n1...Depósito\n2...Saque\n3...Consulta de Saldo\n");
        scanf("%d", &Info[i].OP);
        // printf("Qual o id da conta que você quer acessar?\n");
        switch (Info[i].OP) 
        {
        case 1: 
            scanf("%d", &buffer);
            // printf("Quanto Deseja depositar?\n");
            scanf("%d", &Info[i].Qtd);
            if(buffer < Ncliente){
                Info[i].Zerada = &Pessoa[buffer]; 
                Info[i].valid = 1;
            } else {
                Info[i].valid = 0;
                // printf("Esse não é um número de conta válido\n"); Info[i].valid = 0;
            }
            break;
        case 2: 
            scanf("%d", &buffer);
            // printf("Quanto Deseja Sacar?\n");
            scanf("%d", &Info[i].Qtd);
            if(buffer < Ncliente){
                Info[i].Zerada = &Pessoa[buffer];
                Info[i].valid = 1;
            } else {
                Info[i].valid = 0;
                // printf("Esse não é um número de conta válido\n"); Info[i].valid = 0;
            }
            break;
        case 3:
            scanf("%d", &buffer);
            if(buffer < Ncliente){
                Info[i].Zerada = &Pessoa[buffer];
                Info[i].valid = 1;
            } else {
                Info[i].valid = 0;
                // printf("Esse não é um número de conta válido\n"); Info[i].valid = 0;
            }
            break;
        default: 
            Info[i].valid = 0;
            // printf("Não é um número válido, tente novamente\n");
            break;
        }
    }

    for(i = 0; i < Ncliente; i++) {
        Info[i].ThreadId = i;
        pthread_create(&Thread[i], NULL, ResolveIsso, &Info[i]);
    }

    pthread_barrier_wait(&barreira);
    pthread_barrier_destroy(&barreira);

    for(i = 0; i < Ncliente; i++) {
        printf("Conta %d: R$ %d\n", Pessoa[i].id, Pessoa[i].Money);
    }

    for(int i=0; i<Ncliente; i++)
        pthread_mutex_destroy(&Pessoa[i].mutex);

    pthread_exit(NULL);
}

//===========================================================================

void *ResolveIsso(void *arg) { 
    ThreadInfo *Temp = (ThreadInfo *) arg; 
    printf("Thread %d iniciou\n", Temp->ThreadId);
    if(Temp->valid) {
        switch (Temp->OP) {
            {
            case 1:
                pthread_mutex_lock(&Temp->Zerada->mutex); 
                Temp->Zerada->Money += Temp->Qtd; 
                pthread_mutex_unlock(&Temp->Zerada->mutex); 
                printf("Conta %d: Deposito Realizado\n", Temp->Zerada->id);
                break; 
            case 2: 
                pthread_mutex_lock(&Temp->Zerada->mutex);
                if(Temp->Qtd > Temp->Zerada->Money) 
                    printf("Conta %d: Saldo insuficiente para Saque\n", Temp->Zerada->id); 
                else {
                    Temp->Zerada->Money -= Temp->Qtd; 
                    printf("Conta %d: Saque Realizado\n", Temp->Zerada->id);
                }
                pthread_mutex_unlock(&Temp->Zerada->mutex);
                break;
            case 3:
                pthread_mutex_lock(&Temp->Zerada->mutex);
                printf("Conta %d possui R$%d disponíveis\n", Temp->Zerada->id, Temp->Zerada->Money);
                pthread_mutex_unlock(&Temp->Zerada->mutex);
                break;
            default: {}          
            break;
            }
        }
    } 
    else 
        printf("Conta %d: Operação não realizada, informação não são válidas...\n", Temp->Zerada->id);
    
    printf("Thread %d finanizou\n", Temp->ThreadId);
    pthread_barrier_wait(&barreira);
    pthread_exit(NULL);
}

//===========================================================================
