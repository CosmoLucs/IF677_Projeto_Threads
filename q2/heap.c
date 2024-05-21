#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

//=================================================================

#include "heap.h"

//=================================================================

minHeap* _minHeapCreate(int size){
    minHeap *H = (minHeap*)malloc(sizeof(minHeap));
    H->vetor = (int*)malloc(sizeof(int)*size+1);
    H->cnt=0;
    H->size=size;
    return H;
}
minHeap* _minHeapDestroy(minHeap* H){
    free(H->vetor);
    free(H);
    return H;
}

void _minHeapfyTop(minHeap* H){
    if(H->cnt/2 >= 1){
        int k=1;
        int v = H->vetor[k];
        bool heap = false;
        while(!heap && 2*k <= H->cnt){
            int j=2*k;
            if(j<H->cnt)
                if(H->vetor[j] > H->vetor[j+1])
                    j=j+1;
            if(v <= H->vetor[j])
                heap = true;
            else{
                H->vetor[k] = H->vetor[j];
                k = j;
            }
            H->vetor[k] = v;
        }
    }
}
void _minHeapfyBot(minHeap* H){
    for(int i = H->cnt/2; i>=1; i--){
        int k = i;
        int v = H->vetor[k];
        bool heap = false;
        while(!heap && 2*k <= H->cnt){
            int j=2*k;
            if(j<H->cnt)
                if(H->vetor[j] > H->vetor[j+1])
                    j++;
            if(v <= H->vetor[j])
                heap = true;
            else{
                H->vetor[k] = H->vetor[j];
                k =j;
            }
            H->vetor[k] = v;
        }
    }
}

void _minHeapInserir(minHeap* H, int value){
    if(H->size == H->cnt){
        //alocar mais memoria
        minHeap* newH = (minHeap*)malloc(sizeof(minHeap)*2*H->size);
        newH->cnt = H->cnt;
        newH->size = H->size;
        for(int i=0; i<H->size; i++){
            newH->vetor[i] = H->vetor[i];
        }
        free(H->vetor);
        free(H);
        H = newH;
    }
    H->cnt++;
    H->vetor[H->cnt] = value;
}
int _minHeapPop(minHeap* H){
    if(H->cnt==0) return 0;

    int sub = H->vetor[1];
    H->vetor[1] = H->vetor[H->cnt];
    H->vetor[H->cnt] = sub;

    H->cnt--;
    _minHeapfyTop(H);
    return sub;
}


// int main(){
//     int tam;
//     int valor;
//     scanf("%d", &tam);
//     do{
//         if(tam!=0){
//             minHeap* vet = _minHeapCreate(tam);
//             for(int i=0; i<tam; i++){
//                 scanf("%d", &valor);
//                 _minHeapInserir(vet, valor);
//             }
//             _minHeapfyBot(vet);

//             for(int i=1; i<=tam; i++)
//                 printf("%d", vet->vetor[i]);
//             printf("\n");

//             for(int i=0; i<tam; i++)
//                 printf("%d", _minHeapPop(vet));
//             printf("\n");
//         }
//     }while(tam!=0);
// }