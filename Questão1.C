#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define Tam_Max 100
#define NOME "lucas"

void *lerArquivoThread(void *FileName) {
    //Usar const pra não haver alteração
    const char *nomeArquivo = (const char *)FileName;

    FILE *arquivo = fopen(nomeArquivo, "r");
    if (arquivo == NULL) {
        pthread_exit(NULL);
        exit(1);
    }

    char linha[Tam_Max];
    int *contador = (int *) malloc(sizeof(int));
    while (fgets(linha, sizeof(linha), arquivo) != NULL) {
        if (strstr(linha, NOME) != NULL) { 
            (*contador)++;
        }
    }

    // Fecha o arquivo
    fclose(arquivo);
    //printf("%d nesse arquivo ", *contador);

    pthread_exit((void *) contador);
}

int main() {
    const char *arquivos[] = {"Arquivo1.txt", "Arquivo2.txt", "Arquivo3.txt"};
    pthread_t threads[3];
    int totalOcorrencias = 0;

    // Cria uma thread para cada arquivo
    for (int i = 0; i < 3; ++i) {
        pthread_create(&threads[i], NULL, lerArquivoThread, (void *)arquivos[i]);
    }

    // Aguarda até que todas as threads tenham terminado e soma as ocorrências
    for (int i = 0; i < 3; i++) {
        int *resultado = 0;
        pthread_join(threads[i], (void **)&resultado);
        if (resultado != NULL) {
            totalOcorrencias += *resultado;
            free(resultado);
        }
    }

    // Printa o total de ocorrências
    printf("%d", totalOcorrencias);

    return 0;
}
