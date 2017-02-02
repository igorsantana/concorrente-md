/*a*/

#include "stdio.h"
#include "stdlib.h"
#include "mpi.h"
#include "time.h"

#define TAG_TAMANHO_FILA_PEDIDO 50
#define FILA 49
#define TAG_TAMANHO_FILA_RESPOSTA 51
#define TAG_ATENDIMENTO 52
#define TAG_FLAG 53
#define NUMERO_MAX_CLIENTES 10
#define NUMERO_CAIXAS 5
int numero_cliente = 0;

void atende_cliente(int rank) {
    printf("Cliente atendido no caixa %i\n", rank);
}

int cria_cliente() {
    int numero_c = 1 + (rand() % 5);
    if (NUMERO_MAX_CLIENTES - numero_cliente <= 10) {
        numero_c = 1;
    }
    if (numero_c != 1) {
        while ((numero_c + numero_cliente) > NUMERO_MAX_CLIENTES) {
            numero_c = 1 + (rand() % 5);
        }
    }
    if (numero_cliente == NUMERO_MAX_CLIENTES) {
        return -1;
    }

    numero_cliente += numero_c;
    return numero_c;
}

int menor_fila() {
    int soma_filas = 0;
    int i;
    int menor = 1;
    int tamanho_filas[NUMERO_CAIXAS];
    int pedido_tamanho_fila;

    for (i = 1; i < NUMERO_CAIXAS; i++) {
        MPI_Send(&pedido_tamanho_fila, 1, MPI_INT, i, TAG_TAMANHO_FILA_PEDIDO, MPI_COMM_WORLD);
    }

    for (i = 1; i < NUMERO_CAIXAS; i++) {
        MPI_Recv(&tamanho_filas[i], 1, MPI_INT, i, TAG_TAMANHO_FILA_RESPOSTA, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        soma_filas = soma_filas + tamanho_filas[i];
        if (tamanho_filas[menor] > tamanho_filas[i]) {
            menor = i;
        }
    }

    if (soma_filas <= 0 && numero_cliente >= NUMERO_MAX_CLIENTES) {
        return -1;
    }
    return menor;
}

void produtor() {
    int flag;
    int cliente = 0, i;
    int nulo = -1;
    int index_menor_fila = 1;
    while (1) {
        cliente = cria_cliente();
        index_menor_fila = menor_fila();

        if (cliente == -1) break;

        for (i = 1; i < NUMERO_CAIXAS; i++) {
            int valor;
            if (i != index_menor_fila) {
                valor = nulo;
            } else {
                valor = cliente;
                printf("Serão adicionados %d clientes em %d\n", valor, i);
            }

            MPI_Send(&valor, 1, MPI_INT, i, TAG_ATENDIMENTO, MPI_COMM_WORLD);
        }
    }

    int especial = -50;
    for (i = 1; i < NUMERO_CAIXAS; i++) {
        MPI_Send(&especial, 1, MPI_INT, i, TAG_ATENDIMENTO, MPI_COMM_WORLD);
    }
    for (i = 1; i < NUMERO_CAIXAS; i++) {
        MPI_Recv(&especial, 1, MPI_INT, i, FILA, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    }
    printf("\n\nfinalizado produtor\n\n");
}

void consumidor(int rank) {
    int fila = 0;
    int flag_loop = 1;
    int i, m, flag = 1;
    int resposta_tamanho_fila = 1;

    while (flag_loop) {
        MPI_Recv(&resposta_tamanho_fila, 1, MPI_INT, 0, TAG_TAMANHO_FILA_PEDIDO, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        MPI_Send(&fila, 1, MPI_INT, 0, TAG_TAMANHO_FILA_RESPOSTA, MPI_COMM_WORLD);

        MPI_Recv(&m, 1, MPI_INT, 0, TAG_ATENDIMENTO, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        if (m == -50) {
            while (fila > 0) {
                atende_cliente(rank);
                fila--;
            }
            flag_loop = 0;
        } else if (m != -1) {
            fila += m;

        } else if (fila > 0) {
            atende_cliente(rank);
            fila--;
        }
    }
    MPI_Send(&fila, 1, MPI_INT, 0, FILA, MPI_COMM_WORLD);
    printf("finalizado processo %i\n", rank);
}

int main(int argc, char** argv) {
    srand((unsigned) time(NULL));
    MPI_Init(&argc, &argv);

    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    if (rank == 0) {
        produtor();
    } else {
        consumidor(rank);
    }
    MPI_Finalize();
    return (EXIT_SUCCESS);
}
