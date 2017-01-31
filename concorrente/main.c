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

void produtor(){
    int caixas_fila[NUMERO_CAIXAS], valor = 1;
    int ja_produzidos = 0;
    while(ja_produzidos < NUMERO_CLIENTES){
        int i , j = 0;
        for(i= 1; i <= NUMERO_CAIXAS; i++){
            MPI_Send(&valor, 1, MPI_INT, i + 1, TAG_SEND_FILA, MPI_COMM_WORLD);
        }
        for(i= 1; i <= NUMERO_CAIXAS; i++){
            MPI_Recv(&caixas_fila[i - 1], 1, MPI_INT, i, TAG_CAIXA_RCV, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        }
        int menor = 6666666;
        for(j = 0; j < NUMERO_CAIXAS; j++){
            if(caixas_fila[NUMERO_CAIXAS] < menor){
                menor = j + 1;
            }
        }
        MPI_Send(&valor, 1, MPI_INT, menor, TAG_CAIXA_SND, MPI_COMM_WORLD);
        ja_produzidos++;
        sleep(2);
    }
}

void consumidor(int rank){
    int fila = 0;
    while(1){
        MPI_Send(&fila, 1, MPI_INT, 0, TAG_CAIXA_RCV, MPI_COMM_WORLD);
        sleep(1);
        break;
    }
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