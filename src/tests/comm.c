// low level print that makes a bit easier to debug.
#include "rpi.h"

extern unsigned int _crt0_header;

static void my_putk(char *s) {
    for(; *s; s++)
        uart_put8(*s);
}

void notmain(void) {
    uart_init();
    const uint32_t id = *(uint32_t *)(0x8000);

    printk("id: %x\n", id);
    // my_putk("TRACE:hello world using my_putk\n");
    // printk("TRACE:hello world using printk\n");
    output("HELLO WORLD using output\n ");
}
