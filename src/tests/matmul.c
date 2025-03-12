// #include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

#include "rpi.h"
#include "fmpi.h"
#include "test-interrupts.h"

#define N 4  // Matrix size (NxN)

// Function to multiply a row of A with B and store in C
void matmul(data_type A[], data_type B[N][N], data_type C[]) {
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
    FMPI_Init_async(rank, size, 0);
    // MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    // MPI_Comm_size(MPI_COMM_WORLD, &size);

    data_type A[N][N], B[N][N], C[N][N];

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

        // printk("A_seq: ");
        // int8_t *start = (int8_t *)A;
        // for (int i = 0; i < N * N; i++) {
        //     printk("%d ", start[i]);
        // }
        // printk("\n");

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

    if (rank == 1) {
        rise_fall_int_startup();
    }

    // Call an async put from rank 0
    uint8_t text = 3;
    if (rank == 0) {
        gpio_set_off(TX_ASYNC);
        FMPI_PUT(&text, 1);
    }

    printk("rank %d is here\n", rank);

    // Broadcast matrix B to all processes
    FMPI_Bcast(B, N * N);
    // printk("%d done Bcast\n", rank);

    // int rows_per_process = N / size;  // Assuming N is divisible by size
    assert(N == 4);
    assert(size == 2);
    int rows_per_process = 2;
    data_type A_sub[rows_per_process][N], C_sub[rows_per_process][N];

    // Scatter rows of A to different processes
    FMPI_Scatter(A, rows_per_process * N,
                A_sub, rows_per_process * N);
    // printk("%d done Scatter\n", rank);
    // for (int i = 0; i < rows_per_process; i++) {
    //     printk("%d: ", rank);
    //     for (int j = 0; j < N; j++) {
    //         printk("%d ", A_sub[i][j]);
    //     }
    //     printk("\n");
    // }

    // Compute local matrix multiplication
    for (int i = 0; i < rows_per_process; i++) {
        matmul(A_sub[i], B, C_sub[i]);
    }

    // Gather results back into matrix C
    FMPI_Gather(C_sub, rows_per_process * N,
                C, rows_per_process * N);
    
    // printk("%d done Gather\n", rank);
    // for (int i = 0; i < rows_per_process; i++) {
    //     printk("%d: ", rank);
    //     for (int j = 0; j < N; j++) {
    //         printk("%d ", C_sub[i][j]);
    //     }
    //     printk("\n");
    // }
    
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

    delay_ms(DELAY_MS);

    // MPI_Finalize();
    output("rank: %d finished\n", rank);
}
