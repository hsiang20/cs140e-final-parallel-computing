// #include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

#include "rpi.h"
#include "fmpi.h"

#define N 4  // Matrix size (NxN)

// Function to multiply a row of A with B and store in C
void matmul(int A[], int B[N][N], int C[]) {
    for (int j = 0; j < N; j++) {
        C[j] = 0;
        for (int k = 0; k < N; k++) {
            C[j] += A[k] * B[k][j];
        }
    }
}

// int main(int argc, char* argv[]) {
void notmain(void) {
    uart_init();
    const uint8_t rank = *(uint8_t *)(0x8000);
    const uint8_t size = *(uint8_t *)(0x8000 + 1);
    printk("rank: %d, size: %d\n", rank, size);

    FMPI_Init(rank, size, 0);
    // MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    // MPI_Comm_size(MPI_COMM_WORLD, &size);

    int A[N][N], B[N][N], C[N][N];

    if (rank == 0) {
        // Initialize matrices A and B in root process
        printk("Matrix A:\n");
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < N; j++) {
                A[i][j] = i + j;  // Example values
                printk("%d ", A[i][j]);
            }
            printk("\n");
        }

        printk("\nMatrix B:\n");
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < N; j++) {
                B[i][j] = i - j;  // Example values
                printk("%d ", B[i][j]);
            }
            printk("\n");
        }
        printk("\n");
    }

    // Broadcast matrix B to all processes
    // MPI_Bcast(B, N * N, MPI_INT, 0, MPI_COMM_WORLD);
    FMPI_Bcast(B, N * N);

    // int rows_per_process = N / size;  // Assuming N is divisible by size
    assert(N == 4);
    assert(size == 2);
    int rows_per_process = 2;
    int A_sub[rows_per_process][N], C_sub[rows_per_process][N];

    // Scatter rows of A to different processes
    // MPI_Scatter(A, rows_per_process * N, MPI_INT,
    //             A_sub, rows_per_process * N, MPI_INT,
    //             0, MPI_COMM_WORLD);
    FMPI_Scatter(A, rows_per_process * N,
                A_sub, rows_per_process * N);

    // Compute local matrix multiplication
    for (int i = 0; i < rows_per_process; i++) {
        matmul(A_sub[i], B, C_sub[i]);
    }

    // Gather results back into matrix C
    // MPI_Gather(C_sub, rows_per_process * N, MPI_INT,
    //            C, rows_per_process * N, MPI_INT,
    //            0, MPI_COMM_WORLD);
    FMPI_Gather(C_sub, rows_per_process * N,
                C, rows_per_process * N);
    
    if (rank == 0) {
        // Print the result matrix
        printk("Result Matrix C:\n");
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < N; j++) {
                printk("%d ", C[i][j]);
            }
            printk("\n");
        }
    }

    // MPI_Finalize();
    output("rank: %d finished\n", rank);
}
