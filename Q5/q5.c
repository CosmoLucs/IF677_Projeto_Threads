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

//está dando segmentation fault...e parando por algum motivo...
//alem de calcular errado
#define _XOPEN_SOURCE 600
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdbool.h>


pthread_barrier_t barreiraCalculo;
pthread_barrier_t barreiraEscrita;

typedef struct matrizes{
    int tamLinhas, tamColunas; //sao iguais sempre
    float** A;
    float* b;
    float* x;
    float* xAux;
}matrizes;

typedef struct sistLinear {
    int P;
    matrizes dados; 
}sistLinear;

typedef struct infoThread{
    int id;
    int numJacobi; 
    int k;
    sistLinear *sistema;
} infoThread;

//===========================================================================

void recebendoDados(sistLinear* arg){
    scanf("%d", &arg->P);
    scanf("%d %d", &arg->dados.tamLinhas, &arg->dados.tamColunas);
    arg->dados.A = (float**)malloc(sizeof(float*)*arg->dados.tamLinhas);
    for(int i=0; i<arg->dados.tamLinhas; i++){
        arg->dados.A[i] = (float*)malloc(sizeof(float)*arg->dados.tamColunas);
        for(int j=0; j<arg->dados.tamColunas; j++)
            scanf("%f", &arg->dados.A[i][j]);
    }
    arg->dados.b = (float*)malloc(sizeof(float)*arg->dados.tamLinhas);
    for(int i=0; i<arg->dados.tamLinhas; i++) 
        scanf("%f", &arg->dados.b[i]); 
    //vetor x e xAux 
    arg->dados.x = (float*)calloc(sizeof(float),arg->dados.tamLinhas);
    arg->dados.xAux = (float*)calloc(sizeof(float),arg->dados.tamLinhas);

    for (int i = 0; i < arg->dados.tamLinhas; i++) {
        arg->dados.x[i] = 1.0;
        arg->dados.xAux[i] = 1.0;
    }
}
void recebendoConstantes(sistLinear* arg){
    arg->P = 50; 
    arg->dados.tamLinhas = 2; 
    arg->dados.tamColunas = 2;

    arg->dados.A = (float**)malloc(sizeof(float*) * arg->dados.tamLinhas);
    for(int i = 0; i < arg->dados.tamLinhas; i++){
        arg->dados.A[i] = (float*)malloc(sizeof(float) * arg->dados.tamColunas);
    }
    arg->dados.A[0][0] = 2.0;
    arg->dados.A[0][1] = 1.0;
    arg->dados.A[1][0] = 5.0;
    arg->dados.A[1][1] = 7.0;

    arg->dados.b = (float*)malloc(sizeof(float) * arg->dados.tamLinhas);
    arg->dados.b[0] = 11.0;
    arg->dados.b[1] = 13.0;

    // vetor x e xAux 
    arg->dados.x = (float*)calloc(sizeof(float), arg->dados.tamLinhas);
    arg->dados.xAux = (float*)calloc(sizeof(float), arg->dados.tamLinhas);

    for (int i = 0; i < arg->dados.tamLinhas; i++) {
        arg->dados.x[i] = 1.0;
        arg->dados.xAux[i] = 1.0;
    }
}

void printandoDados(sistLinear* arg){ 
    printf("Dados do Sistema Linear\n"); 
    printf("Matriz A de coeficientes\n"); 
    for(int i=0; i<arg->dados.tamColunas; i++){
        for(int j=0; j<arg->dados.tamColunas; j++) 
            printf("%f ", arg->dados.A[i][j]); 
        printf("\n"); 
    }
    printf("Vetor b\n");
    for(int i=0; i<arg->dados.tamLinhas; i++)
        printf("%f ", arg->dados.b[i]);
    printf("\n");
    printf("Resultado do vetor X com %d iteracoes\n", arg->P);
    for(int i=0; i<arg->dados.tamLinhas; i++)
        printf("x[%d]: %f\n", i, arg->dados.x[i]);
    printf("\n");
}

//===========================================================================


//melhorar//talvez tenha algum erro 
void* JacobiThread(void* arg){ 
    infoThread info = (*(infoThread*) arg); 
    int i = info.numJacobi; 
    while(info.k < info.sistema->P){ 
        //----------------------------------------------------------------- 
        float somatorio_a_x = 0; 
        for(int j=0; j<info.sistema->dados.tamLinhas; j++){ 
            if(j!=i) somatorio_a_x += info.sistema->dados.A[i][j] * info.sistema->dados.x[j];
        } 
        info.sistema->dados.xAux[i] = (1/info.sistema->dados.A[i][i]) * 
                                        (info.sistema->dados.b[i] - somatorio_a_x); 
        
        pthread_barrier_wait(&barreiraCalculo); 
        pthread_barrier_wait(&barreiraEscrita); 
        info.k++; 
        //----------------------------------------------------------------- 
    } 
    pthread_exit(NULL);
}



int main(){
    sistLinear matrizes;
    recebendoDados(&matrizes);

    pthread_barrier_init(&barreiraCalculo, NULL, matrizes.dados.tamLinhas+1);
    pthread_barrier_init(&barreiraEscrita, NULL, matrizes.dados.tamLinhas+1);

    //inicializando threads 
    pthread_t threads[matrizes.dados.tamLinhas]; 
    infoThread info[matrizes.dados.tamLinhas]; 
    for(int i=0; i<matrizes.dados.tamLinhas; i++){ 
        info[i].k=0;
        info[i].id = i; 
        info[i].numJacobi = i; 
        info[i].sistema = &matrizes; 
        pthread_create(&threads[i], NULL, JacobiThread, (void*) &info[i]); 
    } 

    //atualizando o vetor x e o k 
    int k=0; 
    while(k < matrizes.P){ 
        pthread_barrier_wait(&barreiraCalculo); 
        //passando todas as mudanças 
        for(int i=0; i<matrizes.dados.tamLinhas; i++) 
            matrizes.dados.x[i] = matrizes.dados.xAux[i]; 
        pthread_barrier_wait(&barreiraEscrita); 
        k++; 
    }

    //liberando barreira
    pthread_barrier_destroy(&barreiraCalculo);
    pthread_barrier_destroy(&barreiraEscrita);

    //printando os valores do xFinal
    printandoDados(&matrizes);

    //DESACOLANDO MEMÓRIA 
    free(matrizes.dados.xAux);
    free(matrizes.dados.x);
    free(matrizes.dados.b);
    for(int i=0; i<matrizes.dados.tamLinhas; i++)
        free(matrizes.dados.A[i]);
    free(matrizes.dados.A);

    pthread_exit(NULL);
}
