//Projeto - Infra-Estrutura de Software - Projeto Threads
//Questão 01 - status: Funcionando.

//--------------------//----------------------

/* Todos os arquivos ultilizados nessa questão
possuem a mesma formatação, cada linha possui um
único nome, escrito unicamente com letras minúsculas.*/

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define Tam_Max 100

//A Struct irá carregar a palavra que será procurada e o nome do arquivo
struct Arquivo{
    char* SearchName;
    char* FileName;
};

//Declaração global da mutex e da quantidade de nome encontrados
pthread_mutex_t mutex;
int Nnomes;

//Função para leitura do arquivo
void *lerArquivoThread(void *Files) {

    //Boa prática de variável temporária
    struct Arquivo *Temp = (struct Arquivo *) Files;

    //inicialização do arquivo
    FILE *ToReadFile = fopen(Temp->FileName, "r");
    if (ToReadFile == NULL) {
        pthread_exit(NULL);
        exit(1);
    }

    //leitura e procura 
    char *linha = (char *) malloc(Tam_Max);
    if(linha == NULL) {
        pthread_exit(NULL);
        exit(1);
    }

    while (fgets(linha, Tam_Max, ToReadFile) != NULL) {
        while(strstr(linha, Temp->SearchName) != NULL) { 
            pthread_mutex_lock(&mutex);
            Nnomes++;
            linha+=strlen(Temp->SearchName);
            pthread_mutex_unlock(&mutex);
        }
    }

    // Fecha o arquivo, liberar memória e encerra a thread
    fclose(ToReadFile);
    pthread_exit(NULL);
}

int main() {
    int FileQtd;
    printf("Digite a quantidade de arquivos que você deseja analisar:\n");
    scanf("%d", &FileQtd);

    char *Keyword = (char*) malloc(sizeof(char)*Tam_Max); 

    struct Arquivo *ArqList[FileQtd];
    printf("Digite a palavra que você está procurando nos arquivos:\n");
    scanf(" %[^\n]", Keyword); 


    //colocando o nome buscado na struct
    //lendo o nome de cada um dos arquivos
    for(int i=0; i<FileQtd; ++i){
        
        //Alocação inicial da struct
        ArqList[i] = (struct Arquivo*)malloc(sizeof(struct Arquivo) * FileQtd);
        //Alocação de cada string
        ArqList[i]->SearchName = (char*) malloc(strlen(Keyword)+1);
        ArqList[i]->FileName = (char *) malloc(sizeof(char)*Tam_Max);

        //Todas as instâncias da struct irão procurar pela mesma palavra
        strcpy(ArqList[i]->SearchName, Keyword);

        printf("Digite o nome de um arquivo(lembresse de adicionar '.txt' ao final):\n");
        scanf(" %[^\n]", ArqList[i]->FileName);
    }

    //criação das threads
    pthread_t threads[FileQtd];

    // Cria uma thread para cada arquivo
    for (int i = 0; i < FileQtd; ++i){
        //inicialização do mutex para controle
        pthread_mutex_init(&mutex, NULL);

        pthread_create(&threads[i], NULL, lerArquivoThread, ArqList[i]);
        pthread_join(threads[i], NULL);

        pthread_mutex_destroy(&mutex);
    }


    // Printa o total de ocorrências
    printf("%d\n", Nnomes);

    return 0;
}
