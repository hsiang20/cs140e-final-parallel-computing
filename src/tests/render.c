#include <stdio.h>
#include <stdlib.h>

#include "rpi.h"
#include "fmpi.h"
#include "cycle-count.h"

#define DEBUG 0

#define N 128
#define rows_per_process (N / 4)
#define NUM_CIRCLES 1000

typedef uint8_t data_type;
typedef int32_t len_type;

static data_type image[N][N][3];
static len_type x[NUM_CIRCLES], y[NUM_CIRCLES], radius[NUM_CIRCLES];
static data_type r[NUM_CIRCLES], g[NUM_CIRCLES], b[NUM_CIRCLES];

void debug_printk(const char *format, ...) {
    if (DEBUG) {
        va_list args;               
        va_start(args, format);     // Start processing arguments
        printk(format);
        va_end(args); 
    }
}

// Simple Linear Congruential Generator (LCG) without division
unsigned int seed = 12345;

void send_back_to_laptop(data_type image[N][N][3]) {
    for (int i = 0; i < N; i++) {
        // uart_put8(((char *)image)[i]);
        for (int j = 0; j < N; j++) {
            for (int idx = 0; idx < 3; idx++) {
                data_type *pixel = &image[i][j][idx];
                for (int d = 0; d < sizeof(data_type); d++) {
                    uart_put8(((char *)pixel)[d]);
                }
            }
            // delay_ms(10);
        }
    }
}

unsigned int lcg() {
    seed = (1664525 * seed + 1013904223);
    return seed & 0x7FFFFFFF; // Keep it positive
}

// Generate a number within a given range without division
int random_range(int min, int max) {
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

void render_all(data_type image_sub[N][N][3]) {
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            int px = i;
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

uint32_t elapsed_cycles(uint32_t start, uint32_t end) {
    return (end >= start) ? (end - start) : (0xFFFFFFFF - start + end + 1);
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
            radius[i] = random_range(N / 20, N / 7);
            r[i] = random_range(0, 256);
            g[i] = random_range(0, 256);
            b[i] = random_range(0, 256);
        }
    }

    cycle_cnt_init();
    delay_ms(10);
    uint32_t start = cycle_cnt_read();

    FMPI_Bcast(x, NUM_CIRCLES, sizeof(len_type));
    FMPI_Bcast(y, NUM_CIRCLES, sizeof(len_type));
    FMPI_Bcast(radius, NUM_CIRCLES, sizeof(len_type));

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
        uint32_t end = cycle_cnt_read();
        // debug_printk("finish rendering\n");
        send_back_to_laptop(image);

        printk("start    : %u\n", start);
        printk("end      : %u\n", end);

        // delay_ms(100);


        data_type image_sub2[N][N][3] = {0};
        cycle_cnt_init();
        delay_ms(10);
        uint32_t start_one = cycle_cnt_read();
        render_all(image_sub2);
        uint32_t end_one = cycle_cnt_read();
        // send_back_to_laptop(image_sub2);
        
        // delay_ms(500);
        
        printk("start_one: %u\n", start_one);
        printk("end_one  : %u\n", end_one);
        printk("num_cycles for 2: %u\n", elapsed_cycles(start, end));
        printk("num_cycles for 1: %u\n", elapsed_cycles(start_one, end_one));

    }
    delay_ms(100);
}
