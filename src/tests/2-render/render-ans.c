#include <stdio.h>
#include <stdlib.h>

#define N 128
#define NUM_CIRCLES 5000

typedef uint8_t data_type;
typedef int32_t len_type;

static data_type image[N][N][3] = {0};
static len_type x[NUM_CIRCLES], y[NUM_CIRCLES], radius[NUM_CIRCLES];
static data_type r[NUM_CIRCLES], g[NUM_CIRCLES], b[NUM_CIRCLES];

unsigned int seed = 12345;

unsigned int lcg() {
    seed = (1664525 * seed + 1013904223);
    return seed & 0x7FFFFFFF; // Keep it positive
}

// Generate a number within a given range without division
int32_t random_range(int min, int max) {
    return (lcg() & ((max - min) - 1)) + min;
}

// Check if a point is inside a circle
int is_inside_circle(int cx, int cy, int x, int y, int r) {
    return ((x - cx) * (x - cx) + (y - cy) * (y - cy)) <= (r * r);
}

void render(data_type image_sub[N][N][3]) {
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            int px = i;
            int py = j;
            for (int c_idx = 0; c_idx < NUM_CIRCLES; c_idx++) {
                if (is_inside_circle(x[c_idx], y[c_idx], px, py, radius[c_idx])) {
                    image_sub[i][j][0] = (r[c_idx] >> 1) + (image_sub[i][j][0] >> 1);
                    image_sub[i][j][1] = (g[c_idx] >> 1) + (image_sub[i][j][1] >> 1);
                    image_sub[i][j][2] = (b[c_idx] >> 1) + (image_sub[i][j][2] >> 1);
                } 
            }
        }
    }
}

void write_image(data_type image[N][N][3]) {
    FILE *file = fopen("images/image-ans.ppm", "w");
    if (file) {
        fprintf(file, "P3\n%d %d\n255\n", N, N);
        for (int y = 0; y < N; y++) {
            for (int x = 0; x < N; x++) {
                fprintf(file, "%d %d %d ", image[y][x][0], image[y][x][1], image[y][x][2]);
            }
            fprintf(file, "\n");
        }
        fclose(file);
    } else {
        printf("Failed to write the file!\n");
    }
}

int main() {
    for (int i = 0; i < NUM_CIRCLES; i++) {
        x[i] = random_range(0, N);
        y[i] = random_range(0, N);
        radius[i] = random_range(N / 20, N / 7);
        r[i] = random_range(0, 256);
        g[i] = random_range(0, 256);
        b[i] = random_range(0, 256);
    }
    render(image);
    write_image(image);

    return 0;
}