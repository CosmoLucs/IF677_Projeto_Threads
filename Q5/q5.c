/*
O método de Jacobi é uma técnica representativa para solucionar sistemas 
de equações lineares (SEL). Um sistema de equações lineares possui o seguinte
formato: Ax = b, no qual (imagem).
Ex:
2x1 + x2 = 11
5x1 + 7x2 =  13
O métido de Jacobi assume uma solução inicial para as incógnitas (x_i) e o 
resultado é refinado durante P iterações, usando o algoritmo (...).
while(k < P){
    //i=1,2,3...n;
    for(int i=0; i<tamMatrix_X; i++){
        int somatorio_a_x = 0;
        for(int j=0; j<tamMatriz_x_ou_a; i++)
            if(j!=i) somatorio_a_x += a[i][j]*x[j]; //ou é esse que é elevado a k
        x[i]^(k+1) = 1/A[i][i] (b[i] - somatorio_a[i][j]_x[j]^(k))
    }
    k++;
}
Por exemplo, assumindo o SEL apresentado anteriormente, P=10, e 
x[1]^(0) = 1 e x[2]^(0) = 1;
while(k<10){
    x[1]^(k+1) = 1/2 * (11 - x[2]^(k));
    x[2]^(k+1) = 1/7 * (13 - 5 * x[1]^(k));
    k++;
}
Exemplo de execução
k=0
    x[1]^(1) = 1/2 * (11 - x[2]^(0)) = 1/2 * (11-1) = 5;
    x[2]^(1) = 1/7 * (13 - 5*x[1]^(0)) = 1/7*(13-5 * 1) = 1.1428;
k=1
    x[1]^(2) = 1/2 * (11 - 1.1428);
    x[2]^(2) = 1/7 * (13 - 5 * 5);
...

Nesta questão, o objetivo é quebrar a execução sequencial em threads, na qual o 
valor de cada incógnita x[j] pode ser calculado de forma concorrente em relação 
às demais incógnitas
(Ex: x[1]^(k+1) pode ser calcula ao mesmo tempo que x[2]^(k+1)). A quantidade de 
threads a serem criadas vai depender de um parâmetro 'N' passado pelo usuário 
durante a execução do programa, e 'N' deverá ser equivalente à quantidade de 
processadores (ou núcleos) que a máquina possuir. No início do programa, e 
nenhuma thread poderá ficar com menos incógnitas associadas à ela.

Para facilitar a construção do programa e a entrada de dados, as matrizes não 
precisam ser lidas do teclado, ou seja, pode ser inicializadas diretametne dentro 
do programa (ex.:inicialização estática de vetores). Ademais, os valores iniciais
de x[i]^(0) deverão ser iguais a 1, e adote mecanismo (ex.: barriers) para 
sincronizar as threads depois de cada iteração. 
Faça a experimentação executando o programa em uma máquina com 4
processadores/núcleos, demonstrando a melhoria da execução do programa com 1, 2 e 4 
threads.

ATENÇÃO: apesar de x[1]^(k+1) pode ser calculada o mesmo tempo que x[2]^(k+1), 
x[i]^(k+2) só poderão ser calculadas quando todas incógnitas x[i] (ou j) forem 
calculadas. Barriers é uma excelente ferramenta para essa questão. 

OBS: usar duas barreiras. 
*/
#define _XOPEN_SOURCE 600
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdbool.h>

pthread_barrier_t barreiraCalculo;
pthread_barrier_t barreiraEscrita;

typedef struct sistLinear {
    int k;
    int P;
    int tamLinhas, tamColunas;
    float** A;
    float* b;
    float* x;
    float* xAux;
}sistLinear;

typedef struct infoThread{
    int id;
    sistLinear* infoSL;
} infoThread;


//melhorar//talvez tenha algum erro
void* JacobiThread(void* arg){
    infoThread info = (*(infoThread*) arg);
    while(info.infoSL->k < info.infoSL->P){
        for(int i=0; i<info.infoSL->tamLinhas; i++){
            int somatorio_a_x = 0; 
            for(int j=0; j<info.infoSL->tamLinhas; i++){
                if(j!=i) somatorio_a_x += info.infoSL->A[i][j] * info.infoSL->x[j];
            }
            info.infoSL->xAux[i] = 1/info.infoSL->A[i][i] * info.infoSL->b[i] - somatorio_a_x;
            pthread_barrier_wait(&barreiraCalculo);
            pthread_barrier_wait(&barreiraEscrita);
        }
        
    }
    pthread_exit(NULL);
}


int main(){
    sistLinear matrizes;

    //recebendo dados
    scanf("%d %d", &matrizes.tamLinhas, &matrizes.tamColunas);
    matrizes.A = (int**)malloc(sizeof(int*)*matrizes.tamLinhas);
    for(int i=0; i<matrizes.tamLinhas; i++){
        matrizes.A[i] = (int*)malloc(sizeof(int)*matrizes.tamColunas);
        for(int j=0; j<matrizes.tamColunas; j++)
            scanf("%d", &matrizes.A[i][j]);
    }
    matrizes.b = (int*)malloc(sizeof(int)*matrizes.tamLinhas);
    for(int i=0; i<matrizes.tamLinhas; i++)
        scanf("%d", &matrizes.b);
    //vetor x
    matrizes.x = (int*)malloc(sizeof(int)*matrizes.tamLinhas);
    

    pthread_barrier_init(&barreiraCalculo, NULL, matrizes.tamLinhas+1);
    pthread_barrier_init(&barreiraEscrita, NULL, matrizes.tamLinhas+1);

    //inicializando threads
    pthread_t threads[matrizes.tamLinhas];
    infoThread info[matrizes.tamLinhas];
    for(int i=0; i<matrizes.tamLinhas; i++){
        info[i].id = i;
        info[i].infoSL = &matrizes;
    }

    //atualizando o vetor x e o k
    while(matrizes.k < matrizes.P){
        pthread_barrier_wait(&barreiraCalculo);
        //
        matrizes.k++;
        pthread_barrier_wait(&barreiraEscrita);
    }

    //liberando barreira
    pthread_barrier_destroy(&barreiraCalculo);
    pthread_barrier_destroy(&barreiraEscrita);
    //DESACOLANDO MEMÓRIA 
    free(matrizes.b);
    free(matrizes.x);
    for(int i=0; i<matrizes.tamLinhas; i++)
        free(matrizes.A[i]);
    free(matrizes.A);

    return 0;
}
