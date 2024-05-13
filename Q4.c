#define _XOPEN_SOURCE 600
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>
typedef struct {
    bool Valid;
    int index;
    int vector[9];
}ThreadInfo;

pthread_barrier_t barrier;
pthread_t ThreadsLinha[9];
pthread_t ThreadsColuna[9];
int Sudoku[9][9];
ThreadInfo *Info[18];

void* TesteLinha(void *arg) {
    ThreadInfo *Temp = (ThreadInfo *) arg;
    int i;
    for(i = 0; i < 9; i++) {
        Temp->vector[Sudoku[Temp->index][i] - 1]++;
    }

    for(i = 0; i < 9; i++) {
        if(Temp->vector[i] == 0 || Temp->vector[i] > 1) {
            pthread_exit((void *)false);
        }
    }
    pthread_exit((void *)true);
}

void* TesteColuna(void * arg) {
    ThreadInfo *Temp = (ThreadInfo *) arg;
    int i;
    for(i = 0; i < 9; i++) {
        Temp->vector[Sudoku[i][Temp->index] - 1]++;
    }

    for(i = 0; i < 9; i++) {
        if(Temp->vector[i] == 0 || Temp->vector[i] > 1) {
            pthread_exit((void *)false);
        }
    }
    pthread_exit((void *)true);
}

int main(void *argc, void *argv[]) {
    int i, j;
    bool Resposta[18], Respondido = false;
    for(i =0; i < 9; i++) {
        for(j = 0; j <9; j++) {
            scanf("%d", &Sudoku[i][j]);
        }
    }

    pthread_barrier_init(&barrier, NULL, 18);
    
    for(i = 0; i < 9; i++) {
        Info[i] = malloc(sizeof(ThreadInfo));
        Info[i + 9] = malloc(sizeof(ThreadInfo));

        Info[i]->index = i;
        Info[i]->Valid = false;
        for(j = 0; j<9; j++) {
            Info[i]->vector[j] = 0;
        }

        Info[i + 9]->index = i;
        Info[i + 9]->Valid = false;
        for(j = 0; j<9; j++) {
            Info[i + 9]->vector[j] = 0;
        }

        pthread_create(&ThreadsLinha[i], NULL, TesteLinha,(void *)Info[i]);
        pthread_create(&ThreadsColuna[i], NULL, TesteColuna, (void *)Info[i + 9]);
    }

    for(i = 0; i < 9; i++) {
        void *rst;
        pthread_join(ThreadsLinha[i], &rst);
        Resposta[i] = (bool) rst;
        void* rst2;
        pthread_join(ThreadsColuna[i], &rst2);
        Resposta[i + 9] = (bool) rst2;
    }

    pthread_barrier_wait(&barrier);

    for(i = 0; i < 18; i++) {
        if(Resposta[i] == false) {
            if(!Respondido) {
                if(i < 9) {
                    printf("Por causa da linha %d, essa não é uma respota válida\n", i+1);
                    Respondido = true;
                } else {
                    printf("Por causa da coluna %d, essa não é uma respota válida\n", i-8);
                    Respondido = true;
                }
            }
        }
    }

    if(!Respondido) {
        printf("Essa é uma resposta válida! Parabéns, você completou o sudoku!\n");
    }

    pthread_barrier_destroy(&barrier);
}