// low level print that makes a bit easier to debug.
#include "rpi.h"
#include "timer.h"

static void my_putk(char *s) {
    for(; *s; s++)
        uart_put8(*s);
}

void notmain(void) {
    uart_init();

    interrupt_init();
    timer_init(1, 0x1000);
    
    my_putk("TRACE:hello world using my_putk\n");
    printk("TRACE:hello world using printk\n");
    output("HELLO WORLD using output\n ");

    enable_interrupts();
    delay_ms(3000);
    printk("overflow_count: %d\n", get_overflow_count());
}
