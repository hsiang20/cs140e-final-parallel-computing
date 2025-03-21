// engler, cs140e: driver for "bootloader" for an r/pi connected via 
// a tty-USB device.
//
// most of it is argument parsing.
//
// Unless you know what you are doing:
//              DO NOT MODIFY THIS CODE!
//              DO NOT MODIFY THIS CODE!
//              DO NOT MODIFY THIS CODE!
//              DO NOT MODIFY THIS CODE!
//              DO NOT MODIFY THIS CODE!
//
// You shouldn't have to modify any code in this file.  Though, if you find
// a bug or improve it, let us know!
#include <ctype.h>
#include <stdarg.h>
#include <string.h>
#include <sys/stat.h>
#include <termios.h>

#include "libunix.h"
#include "put-code.h"

#include <stdlib.h>

static char *progname = 0;

int is_integer(const char *str) {
    // Check for an empty string
    if (str == NULL || *str == '\0') {
        return 0;
    }
    // Handle negative numbers
    if (*str == '-') {
        str++;  // Skip the minus sign
    }
    // Check if all characters are digits
    while (*str != '\0') {
        if (!isdigit(*str)) {
            return 0;  // Not an integer if any non-digit character is found
        }
        str++;
    }
    return 1;  // It's a valid integer
}

static void usage(const char *msg, ...) {
    va_list args;
    va_start(args, msg);
    vfprintf(stderr, msg, args);
    va_end(args);

    output("\nusage: %s  [--trace-all] [--trace-control] ([device] | [--last] | [--first] [--device <device>]) <pi-program> \n", progname);
    output("    pi-program = has a '.bin' suffix\n");
    output("    specify a device using any method:\n");
    output("        <device>: has a '/dev' prefix\n");
    output("       --last: gets the last serial device mounted\n");
    output("        --first: gets the first serial device mounted\n");
    output("        --device <device>: manually specify <device>\n");
    output("    --baud <baud_rate>: manually specify baud_rate\n");
    output("    --trace-all: trace all put/get between rpi and unix side\n");
    output("    --trace-control: trace only control [no data] messages\n");
    exit(1);
}

int main(int argc, char *argv[]) { 
    char *dev_name = 0;
    char *pi_prog = 0;

    // used to pass the file descriptor to another program.
    char **exec_argv = 0;

    // a good extension challenge: tune timeout and baud rate transmission
    //
    // on linux, baud rates are defined in:
    //  /usr/include/asm-generic/termbits.h
    //
    // when I tried with sw-uart, these all worked:
    //      B9600
    //      B115200
    //      B230400
    //      B460800
    //      B576000
    // can almost do: just a weird start character.
    //      B1000000
    // all garbage?
    //      B921600
    unsigned baud_rate = B115200;

    // by default is 0x8000
    unsigned boot_addr = ARMBASE;

    // we do manual option parsing to make things a bit more obvious.
    // you might rewrite using getopt().
    progname = argv[0];
    int num_pis = 0;
    for(int i = 1; i < argc; i++) {
        if (is_integer(argv[i])) {
            char *endptr;
            long int value = strtol(argv[i], &endptr, 10);

            // Check if strtol failed to convert the string to an integer
            if (*endptr != '\0') {
                sys_die(main, "Argument %s is not a valid integer", argv[i]);
            }
            num_pis = value;
            printf("Argument %s is a valid integer: %ld\n", argv[i], value);
        } else if(strcmp(argv[i], "--trace-control") == 0)  {
            trace_p = TRACE_CONTROL_ONLY;
        } else if(strcmp(argv[i], "--trace-all") == 0)  {
            trace_p = TRACE_ALL;
        } else if(strcmp(argv[i], "--last") == 0)  {
            dev_name = find_ttyusb_last();
        } else if(strcmp(argv[i], "--first") == 0)  {
            dev_name = find_ttyusb_first();
        // we assume: anything that begins with /dev/ is a device.
        } else if(prefix_cmp(argv[i], "/dev/")) {
            dev_name = argv[i];
        // we assume: anything that ends in .bin is a pi-program.
        } else if(suffix_cmp(argv[i], ".bin")) {
            pi_prog = argv[i];
        } else if(strcmp(argv[i], "--baud") == 0) {
            i++;
            if(!argv[i])
                usage("missing argument to --baud\n");
            baud_rate = atoi(argv[i]);
        } else if(strcmp(argv[i], "--addr") == 0) {
            i++;
            if(!argv[i])
                usage("missing argument to --addr\n");
            boot_addr = atoi(argv[i]);
        } else if(strcmp(argv[i], "--exec") == 0) {
            i++;
            if(!argv[i])
                usage("missing argument to --exec\n");
            exec_argv = &argv[i];
            break;
        } else if(strcmp(argv[i], "--device") == 0) {
            i++;
            if(!argv[i])
                usage("missing argument to --device\n");
            dev_name = argv[i];
        } else {
            usage("unexpected argument=<%s>\n", argv[i]);
        }
    }
    if(!pi_prog)
        usage("no pi program\n");

    PiDevice devices[num_pis];
    for (int i = 0; i < num_pis; i++) {
        dev_name = find_ttyusb_i(i);
        if (!dev_name)
            panic("didn't find a device\n");
        debug_output("done with options: dev name=<%s>, pi-prog=<%s>, trace=%d\n", 
            dev_name, pi_prog, trace_p);

        if(exec_argv)
            argv_print("BOOT: --exec argv:", exec_argv);

        // 2. open the ttyUSB in 115200, 8n1 mode
        int tty = open_tty(dev_name);
        if(tty < 0)
            panic("can't open tty <%s>\n", dev_name);

        // timeout is in tenths of a second.  tuning this can speed up
        // checking.
        //
        // if you are on linux you can shrink down the <2*8> timeout
        // threshold.  if your my-install isn't reseting when used 
        // during checkig, it's likely due to this timeout being too
        // small.
        double timeout_tenths = 2*5;
        int fd = set_tty_to_8n1(tty, baud_rate, timeout_tenths);
        if(fd < 0)
            panic("could not set tty: <%s>\n", dev_name);

        // 3. read in program [probably should just make a <file_size>
        //    call and then shard out the pieces].
        unsigned nbytes;
        uint8_t *code = read_file(&nbytes, pi_prog);

        // 4. let's send it!
        debug_output("%s: tty-usb=<%s> program=<%s>: about to boot\n", 
                    progname, dev_name, pi_prog);

        // added 5. before send, change the header for id
        code[0] = i;
        code[1] = num_pis;
        // debug_output("header is now: %d\n", code[0]);
        simple_boot(fd, boot_addr, code, nbytes);

        devices[i].pi_fd = fd;
        devices[i].portname = dev_name;
        output("Found %s with fd: %d\n", dev_name, fd);
    }    



    // 5. echo output from pi
    if(!exec_argv)
        // pi_echo(0, fd, dev_name);
        pi_echo(0, devices, num_pis);
    else {
        todo("not handling exec_argv");
        // handoff_to(fd, TRACE_FD, exec_argv);
    }
	return 0;
}
