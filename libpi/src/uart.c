// simple mini-uart driver: implement every routine 
// with a <todo>.
//
// NOTE: 
//  - from broadcom: if you are writing to different 
//    devices you MUST use a dev_barrier().   
//  - its not always clear when X and Y are different
//    devices.
//  - pay attenton for errata!   there are some serious
//    ones here.  if you have a week free you'd learn 
//    alot figuring out what these are (esp hard given
//    the lack of printing) but you'd learn alot, and
//    definitely have new-found respect to the pioneers
//    that worked out the bcm eratta.
//
// historically a problem with writing UART code for
// this class (and for human history) is that when 
// things go wrong you can't print since doing so uses
// uart.  thus, debugging is very old school circa
// 1950s, which modern brains arne't built for out of
// the box.   you have two options:
//  1. think hard.  we recommend this.
//  2. use the included bit-banging sw uart routine
//     to print.   this makes things much easier.
//     but if you do make sure you delete it at the 
//     end, otherwise your GPIO will be in a bad state.
//
// in either case, in the next part of the lab you'll
// implement bit-banged UART yourself.
#include "rpi.h"
#include "gpio.h"

// change "1" to "0" if you want to comment out
// the entire block.
#if 1
//*****************************************************
// We provide a bit-banged version of UART for debugging
// your UART code.  delete when done!
//
// NOTE: if you call <emergency_printk>, it takes 
// over the UART GPIO pins (14,15). Thus, your UART 
// GPIO initialization will get destroyed.  Do not 
// forget!   

// header in <libpi/include/sw-uart.h>
#include "sw-uart.h"
static sw_uart_t sw_uart;

// if we've ever called emergency_printk better
// die before returning.
static int called_sw_uart_p = 0;

// a sw-uart putc implementation.
static int sw_uart_putc(int chr) {
    sw_uart_put8(&sw_uart,chr);
    return chr;
}

// call this routine to print stuff. 
//
// note the function pointer hack: after you call it 
// once can call the regular printk etc.
static void emergency_printk(const char *fmt, ...) {
    // track if we ever called it.
    called_sw_uart_p = 1;


    // we forcibly initialize each time it got called
    // in case the GPIO got reset.
    // setup gpio 14,15 for sw-uart.
    sw_uart = sw_uart_default();

    // all libpi output is via a <putc>
    // function pointer: this installs ours
    // instead of the default
    rpi_putchar_set(sw_uart_putc);

    printk("NOTE: HW UART GPIO is in a bad state now\n");

    // do print
    va_list args;
    va_start(args, fmt);
    vprintk(fmt, args);
    va_end(args);

}

#undef todo
#define todo(msg) do {                      \
    emergency_printk("%s:%d:%s\nDONE!!!\n",      \
            __FUNCTION__,__LINE__,msg);   \
    rpi_reboot();                           \
} while(0)

// END of the bit bang code.
#endif

enum {  AUX_EN = 0x20215004, 
        AUX_MU_CNTL = 0x20215060, 
        AUX_MU_IER = 0x20215044, 
        AUX_MU_IIR = 0x20215048, 
        AUX_MU_LCR = 0x2021504C, 
        AUX_MU_BAUD = 0x20215068, 
        AUX_MU_LSR = 0x20215054, 
        AUX_MU_STAT = 0x20215064, 
        AUX_MU_IO = 0x20215040, 
        AUX_MU_MCR = 0x20215050
        };

//*****************************************************
// the rest you should implement.

// called first to setup uart to 8n1 115200  baud,
// no interrupts.
//  - you will need memory barriers, use <dev_barrier()>
//
//  later: should add an init that takes a baud rate.
void uart_init(void) {
    // NOTE: make sure you delete all print calls when
    // done!
    // emergency_printk("start here\n");
    // printk("write UART addresses in order\n");

    dev_barrier();

    // turn off UART
    // unsigned aux_en = GET32(AUX_EN);
    // aux_en &= ~(0b1);
    // PUT32(AUX_EN, aux_en);
    // dev_barrier();
    // printk("uart turned off");

    // set GPIO
    gpio_set_function(GPIO_TX, GPIO_FUNC_ALT5);
    gpio_set_function(GPIO_RX, GPIO_FUNC_ALT5);
    dev_barrier();
    // printk("gpio set\n");

    // turn on UART in AUX
    unsigned aux_en = GET32(AUX_EN);
    aux_en |= 0b1;
    PUT32(AUX_EN, aux_en);
    dev_barrier();
    // printk("uart turned on\n");

    // disable TX/RX
    // unsigned aux_mu_cntl = GET32(AUX_MU_CNTL);
    // aux_mu_cntl &= ~(0b11);
    unsigned aux_mu_cntl = 0;
    PUT32(AUX_MU_CNTL, aux_mu_cntl);
    // printk("tx/rx disabled\n");

    // clear receive/transmit FIFO
    // unsigned aux_mu_iir = GET32(AUX_MU_IIR);
    // aux_mu_iir |= 0b110;
    unsigned aux_mu_iir = 0b110;
    PUT32(AUX_MU_IIR, aux_mu_iir);
    
    // disable interrupt
    // unsigned aux_mu_ier = GET32(AUX_MU_IER);
    // aux_mu_ier &= ~(0b11);
    unsigned aux_mu_ier = 0b00;
    PUT32(AUX_MU_IER, aux_mu_ier);

    // configure baud
    // set data size to 3 (8 bit)
    // unsigned aux_mu_lcr = GET32(AUX_MU_LCR);
    // aux_mu_lcr |= 0b11;
    unsigned aux_mu_lcr = 0b11;
    PUT32(AUX_MU_LCR, aux_mu_lcr);
    // I don't know why we need to set this
    unsigned aux_mu_mcr = 0;
    PUT32(AUX_MU_MCR, aux_mu_mcr);
    // set baudrate to 115200 -> set the reg to 270 = 0b100001110
    unsigned baudrate = 270;
    PUT32(AUX_MU_BAUD, baudrate);


    // enable tx/rx
    // aux_mu_cntl = GET32(AUX_MU_CNTL);
    // aux_mu_cntl |= 0b11;
    aux_mu_cntl = 0b11;
    PUT32(AUX_MU_CNTL, aux_mu_cntl);
    dev_barrier();

    // perhaps confusingly: at this point normal printk works
    // since we overrode the system putc routine.

    // todo("must implement\n");

    // delete everything to do w/ sw-uart when done since
    // it trashes your hardware state and the system
    // <putc>.
    demand(!called_sw_uart_p, 
        delete all sw-uart uses or hw UART in bad state);
}

// disable the uart: make sure all bytes have been
// 
void uart_disable(void) {
    // todo("must implement\n");
    uart_flush_tx();
    unsigned aux_en = GET32(AUX_EN);
    aux_en &= ~(0b1);
    PUT32(AUX_EN, aux_en);
}

// returns one byte from the RX (input) hardware
// FIFO.  if FIFO is empty, blocks until there is 
// at least one byte.
int uart_get8(void) {
    // todo("must implement\n");
    dev_barrier();
    while (!uart_has_data()) {}
    unsigned mu_io = GET32(AUX_MU_IO);
    unsigned io_mask = 0b11111111;
    unsigned data = mu_io & io_mask;
    dev_barrier();
    return data;
}

// returns 1 if the hardware TX (output) FIFO has room
// for at least one byte.  returns 0 otherwise.
int uart_can_put8(void) {
    // todo("must implement\n");
    unsigned aux_mu_lsr = GET32(AUX_MU_LSR);
    unsigned has_room_mask = 0b100000;
    // has_room is 1 if bit 1 is 1, which means it is not full
    unsigned has_room = (aux_mu_lsr & has_room_mask);
    if (has_room == 0) {
        return 0;
    }
    else {
        return 1;
    }
}

// put one byte on the TX FIFO, if necessary, waits
// until the FIFO has space.
int uart_put8(uint8_t c) {
    // todo("must implement\n");
    dev_barrier();
    while (!uart_can_put8()) {}
    // unsigned mu_io = GET32(AUX_MU_IO);
    // unsigned io_mask = 0b11111111;
    // mu_io &= ~(io_mask);
    // mu_io |= c;
    PUT32(AUX_MU_IO, c);
    dev_barrier();
    return 1;
}

// returns:
//  - 1 if at least one byte on the hardware RX FIFO.
//  - 0 otherwise
int uart_has_data(void) {
    // todo("must implement\n");
    unsigned aux_mu_lsr = GET32(AUX_MU_LSR);
    unsigned has_data_mask = 0b1;
    // has_data is 1 if bit 0 is 1, which means there is symbol in receive fifo
    unsigned has_data = (aux_mu_lsr & has_data_mask);
    if (has_data == 0) {
        return 0;
    }
    else {
        return 1;
    }
}

// returns:
//  -1 if no data on the RX FIFO.
//  otherwise reads a byte and returns it.
int uart_get8_async(void) { 
    if(!uart_has_data())
        return -1;
    return uart_get8();
}

// returns:
//  - 1 if TX FIFO empty AND idle.
//  - 0 if not empty.
int uart_tx_is_empty(void) {
    // todo("must implement\n");
    unsigned aux_mu_lsr = GET32(AUX_MU_LSR);
    unsigned tx_done_mask = 0b1000000;
    // if tx_done is not 0, it means transmitter is done
    unsigned tx_done = aux_mu_lsr & tx_done_mask;
    if (tx_done == 0) {
        return 0;
    }
    else {
        return 1;
    }
}

// return only when the TX FIFO is empty AND the
// TX transmitter is idle.  
//
// used when rebooting or turning off the UART to
// make sure that any output has been completely 
// transmitted.  otherwise can get truncated 
// if reboot happens before all bytes have been
// received.
void uart_flush_tx(void) {
    while(!uart_tx_is_empty())
        rpi_wait();
}
