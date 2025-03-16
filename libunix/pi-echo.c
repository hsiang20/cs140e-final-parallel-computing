#include <assert.h>
#include <ctype.h>
#include <unistd.h>
#include "libunix.h"

#include <stdio.h>
#define RENDER 0

#define N 128
typedef uint8_t data_type;

static int32_t image[N][N][3];
static int x = 0, y = 0, idx = 0;

// hack-y state machine to indicate when we've seen the special string
// 'DONE!!!' from the pi telling us to shutdown.
int pi_done(unsigned char *s) {
    unsigned pos = 0;
    const char exit_string[] = "DONE!!!\n";
    const int n = sizeof exit_string - 1;

    for(; *s; s++) {
        assert(pos < n);
        if(*s != exit_string[pos++]) {
            pos = 0;
            return pi_done(s+1); // check remainder
        }
        // maybe should check if "DONE!!!" is last thing printed?
        if(pos == sizeof exit_string - 1)
            return 1;
    }
    return 0;
}

// overwrite any unprintable characters with a space.
// otherwise terminals can go haywire/bizarro.
// note, the string can contain 0's, so we send the
// size.
void remove_nonprint(uint8_t *buf, int n) {
    for(int i = 0; i < n; i++) {
        uint8_t *p = &buf[i];
        if(isprint(*p) || (isspace(*p) && *p != '\r'))
            continue;
        *p = ' ';
    }
}


int update_pixel() {
    if (++idx == 3) {
        idx = 0;
        if (++y == N) {
            y = 0;
            x++;
        }
    }
    return x == N;
}

void update_image(unsigned char buf[4096], int n) {
    if (n % sizeof(data_type) != 0)
        output("n: %d not aligned %lu\n", n, sizeof(data_type));

    for (int i = 0; i < n / sizeof(data_type); i++) {
        image[x][y][idx] = ((data_type *)buf)[i];
        if (update_pixel()) return;
    }
}

void write_image(int32_t image[N][N][3]) {
    FILE *file = fopen("images/image.ppm", "w");
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

// read and echo the characters from the usbtty until it closes 
// (pi rebooted) or we see a string indicating a clean shutdown.
// void pi_echo(int unix_fd, int pi_fd, const char *portname) {
void pi_echo(int unix_fd, PiDevice* devices, int num_pis) {
    // assert(pi_fd);
    for (int i = 0; i < num_pis; i++)
        assert(devices[i].pi_fd);
#if 0
    if(portname)
        output("listening on ttyusb=<%s>\n", portname);
#endif

    int *done = malloc(num_pis * sizeof(int));
    int count = 0;
    
    // added
    int n_total = 0;
    // 
    while(1) {
        unsigned char buf[4096];
        for (int i = 0; i < num_pis; i++) {
            if (done[i])
                continue;
            
            int pi_fd = devices[i].pi_fd;
            const char* portname = devices[i].portname;

            int n;
            if((n=read_timeout(unix_fd, buf, sizeof buf, 1000))) {
                buf[n] = 0;
                // output("about to echo <%s> to pi\n", buf);
                write_exact(pi_fd, buf, n);
            }

            if(!can_read_timeout(pi_fd, 1000))
                continue;
            n = read(pi_fd, buf, sizeof buf - 1);

            if(!n) {
                // this isn't the program's fault.  so we exit(0).
                if(!portname || tty_gone(portname)) {
                    // clean_exit("pi ttyusb connection closed.  cleaning up\n");
                    clean_exit("pi ttyusb %s connection closed.  cleaning up\n", portname);
                }
                // so we don't keep banginging on the CPU.
                usleep(1000);
            } else if(n < 0) {
                sys_die(read, "pi connection %s closed.  cleaning up\n", portname);
            } else {
                buf[n] = 0;
                if (RENDER && i == 0 && x < N) {
                    update_image(buf, n);
                    n_total += n;
                } else {
                    // if you keep getting "" "" "" it's b/c of the GET_CODE message from bootloader
                    remove_nonprint(buf,n);
                    output("%s", buf);
                }

                if(pi_done(buf)) {
                    // output("\nSaw done\n");
                    // clean_exit("\nbootloader: pi exited.  cleaning up\n");
                    done[i] = 1;
                    count++;
                }
            }
            if (count == num_pis) {
                if (RENDER) {
                    output("n_total: %d\n", n_total);
                    write_image(image);
                }
                free(done);
                clean_exit("\nbootloader: pi exited.  cleaning up\n");
            }
        }
    }
    notreached();
}
