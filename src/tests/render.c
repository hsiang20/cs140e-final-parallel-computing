#include <stdio.h>
#include <stdlib.h>

#include "rpi.h"
#include "fmpi.h"

#define rows_per_process 128
#define N 512
#define NUM_CIRCLES 100

typedef int32_t data_type;

static data_type image[N][N][3];
static data_type x[NUM_CIRCLES], y[NUM_CIRCLES], radius[NUM_CIRCLES];
static data_type r[NUM_CIRCLES], g[NUM_CIRCLES], b[NUM_CIRCLES];

// Simple Linear Congruential Generator (LCG) without division
unsigned int seed = 12345;

void send_back_to_laptop(data_type image[N][N][3]) {
    for (int i = 0; i < N * N * 3; i++) {
        uart_put8(((char *)image)[i]);
    }
}

unsigned int lcg() {
    seed = (1664525 * seed + 1013904223);
    return seed & 0x7FFFFFFF; // Keep it positive
}

// Generate a number within a given range without division
data_type random_range(int min, int max) {
    return (lcg() & ((max - min) - 1)) + min;
}

// Check if a point is inside a circle
int is_inside_circle(int cx, int cy, int x, int y, int r) {
    return ((x - cx) * (x - cx) + (y - cy) * (y - cy)) <= (r * r);
}

void render(data_type image_sub[rows_per_process][N][3], int rank) {
    for (int i = 0; i < rows_per_process; i++) {
        for (int j = 0; j < N; j++) {
            int px = rank * rows_per_process + i;
            int py = j;
            for (int c_idx = 0; c_idx < NUM_CIRCLES; c_idx++) {
                if (is_inside_circle(x[c_idx], y[c_idx], px, py, radius[c_idx])) {
                    image_sub[i][j][0] = r[c_idx];
                    image_sub[i][j][1] = g[c_idx];
                    image_sub[i][j][2] = b[c_idx];
                } 
            }
        }
    }
}

void notmain(void) {
    uart_init();
    const uint8_t rank = *(uint8_t *)(0x8000);
    const uint8_t size = *(uint8_t *)(0x8000 + 1);
    const uint8_t root = 0;

    FMPI_Init(rank, size, root);

    if (rank == root) {
        for (int i = 0; i < NUM_CIRCLES; i++) {
            x[i] = random_range(0, N);
            y[i] = random_range(0, N);
            radius[i] = random_range(10, 50);
            r[i] = random_range(0, 256);
            g[i] = random_range(0, 256);
            b[i] = random_range(0, 256);
        }
    }

    FMPI_Bcast(x, NUM_CIRCLES, sizeof(data_type));
    FMPI_Bcast(y, NUM_CIRCLES, sizeof(data_type));
    FMPI_Bcast(radius, NUM_CIRCLES, sizeof(data_type));
    FMPI_Bcast(r, NUM_CIRCLES, sizeof(data_type));
    FMPI_Bcast(g, NUM_CIRCLES, sizeof(data_type));
    FMPI_Bcast(b, NUM_CIRCLES, sizeof(data_type));

    data_type image_sub[rows_per_process][N][3] = {0};
    // FMPI_Scatter(image, rows_per_process * N * 3,
    //     image_sub, rows_per_process * N * 3, sizeof(data_type));
    
    render(image_sub, rank);

    FMPI_Gather(image_sub, rows_per_process * N * 3,
        image, rows_per_process * N * 3, sizeof(data_type));

    if (rank == root) {
        send_back_to_laptop(image);
    }

}
