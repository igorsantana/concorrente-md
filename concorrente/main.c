#include "stdio.h"
#include "stdlib.h"
#include "mpi.h"
#include <time.h>

#define TAG_TAMANHO_FILA_PEDIDO 50
#define TAG_TAMANHO_FILA_RESPOSTA 51
#define TAG_ATENDIMENTO 51
#define NUMERO_MAX_CLIENTES 20
#define NUMERO_CAIXAS 5
int numero_cliente = 0;

void atende_cliente(int rank) {
    printf("Cliente atendido no caixa %i\n", rank);
}

int cria_cliente() {
    
    int numero_c =  1 + ( rand() % 5 );
    if(NUMERO_MAX_CLIENTES - numero_cliente <= 10){
        numero_c =  1;
    }
    if(numero_c != 1){
        while((numero_c + numero_cliente) > NUMERO_MAX_CLIENTES){
            numero_c = 1 + ( rand() % 5 );
        }
    }
    if (numero_cliente == NUMERO_MAX_CLIENTES) {
        return -1;
    }
    
    numero_cliente+= numero_c;
    return numero_c;
}

int menor_fila() {
    int i;
    int menor = 1;
    int tamanho_filas[NUMERO_CAIXAS];
    int pedido_tamanho_fila;

    for (i = 1; i < NUMERO_CAIXAS; i++) {
        MPI_Send(&pedido_tamanho_fila, 1, MPI_INT, i, TAG_TAMANHO_FILA_PEDIDO, MPI_COMM_WORLD);
    }

    for (i = 1; i < NUMERO_CAIXAS; i++) {
        MPI_Recv(&tamanho_filas[i], 1, MPI_INT, i, TAG_TAMANHO_FILA_RESPOSTA, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        if (tamanho_filas[menor] > tamanho_filas[i]) {
            menor = i;
        }
    }

    return menor;
}

void produtor() {
    int cliente = 0, i;
    int nulo = -1;
    int index_menor_fila;
    while (1) {
        cliente = cria_cliente();
        
        if (cliente == -1) {
            break;
        }

        index_menor_fila = menor_fila();

        for (i = 1; i < NUMERO_CAIXAS; i++) {
            if (i != index_menor_fila) {
                MPI_Send(&nulo, 1, MPI_INT, i, TAG_ATENDIMENTO, MPI_COMM_WORLD);
            } else {
                printf("Serão adicionados %d clientes em %d\n", cliente, i);
                MPI_Send(&cliente, 1, MPI_INT, index_menor_fila, TAG_ATENDIMENTO, MPI_COMM_WORLD);
            }
        }
    }
}

void consumidor(int rank) {
    int fila = 0;
    int i, m, flag = 1;
    int resposta_tamanho_fila = 1;

    while (1) {
        MPI_Recv(&resposta_tamanho_fila, 1, MPI_INT, 0, TAG_TAMANHO_FILA_PEDIDO, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        MPI_Send(&fila, 1, MPI_INT, 0, TAG_TAMANHO_FILA_RESPOSTA, MPI_COMM_WORLD);

        MPI_Recv(&m, 1, MPI_INT, 0, TAG_ATENDIMENTO, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        if (m != -1) {
            fila += m;
        }
        if(fila > 0){
            atende_cliente(rank);
            fila--;
        }
        
    }
}

int main(int argc, char** argv) {
    srand( (unsigned)time(NULL) );
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
