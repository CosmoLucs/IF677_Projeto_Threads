#define _XOPEN_SOURCE 600
#include <pthread.h>
#include <stdio.h>
#include <math.h>

typedef struct {
    int OP;
    int Qtd;
    int id;
}ThreadInfo;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_barrier_t barrier;

void *Operations(void* arg); //Op

int conta = 0;

int main(int argc, char *arcv[]) {
    int i, Nclientes;
    scanf("%d", &Nclientes);
    pthread_t Threads[Nclientes];
    ThreadInfo Info[Nclientes];

    for(i = 0; i < Nclientes; i++) {
        Info[i].id = i;
        printf("Qual operação deseja Realizar?\n1...Depósito\n2...Saque\n3...Consulta de Saldo\n");
        scanf("%d", &Info[i].OP);
        switch (Info[i].OP)
        {
        case 1:
            printf("Quanto Deseja depositar?\n");
            scanf("%d", &Info[i].Qtd);
            break;
        case 2: 
            printf("Quanto deseja Sacar?\n");
            scanf("%d", &Info[i].Qtd);
            break;
        case 3:
            break;
        default: 
            printf("Não é um número válido, tente novamente\n");
            break;
        }
    }

    for(i = 0; i < Nclientes; i++) {
        pthread_create(&Threads[i], NULL, Operations, &Info[i]);
    }

    for(i = 0; i < Nclientes; i++) {
        pthread_join(Threads[i],NULL);
    }
}

void *Operations(void *arg) {
    ThreadInfo *Temp = (ThreadInfo *) arg;
    pthread_mutex_init(&mutex, NULL);
    switch (Temp->OP) {
        {
        case 1:
            conta += Temp->Qtd;
            printf("Thread %d: Deposito Realizado\n", Temp->id);
            break;
        case 2: 
            if(Temp->Qtd > conta) printf("Thread %d: Saldo insuficiente para Saque\n", Temp->id); else {
                conta -= Temp->Qtd; printf("Thread %d: Saque Realizado\n", Temp->id);}
            break;
        case 3:
            printf("Thread %d: A conta possui R$%d disponíveis", Temp->id, conta);
        default: {}          
        break;
        }
    pthread_mutex_destroy(&mutex);
    }
    pthread_exit(NULL);
}