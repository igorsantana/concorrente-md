/*Código paralelo*/

//Bibliotecas
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include "mpi.h"

#define NUMERO_CLIENTES 10
#define NUMERO_CAIXAS 5
#define TAG_CAIXA_RCV 50
#define TAG_CAIXA_SND 51
#define TAG_SEND_FILA 52

int RANK_TO_RECEIVE;

void produtor() {
    int caixas_fila[NUMERO_CAIXAS],
            numero_cliente = 0,
            valor = 1,
            ack = 0,
            soma_filas = 1;

    int j;
    for (j = 0; j < NUMERO_CAIXAS; j++) {
        caixas_fila[j] = 0;
    }

    int indexMenorFila = 0;
    while (numero_cliente < NUMERO_CLIENTES) {
        int i, flag = 0;

        for (i = 0; i < NUMERO_CAIXAS; i++) {
            if (caixas_fila[i] < caixas_fila[indexMenorFila]) {
                indexMenorFila = i;
            }
        }
        caixas_fila[indexMenorFila]++;

        for (i = 0; i < NUMERO_CAIXAS; i++) {
            if (caixas_fila[i] > 0) {
                printf("Cliente inserido no caixa %i\n", i + 1);
                MPI_Send(&valor, 1, MPI_INT, i + 1, TAG_CAIXA_SND, MPI_COMM_WORLD);
                numero_cliente++;
            }
        }

        for (i = 0; i < NUMERO_CAIXAS; i++) {
            if (caixas_fila[i] > 0) {
                MPI_Recv(&ack, 1, MPI_INT, i + 1, TAG_CAIXA_RCV, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                printf("Cliente atendido com sucesso no caixa %i\n", i + 1);
                caixas_fila[i]--;
            }
        }

        for (i = 0; i < NUMERO_CAIXAS; i++) {
            soma_filas += caixas_fila[i];
        }
        if (soma_filas <= 0) {
            MPI_Finalize();
        }
    }
}

void consumidor(int rank) {
    int cliente_atendido = 0;
    int ack = 1;
    while (1) {
        MPI_Recv(&cliente_atendido, 1, MPI_INT, 0, TAG_CAIXA_SND, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        sleep(1);
        MPI_Send(&ack, 1, MPI_INT, 0, TAG_CAIXA_RCV, MPI_COMM_WORLD);
    }
}

int main(int argc, char** argv) {

    MPI_Init(&argc, &argv);

    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    if (rank == 0) {
        produtor();
    } else {
        consumidor(rank);
    }
    return (EXIT_SUCCESS);
}