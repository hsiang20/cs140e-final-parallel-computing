// low level print that makes a bit easier to debug.
#include "rpi.h"
#include "sw-uart.h"
#include "test-interrupts.h"

static unsigned tx = 16;
static unsigned rx = 17;

static void my_putk(char *s) {
    for(; *s; s++)
        uart_put8(*s);
}

void notmain(void) {
    uart_init();
    
    // for gpio interrupt
    rise_fall_int_startup();

    const uint32_t id = *(uint32_t *)(0x8000);

    printk("id: %d\n", id);

    // use pin 16 for tx, 17 for rx
    sw_uart_t u = sw_uart_init(tx, rx, 115200);
    uint8_t sent = 12;

    if (id == 0) {
        delay_ms(1000 * 1);
        sw_uart_put8(&u, sent);
        printk("TRACE: sent: %d\n", sent);
        // delay_ms(1000 * 5);
    } else if (id == 1) {
        // uint8_t recv = sw_uart_get8(&u);
        // printk("TRACE: recv: %d\n", recv);
        delay_ms(1000 * 3);
    }

    printk("n_rising: %d, n_falling: %d\n", n_rising, n_falling);

    output("HELLO WORLD using output\n");
}
