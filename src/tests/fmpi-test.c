#include <stdio.h>
#include <stdlib.h>

#include "rpi.h"
#include "fmpi.h"

#define rows_per_process (64)
#define N (256)  // Matrix size (NxN)

typedef int32_t data_type;

static uint8_t rank, size;

void verify_Bcast(data_type B[N][N]) {
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            if (B[i][j] != 0) {
                printk("%d has B[%d][%d] = %d\n", rank, i, j, B[i][j]);
            }
        }
    }
    printk("Done verify Bcast\n");
}

void verify_Scatter(data_type A_sub[rows_per_process][N]) {
    for (int i = 0; i < rows_per_process; i++) {
        for (int j = 0; j < N; j++) {
            if (A_sub[i][j] != 0) {
                printk("%d has A_sub[%d][%d] = %d\n", rank, i, j, A_sub[i][j]);
            }
        }
    }
    printk("Done verify Scatter\n");
}

void verify_Gather(data_type C[N][N]) {
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            if (C[i][j] != 0) {
                printk("%d has C[%d][%d] = %d\n", rank, i, j, C[i][j]);
            }
        }
    }
    printk("Done verify Gather\n");
}

void notmain(void) {
    uart_init();
    FMPI_Init(&rank, &size, 0);
    printk("rank: %d, size: %d\n", rank, size);

    data_type B[N][N];
    if (rank == 0) {
        memset(B, 0, sizeof(B));
    }
    FMPI_Bcast(B, N * N, sizeof(data_type));
    printk("%d done Bcast\n", rank);
    verify_Bcast(B);


    data_type A[N][N];
    if (rank == 0) {
        memset(A, 0, sizeof(A));
    }
    data_type A_sub[rows_per_process][N];
    FMPI_Scatter(A, rows_per_process * N,
        A_sub, rows_per_process * N, sizeof(data_type));
    printk("%d done Scatter\n", rank);
    verify_Scatter(A_sub);


    data_type C[N][N];
    data_type C_sub[rows_per_process][N];
    memset(C_sub, 0, sizeof(C_sub));
    FMPI_Gather(C_sub, rows_per_process * N,
        C, rows_per_process * N, sizeof(data_type));
    printk("%d done Gather\n", rank);
    if (rank == 0)
        verify_Gather(C);
    
}