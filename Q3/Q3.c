#define _XOPEN_SOURCE 600
#include <pthread.h>
#include <stdio.h>
#include <math.h>
#include <stdbool.h>
 
typedef struct {
    int Money;
    int id;
} Conta;


typedef struct {
    Conta Cleiton;
    int Qtd;
    int ThreadId;
    int OP;
    bool valid;
} ThreadInfo;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

void* ResolveIsso(void *arg);

int main (int argc, char *argv[]) {
    int Ncliente, i;
    scanf("%d", &Ncliente);

    //declaração
    ThreadInfo Info[Ncliente];
    pthread_t Thread[Ncliente];
    Conta Gisele[Ncliente];

    for(i = 0; i < Ncliente; i++) {
        Gisele[i].Money = 5;
        Gisele[i].id = i;
    }

    for(i = 0; i < Ncliente; i++) {
        printf("Qual operação deseja Realizar?\n1...Depósito\n2...Saque\n3...Consulta de Saldo\n");
        scanf("%d", &Info[i].OP);
        switch (Info[i].OP)
        {
        case 1:
            printf("Qual o id da conta que você quer acessar?\n");
            scanf("%d", &Info[i].Cleiton.id);
            printf("Quanto Deseja depositar?\n");
            scanf("%d", &Info[i].Qtd);
            if(Info[i].Cleiton.id < Ncliente){
                Info[i].Cleiton = Gisele[Info[i].Cleiton.id];
                Info[i].valid = 1;
            } else {printf("Esse não é um número de conta válido\n"); Info[i].valid = 0;}
            break;
        case 2: 
            printf("Qual o id da conta que você quer acessar?\n");
            scanf("%d", &Info[i].Cleiton.id);
                printf("Quanto Deseja Sacar?\n");
                scanf("%d", &Info[i].Qtd);
            if(Info[i].Cleiton.id < Ncliente){
                Info[i].Cleiton = Gisele[Info[i].Cleiton.id];
                Info[i].valid = 1;
            } else {printf("Esse não é um número de conta válido\n"); Info[i].valid = 0;}
            break;
        case 3:
            printf("Qual o id da conta que você quer acessar?\n");
            scanf("%d", &Info[i].Cleiton.id);
            if(Info[i].Cleiton.id < Ncliente){
                Info[i].Cleiton = Gisele[Info[i].Cleiton.id];
                Info[i].valid = 1;
            } else {printf("Esse não é um número de conta válido\n"); Info[i].valid = 0;}
            break;
        default: 
            Info[i].valid = 0;
            printf("Não é um número válido, tente novamente\n");
            break;
        }
    }

    for(i = 0; i < Ncliente; i++) {
        Info[i].ThreadId = i;
        pthread_create(&Thread[i], NULL, ResolveIsso, &Info[i]);
    }

    for(i = 0; i < Ncliente; i++) {
        pthread_join(Thread[i], NULL);
    }

    for(i = 0; i < Ncliente; i++) {
        printf("Conta %d: R$%d\n", Info[i].Cleiton.id, Info[i].Cleiton.Money);
    }
    pthread_exit(NULL);
}

void *ResolveIsso(void *arg) {
    ThreadInfo *Temp = (ThreadInfo *) arg;
    if(Temp->valid) {
        switch (Temp->OP) {
            {
            case 1:
                pthread_mutex_lock(&mutex);
                Temp->Cleiton.Money += Temp->Qtd;
                pthread_mutex_unlock(&mutex);
                printf("Thread %d: Deposito Realizado\n", Temp->ThreadId);
                break;
            case 2:
                pthread_mutex_lock(&mutex);
                if(Temp->Qtd > Temp->Cleiton.Money) printf("Thread %d: Saldo insuficiente para Saque\n", Temp->ThreadId); else {
                    Temp->Cleiton.Money -= Temp->Qtd; printf("Thread %d: Saque Realizado\n", Temp->ThreadId);}
                pthread_mutex_unlock(&mutex);
                break;
            case 3:
                pthread_mutex_lock(&mutex);
                printf("Thread %d: A conta %d possui R$%d disponíveis\n", Temp->ThreadId, Temp->Cleiton.id, Temp->Cleiton.Money);
                pthread_mutex_unlock(&mutex);
                break;
            default: {}          
            break;
            }
        }
    } else printf("Thread %d: Operação não realizada, informação não são válidas...\n", Temp->ThreadId);
    pthread_exit(NULL);
}