#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

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

int main(int argc, char* argv[]) {
    int rank, size;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    int A[N][N], B[N][N], C[N][N];

    if (rank == 0) {
        // Initialize matrices A and B in root process
        printf("Matrix A:\n");
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < N; j++) {
                A[i][j] = i + j;  // Example values
                printf("%d ", A[i][j]);
            }
            printf("\n");
        }

        printf("\nMatrix B:\n");
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < N; j++) {
                B[i][j] = i - j;  // Example values
                printf("%d ", B[i][j]);
            }
            printf("\n");
        }
        printf("\n");
    }

    // Broadcast matrix B to all processes
    MPI_Bcast(B, N * N, MPI_INT, 0, MPI_COMM_WORLD);

    int rows_per_process = N / size;  // Assuming N is divisible by size
    int A_sub[rows_per_process][N], C_sub[rows_per_process][N];

    // Scatter rows of A to different processes
    MPI_Scatter(A, rows_per_process * N, MPI_INT,
                A_sub, rows_per_process * N, MPI_INT,
                0, MPI_COMM_WORLD);

    // Compute local matrix multiplication
    for (int i = 0; i < rows_per_process; i++) {
        matmul(A_sub[i], B, C_sub[i]);
    }

    // Gather results back into matrix C
    MPI_Gather(C_sub, rows_per_process * N, MPI_INT,
               C, rows_per_process * N, MPI_INT,
               0, MPI_COMM_WORLD);

    if (rank == 0) {
        // Print the result matrix
        printf("Result Matrix C:\n");
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < N; j++) {
                printf("%d ", C[i][j]);
            }
            printf("\n");
        }
    }

    MPI_Finalize();
    return 0;
}
