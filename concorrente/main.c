/*Código paralelo*/

//Bibliotecas
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include "mpi.h"

#define NUMERO_CLIENTES 100
#define NUMERO_CAIXAS 5
#define TAG_CAIXA_RCV 50
#define TAG_CAIXA_SND 51
#define TAG_SEND_FILA 52

int RANK_TO_RECEIVE;

void produtor(){
    int caixas_fila[NUMERO_CAIXAS], valor = 1,
        zero = 0, ja_produzidos = 0;
    
    while(ja_produzidos < NUMERO_CLIENTES){
        
        int i, flag = 0;
        
        for(i= 1; i <= NUMERO_CAIXAS; i++){
            MPI_Recv(&caixas_fila[i - 1], 1, MPI_INT, i, TAG_CAIXA_RCV, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        }
        int menor = 6666666;
        for(i = 0; i < NUMERO_CAIXAS; i++){
            if(caixas_fila[NUMERO_CAIXAS] < menor){
                menor = i + 1;
            }
        }
        printf("Menor fila é a do caixa: %d\n", menor);
        
        for(i = 1; i <= NUMERO_CAIXAS; i++){
            if(i == menor){
                MPI_Send(&valor, 1, MPI_INT, i, TAG_CAIXA_SND, MPI_COMM_WORLD);
            } else {
                MPI_Send(&zero, 1, MPI_INT, i, TAG_CAIXA_SND, MPI_COMM_WORLD);
            }
        }
        ja_produzidos++;
    }
//    MPI_Bcast
}

void consumidor(int rank){
    int fila = 0, to_add = 0;
    while(1){
        
        MPI_Send(&fila, 1, MPI_INT, 0, TAG_CAIXA_RCV, MPI_COMM_WORLD);
        MPI_Recv(&to_add, 1, MPI_INT, 0, TAG_CAIXA_SND, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        
        if(to_add != 0){
            fila++;
            to_add = 0;
        }  
        if(fila > 0){
            printf("Consumindo item no rank %d\n", rank);
            fila--;
        }
        if(fila == 0){
            printf("Fila do processo %d está vazia\n", rank);
//            break;
        }
    }
    sleep(1);
}

void start() {
    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    if(rank == 0){
        produtor();
    } else {
        consumidor(rank);
    }

}

int main(int argc, char** argv) {

    MPI_Init(&argc, &argv);
    start();
    MPI_Finalize();
    

    return (EXIT_SUCCESS);
}