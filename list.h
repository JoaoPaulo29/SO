#ifndef LIST_H
#define LIST_H

#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>


#define MAXARGS 7
#define BUFFER_SIZE 100

typedef struct no{
    int pid,estado;
    time_t tempoInicio,tempoFinal;
    struct no *proximo;
}No;

typedef struct{
    No *cabeca;
}lista;

int controlarExite=0;
pthread_mutex_t buffer_mutex;
pthread_t thread;
lista *listPid;
int numeroFilhos = 0;
int iteracao, maximaConcorrencia, totalTempo=0;
pthread_cond_t condicaoPrincipal, condicaoMonitora;

lista* inicialiZarListaPid();
int inserirNaLista(lista *list, int pid, time_t tempoInicio);
void* tarefaMonitora();
void atualizacaoTempoProcesso(lista *list, int pid, time_t endtime,int estado);
void listarProcesso(lista *list);

lista* inicialiZarListaPid(){
    lista *list=(lista*)malloc(sizeof (lista));
    list->cabeca=NULL;
    return list;
}
int inicioDoProcesso(lista *list, int pid){
    No* aux=list->cabeca;
    while(aux!=NULL){
        if(aux->pid==pid){
            return aux->tempoInicio;
        }
        aux=aux->proximo;
    }
    return -1;
}

void listarProcesso(lista* list){
    printf("\n\nlistar os processos com o tempo de inicio e fim.\n\n");
    No* aux=list->cabeca; int i=1;
    while (aux != NULL){
        printf("%d - ",i);
        if(WIFEXITED(aux->estado))
            printf("pid: %d exited normally; status=%d.", aux->pid, WEXITSTATUS(aux->estado));
        else
            printf("pid: %d terminated without calling exit.", aux->pid);
        i++;
        printf(" Execution time: %d s\n", (int)  (aux->tempoFinal - aux->tempoInicio));
        aux = aux->proximo;
  }
    printf("\nFim Da Lista.\n");
}

void atualizacaoTempoProcesso(lista *list, int pid, time_t tempoFinal, int estado){
   No *aux=list->cabeca;
   while(aux!=NULL){
       if(aux->pid==pid){
            aux->tempoFinal=tempoFinal;
            aux->estado=estado;
            break;
            }
        aux=aux->proximo;
    }
    printf("teminated process with pid: %d\n", pid);
}    
  
int inserirNaLista(lista *list, int pid, time_t tempoInicio){
    if(list==NULL) return 0;
    No *novo=(No*)malloc(sizeof(No));
    if(novo==NULL) return 0;
    novo->pid= pid;
    novo->tempoInicio=tempoInicio;
    novo->tempoFinal=0;
    novo->estado=0;
    novo->proximo=NULL;
   if((list->cabeca)==NULL){
       list->cabeca=novo;
       return 1;
   }else{
       No *aux= list->cabeca;
       while(aux->proximo!=NULL)
           aux=aux->proximo;
       aux->proximo=novo;
   }
   return 1;
}
void EliminarLista(lista *list){
    No *elemento=list->cabeca, *proximoElemento;
    while(elemento!=NULL){
        proximoElemento=elemento->proximo;
        free(elemento);
        elemento=proximoElemento;
    } free(list);
}
void fecharFicheiro(FILE *file){
    if (fclose(file)) {
        perror("Error closing file");
        exit(EXIT_FAILURE);
    }
}
void condicao_Wait(pthread_cond_t* condicao, pthread_mutex_t* mutex) {
    if (pthread_cond_wait(condicao, mutex)) {
        perror("Erro no  pthread_cond_wait");
        exit(EXIT_FAILURE);
    }
}
#endif 
