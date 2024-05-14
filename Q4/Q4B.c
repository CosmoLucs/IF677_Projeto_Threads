#define _XOPEN_SOURCE 600
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>

#define SIZE 9

typedef struct{
    int vector[9];
    int index;
    int matrix[3][3];
}MiniMatrixInfo;

typedef struct {
    bool Valid;
    int index;
    int vector[9];
}ThreadInfo;

pthread_barrier_t barrier;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_t ThreadsLinha[9];
pthread_t ThreadsColuna[9];
pthread_t ThreadMiniMatrix[9];
int Sudoku[9][9];
ThreadInfo InfoLinha[SIZE], InfoColuna[SIZE];
MiniMatrixInfo InfoMiniMatrix[SIZE];

void *TesteLinha(void *arg);
void *TesteColuna(void *arg);
void *TesteMiniMatrix(void *arg);

int main(int argc, int *argv[]) {
    int i, j, k = 0, l;
    bool RespostaLinha[SIZE] = { false }, RespostaColuna[SIZE] = { false }, RespostaMiniMatrizes[SIZE] = { false }, Respondido = false;
    //Realiza leitura da matrix sudoku
    for(i = 0; i < SIZE; i++) {
        for(j = 0; j < SIZE; j++) {
            scanf("%d", &Sudoku[i][j]);
        }
    }

    //Inicialização do array THreadInfo
    for(i = 0; i < SIZE; i++) {
        InfoLinha[i].index = i;
        InfoLinha[i].Valid = false;
        memset(InfoLinha[i].vector, 0, sizeof(int));
    }

    //Criação das Threads
    for(i = 0; i < SIZE; i++) {
        pthread_create(&ThreadsLinha[i], NULL, TesteLinha, &InfoLinha[i]);
    }

    for(i = 0; i < 9; i++) {
        void *rst;
        pthread_join(ThreadsLinha[i], &rst);
        RespostaLinha[i] = (bool) rst;
    }

    for(i = 0; i < SIZE; i++) {
        if(RespostaLinha[i] == false) {
            if(!Respondido) {
                printf("Por causa da linha %d, essa não é uma respota válida\n", i+1);
                Respondido = true;
                exit(1);
            }
        }
    }
    
    //////////Se o programa não saiu, vamos ver as colunas//////////////////////////////////////////////////

    //Inicialização do array THreadInfo
    for(i = 0; i < SIZE; i++) {
        InfoColuna[i].index = i;
        InfoColuna[i].Valid = false;
        memset(InfoColuna[i].vector, 0, sizeof(int));
    }

    //Criação das Threads
    for(i = 0; i < SIZE; i++) {
        pthread_create(&ThreadsColuna[i], NULL, TesteColuna, &InfoColuna[i]);
    }

    for(i = 0; i < 9; i++) {
        void *rst;
        pthread_join(ThreadsColuna[i], &rst);
        RespostaColuna[i] = (bool) rst;
    }

    for(i = 0; i < SIZE; i++) {
        if(RespostaColuna[i] == false) {
            if(!Respondido) {
                printf("Por causa da Coluna %d, essa não é uma respota válida\n", i+1);
                Respondido = true;
                exit(1);
            }
        }
    }

    /////////Se o programa não saiu, vamos cada bloquinho//////////////////////////////////////////////////    
    //Inicialização do MiniMatrixInfo
    
    for(i = 0; i < SIZE; i+=3) {
        for(j = 0; j < SIZE; j+=3) {
            memset(&InfoMiniMatrix[(i/3)*3 + (j/3)].vector, 0, sizeof(int));
            for(k = 0; k < 3; k++) {
                for(l = 0; l < 3; l++) {
                    InfoMiniMatrix[(i/3)*3 + (j/3)].matrix[k][l] = Sudoku[i + k][j + l];
                }
            }
        }
    }

    for(i = 0; i < SIZE; i++) {
        pthread_create(&ThreadMiniMatrix[i], NULL, TesteMiniMatrix, &InfoMiniMatrix[i]);
    }

    for(i = 0; i < 9; i++) {
        void *rst;
        pthread_join(ThreadMiniMatrix[i], &rst);
        RespostaMiniMatrizes[i] = (bool) rst;
    }

    for(i = 0; i < SIZE; i++) {
        if(RespostaMiniMatrizes[i] == false) {
            if(!Respondido) {
                printf("Por causa da mini matriz %d, essa não é uma respota válida\n", i + 1);
                Respondido = true;
                exit(1);
            }
        }
    }

    if(!Respondido) {
        printf("Essa é uma resposta válida! Parabéns, você completou o sudoku!\n");
    }
}

void* TesteLinha(void *arg) {
    ThreadInfo *Temp = (ThreadInfo *) arg;
    int i;
    for(i = 0; i < 9; i++) {
        pthread_mutex_lock(&mutex);
        Temp->vector[Sudoku[Temp->index][i] - 1]++;
        pthread_mutex_unlock(&mutex);
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
        pthread_mutex_lock(&mutex);
        Temp->vector[Sudoku[i][Temp->index] - 1]++;
        pthread_mutex_unlock(&mutex);
    }

    for(i = 0; i < 9; i++) {
        
        if(Temp->vector[i] == 0 || Temp->vector[i] > 1) {
            pthread_exit((void *)false);
        }
    }
    pthread_exit((void *)true);
}

void *TesteMiniMatrix(void* arg) {
    MiniMatrixInfo *Temp = (MiniMatrixInfo *) arg;
    int i, j;
    for(i = 0; i < 3; i++) {
        for(j = 0; j < 3; j++) {
            pthread_mutex_lock(&mutex);
            Temp->vector[Temp->matrix[i][j] - 1]++;
            pthread_mutex_unlock(&mutex);
        }
    }

    for(i = 0; i < SIZE; i++) {
        if(Temp->vector[i] == 0 || Temp->vector[i] > 1) {
            pthread_exit((void *)false);
        }
    }
    pthread_exit((void *) true);
}