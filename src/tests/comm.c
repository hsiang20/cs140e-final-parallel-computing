// low level print that makes a bit easier to debug.
#include "rpi.h"
#include "sw-uart.h"

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

    // use pin 16 for tx, 17 for rx
    sw_uart_t u = sw_uart_init(16, 17, 115200);
    uint8_t sent = 5;

    if (id == 0) {
        uint8_t recv = sw_uart_get8(&u);
        printk("TRACE: recv: %d\n", recv);
    } else if (id == 1) {
        sw_uart_put8(&u, sent);
        printk("TRACE: sent: %d\n", sent);
    }

    output("HELLO WORLD using output\n ");
}
