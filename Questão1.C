#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define Tam_Max 100
#define NOME "lucas"

struct Arquivo{
    char* Caleb;
    char* arq;
};

void *lerArquivoThread(void *FileName) {
    //Usar const pra não haver alteração
    struct Arquivo *sapato = (struct Arquivo *) FileName;

    FILE *arquivo = fopen(sapato->arq, "r");
    if (arquivo == NULL) {
        pthread_exit(NULL);
        exit(1);
    }

    char linha[Tam_Max];
    int *contador = (int *) malloc(sizeof(int));
    while (fgets(linha, sizeof(linha), arquivo) != NULL) {
        if (strstr(linha, sapato->Caleb) != NULL) { 
            (*contador)++;
        }
    }

    // Fecha o arquivo
    fclose(arquivo);
    printf("%d nesse arquivo \n", *contador);

    pthread_exit((void *) contador);
}

int main() {
    int qtd = scanf("%d", &qtd);
    char *Pedro = (char*) malloc(sizeof(char)*Tam_Max); 
    struct Arquivo *ArqList[qtd];
    scanf(" %[^\n]", Pedro); 
    printf("%s\n", Pedro);

    for(int i=0; i<qtd; ++i){
        //colocando o nome buscado na struct
        ArqList[i]->Caleb = (char*) malloc(strlen(Pedro)+1);
        strcpy(ArqList[i]->Caleb, Pedro);
        printf(" %s\n", ArqList[i]->Caleb);
        //lendo o nome de cada um dos arquivos
        printf("Coloca o .txt no final do nome do arquivo\n");
        ArqList[i]->arq = (char *) malloc(sizeof(char)*Tam_Max);
        scanf(" %[^\n]", ArqList[i]->arq);
        printf(" %s\n", ArqList[i]->arq);
    }

    pthread_t threads[qtd];
    int totalOcorrencias = 0;

    // Cria uma thread para cada arquivo
    for (int i = 0; i < qtd; ++i) {
        pthread_create(&threads[i], NULL, lerArquivoThread, ArqList[i]);
    }

    // Aguarda até que todas as threads tenham terminado e soma as ocorrências
    for (int i = 0; i < qtd; i++) {
        int *resultado = 0;
        pthread_join(threads[i], (void **)&resultado);
        if (resultado != NULL) {
            totalOcorrencias += *resultado;
            free(resultado);
        }
    }

    // Printa o total de ocorrências
    printf("%d\n", totalOcorrencias);

    return 0;
}
