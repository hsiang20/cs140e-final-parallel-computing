// low level print that makes a bit easier to debug.
#include "rpi.h"
#include "sw-uart.h"

// extern unsigned int _crt0_header;
static unsigned tx = 16;
static unsigned rx = 17;

static void my_putk(char *s) {
    for(; *s; s++)
        uart_put8(*s);
}

void notmain(void) {
    uart_init();
    const uint8_t id = *(uint8_t *)(0x8000);

    printk("id: %d\n", id);

    // use pin 16 for tx, 17 for rx
    sw_uart_t u = sw_uart_init(tx, rx, 115200);
    uint8_t sent = 5;

    if (id == 0) {
        delay_ms(1000 * 3);
        sw_uart_put8(&u, sent);
        printk("TRACE: sent: %d\n", sent);
    } else if (id == 1) {
        uint8_t recv = sw_uart_get8(&u);
        printk("TRACE: recv: %d\n", recv);
    }

    output("HELLO WORLD using output\n");
}
