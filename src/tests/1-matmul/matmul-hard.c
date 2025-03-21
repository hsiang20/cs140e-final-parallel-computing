// #include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

#include "rpi.h"
#include "fmpi.h"
#include "test-interrupts.h"

#define rows_per_process 128
#define N 256  // Matrix size (NxN)

typedef int32_t data_type;

void generate_matrices(data_type A[N][N], data_type B[N][N], data_type result[N][N]) {
    // Fill A with rows of increasing numbers
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            A[i][j] = i + 1;
            B[i][j] = 1;  // B is all ones
        }
    }

    // Calculate the expected result: each row sums to (i+1) * N
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            result[i][j] = (i + 1) * N;
            // result[i][j] = 0;
        }
    }
}

void matrix_multiply(data_type A[N][N], data_type B[N][N], data_type C[N][N]) {
    // Standard matrix multiplication
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            C[i][j] = 0;
            for (int k = 0; k < N; k++) {
                C[i][j] += A[i][k] * B[k][j];
            }
        }
    }
}

int verify_result(data_type C[N][N], data_type expected[N][N]) {
    // Check if C matches the expected result
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            if (C[i][j] != expected[i][j]) {
                printk("Verify: at [%d][%d], expected %d, got %d\n", i, j, expected[i][j], C[i][j]);
                return 0;  // Mismatch
            }
        }
    }
    return 1;  // All correct
}

// Function to multiply a row of A with B and store in C
void matmul(data_type A[], data_type B[N][N], data_type C[], int rank, int i) {
    for (int j = 0; j < N; j++) {
        C[j] = 0;
        for (int k = 0; k < N; k++) {
            C[j] += A[k] * B[k][j];
            // assert(B[k][j] == 0);
            // if (A[k] != (rank*rows_per_process+i+1)) {
            //     printk("rank: %d, A[%d][%d] is %d\n", rank, rank*rows_per_process+i+1, k, A[k]);
            // }
            // if (B[k][j] != 0) {
            //     printk("rank: %d, B[%d][%d] is %d\n", rank, k, j, B[k][j]);
            // }
        }
    }
}

// int main(int argc, char* argv[]) {
void notmain(void) {
    uart_init();
    uint8_t rank, size;
    // int rows_per_process = 256 / size;  // Assuming N is divisible by size
    // assert(N == 4);
    // assert(size == 2);
    data_type A[N][N], B[N][N], C[N][N], expected[N][N];
    data_type A_sub[rows_per_process][N], C_sub[rows_per_process][N];

    FMPI_Init(&rank, &size, 0);
    printk("rank: %d, size: %d\n", rank, size);
    // MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    // MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (rank == 0) {
        generate_matrices(A, B, expected);
    }

    uint32_t start = cycle_cnt_read();

    // Broadcast matrix B to all processes
    FMPI_Bcast(B, N * N, sizeof(data_type));
    printk("%d done Bcast\n", rank);

    // Scatter rows of A to different processes
    FMPI_Scatter(A, rows_per_process * N,
                A_sub, rows_per_process * N, sizeof(data_type));
    printk("%d done Scatter\n", rank);

    FMPI_Barrier();

    // Compute local matrix multiplication
    for (int i = 0; i < rows_per_process; i++) {
        matmul(A_sub[i], B, C_sub[i], rank, i);
    }
    printk("%d done computation\n", rank);

    // Gather results back into matrix C
    FMPI_Gather(C_sub, rows_per_process * N,
                C, rows_per_process * N, sizeof(data_type));
    printk("%d done Gather\n", rank);

    FMPI_Barrier();

    uint32_t end = cycle_cnt_read();

    if (rank == 0) {
        if (verify_result(C, expected)) {
            printk("Matrix multiplication successful and verified!\n");
        } else {
            printk("Matrix multiplication failed verification.\n");
        }
    }

    delay_ms(DELAY_MS);

    // MPI_Finalize();
    // output("rank: %d finished!!\n", rank);
}
