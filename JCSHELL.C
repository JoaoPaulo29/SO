/*
// jc-shell - exercicio 1, version 1
// Sistemas Operativos, DEI-CC/FC/UAN 2020
*/

#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>

#include <unistd.h>
#include <time.h>
#include <pthread.h>

#include "list.h"
#include "commandlinereader.h"

void *tarefaMonitora() {
  FILE *arquivo;
  arquivo = fopen("log.txt", "a");//Abrir o ficheiro log.txt com modo de leitura a
  if (arquivo == NULL) {exit(EXIT_FAILURE);}
  while (1) {
    pthread_mutex_lock(&buffer_mutex);
    while (numeroFilhos == 0 && controlarExite == 0) {
      pthread_cond_wait(&condicaoMonitora, &buffer_mutex); //ESPERAR A CONDIÇÃO MONITORA
    }
    if (controlarExite == 1 && numeroFilhos == 0) {
      pthread_mutex_unlock(&buffer_mutex);
      break;
    }
    pthread_mutex_unlock(&buffer_mutex);
    /*wait for child*/
    int status,childpid = wait(&status);
    if (childpid == -1) {
      perror("Error waiting for child");
      exit(EXIT_FAILURE);
    }

    time_t tempoFinal = time(NULL);
    pthread_mutex_lock(&buffer_mutex);
    numeroFilhos--;
    atualizacaoTempoProcesso(listPid, childpid, tempoFinal, status);
    int duracao = tempoFinal - inicioDoProcesso(listPid, childpid);
    if (maximaConcorrencia > 0) {pthread_cond_signal(&condicaoPrincipal);/*dESPERTAR A CONDIÇÃO PRINCIPAL*/}
    pthread_mutex_unlock(&buffer_mutex);
    totalTempo += duracao;
    iteracao++;
    //escrever no ficheiro
    fprintf(arquivo, "iteracao %d\npid: %d execution time: %d s\ntotal execution time: %d s\n", (iteracao), childpid, duracao, totalTempo);
    //LIMPAR O BUFFER(LIXO) DO FICHEIRO
    fflush(arquivo);
  }
  fecharFicheiro(arquivo);//FECHAR O ARQUIVO
  pthread_exit(NULL);
}

int main (int argc, char **argv){
    char buffer[BUFFER_SIZE], *args[MAXARGS];
    int pid,numeroArgumentos;
    listPid=inicialiZarListaPid();
    if(argc!=2){argc=2;}
    maximaConcorrencia=argc;
    //CRIAR A CONDIÇÃO PRINCIPAL
    if(maximaConcorrencia>0 && pthread_cond_init(&condicaoPrincipal,NULL)){
        perror("Erro ao initializar condicão");
        exit(EXIT_FAILURE);
    }
    pthread_cond_init(&condicaoMonitora,NULL);//inicializar a condição monitora
    FILE* arquivo=fopen("log.txt","r");//Abrir o ficheiro log.txt com modo de leitura r
    if(arquivo==NULL){iteracao=0;}
   if(arquivo){
     //FAZER LEITURA DOS DADOS(ITERAÇÃO,DURAÇÃO E O TEMPO TOTAL) QUE ESTÃO NO ARQUIVO
        while(fgets(buffer,BUFFER_SIZE,arquivo)){
          int duracao;
            if(sscanf(buffer, "iteracao %d", &iteracao)){continue;}
            if(sscanf(buffer, "total execution time: %d", &duracao)){continue;}
            if(sscanf(buffer,"pid: %d execution time: %d s", &pid, &duracao)){totalTempo+=duracao;}
        }
        fecharFicheiro(arquivo);
    }
    pthread_mutex_init(&buffer_mutex,NULL);//INICIALIZAR A MUTEX
    pthread_create(&thread, NULL, tarefaMonitora, NULL);//CRIAR TAREFA
    printf("Child processes concurrency limit: %d", maximaConcorrencia);
    (maximaConcorrencia == 0) ? printf(" (sem limite)\n\n") : printf("\n\n");
    while(1){
        numeroArgumentos = readLineArguments(args, MAXARGS, buffer, BUFFER_SIZE);
        if (numeroArgumentos < 0) {
            printf("Erro no readLineArguments()\n");
            continue;
        }
        if (numeroArgumentos == 0) {continue;}
        if (strcmp(args[0], "exit") == 0) {break; }
        if(maximaConcorrencia > 0){
            pthread_mutex_lock(&buffer_mutex);
            while(numeroFilhos==maximaConcorrencia){
                condicao_Wait(&condicaoPrincipal,&buffer_mutex);//ESPERAR A CONDIÇÃO PRINCIPAL
            }
            pthread_mutex_unlock(&buffer_mutex);
        }
        time_t tempoInicio = time(NULL);
        pid = fork();
        if(pid < 0) {
            pthread_cond_signal(&condicaoPrincipal);
            continue;
        }
        if (pid > 0) {
            pthread_mutex_lock(&buffer_mutex);
            numeroFilhos ++;
            inserirNaLista(listPid, pid,tempoInicio);
            pthread_cond_signal(&condicaoMonitora);
            pthread_mutex_unlock(&buffer_mutex);
        }
        if (pid == 0) {
            if (execv(args[0], args) == -1) {
                perror("Erro no comando.");
                  exit(EXIT_FAILURE);
            }
        }
    }
    pthread_mutex_lock(&buffer_mutex);
    controlarExite = 1;
    pthread_cond_signal(&condicaoMonitora);
    pthread_mutex_unlock(&buffer_mutex);
    pthread_join(thread, NULL);
    listarProcesso(listPid);
    pthread_mutex_destroy(&buffer_mutex);
    pthread_cond_destroy(&condicaoPrincipal);
    pthread_cond_destroy(&condicaoMonitora);
    EliminarLista(listPid);
    return 0;
}
