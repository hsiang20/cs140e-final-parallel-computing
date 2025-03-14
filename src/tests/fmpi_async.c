#include <stdio.h>
#include <stdlib.h>

#include "rpi.h"
#include "fmpi-async.h"
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
    printk("NOTMAIN: rank: %d, size: %d\n", rank, size);

    FMPI_Init_async(rank, size, 0);

    uint32_t text = 0xDEADBEEF;
    uint32_t address = 0;

    if (rank == 0) {
        delay_ms(3000);
        // FMPI_PUT(&text, 1);
        // delay_ms(2000);
        // gpio_set_off(TX_ASYNC);
        FMPI_PUT(1, &text, &address);
        delay_ms(3000);
    }
    if (rank == 1) {
        // for gpio interrupt
        rise_fall_int_startup();
        delay_ms(10000);
    }

    delay_ms(DELAY_MS);

    // MPI_Finalize();
    printk("rank: %d finished\n", rank);
}
