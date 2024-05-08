#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define Tam_Max 100

struct Arquivo{
    char* Caleb;
    char* arq;
};

pthread_mutex_t mutex;
int Nnomes;

void *lerArquivoThread(void *FileName) {
    //Usar const pra não haver alteração
    struct Arquivo *sapato = (struct Arquivo *) FileName;

    FILE *arquivo = fopen(sapato->arq, "r");
    if (arquivo == NULL) {
        pthread_exit(NULL);
        exit(1);
    }

    char linha[Tam_Max];
    while (fgets(linha, sizeof(linha), arquivo) != NULL) {
        if (strstr(linha, sapato->Caleb) != NULL) { 
            pthread_mutex_lock(&mutex);
            Nnomes++;
            pthread_mutex_unlock(&mutex);
        }
    }

    // Fecha o arquivo
    fclose(arquivo);
    pthread_exit(NULL);
}

int main() {
    int qtd;
    scanf("%d", &qtd);
    char *Pedro = (char*) malloc(sizeof(char)*Tam_Max); 
    struct Arquivo *ArqList[qtd];
    scanf(" %[^\n]", Pedro); 

    //colocando o nome buscado na struct
    //lendo o nome de cada um dos arquivos
    for(int i=0; i<qtd; ++i){
        ArqList[i] = (struct Arquivo*)malloc(sizeof(struct Arquivo));
        ArqList[i]->Caleb = (char*) malloc(strlen(Pedro)+1);
        strcpy(ArqList[i]->Caleb, Pedro);
        printf("Digite o nome e coloque o .txt no final do nome do arquivo\n");
        ArqList[i]->arq = (char *) malloc(sizeof(char)*Tam_Max);
        scanf(" %[^\n]", ArqList[i]->arq);
    }

    pthread_t threads[qtd];

    // Cria uma thread para cada arquivo
    for (int i = 0; i < qtd; ++i){
        pthread_mutex_init(&mutex, NULL);
        pthread_create(&threads[i], NULL, lerArquivoThread, ArqList[i]);
        pthread_join(threads[i], NULL);
        pthread_mutex_destroy(&mutex);
    }


    // Printa o total de ocorrências
    printf("%d\n", Nnomes);

    return 0;
}
