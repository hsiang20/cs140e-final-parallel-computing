// engler, cs140 put your gpio-int implementations in here.
#include "rpi.h"

// in libpi/include: has useful enums.
#include "rpi-interrupts.h"

enum {
    GPLEV0 = 0x20200034, 
    GPEDS0 = 0x20200040,
    GPREN0 = 0x2020004C, 
    GPFEN0 = 0x20200058, 
    GPHEN0 = 0x20200064, 
    IRQPE2 = 0x2000B208,
    ENIRQ2 = 0x2000B214 
};


// returns 1 if there is currently a GPIO_INT0 interrupt, 
// 0 otherwise.
//
// note: we can only get interrupts for <GPIO_INT0> since the
// (the other pins are inaccessible for external devices).
int gpio_has_interrupt(void) {
    // todo("implement: is there a GPIO_INT0 interrupt?\n");
    dev_barrier();
    unsigned int irqpe2 = GET32(IRQPE2);
    dev_barrier();
    irqpe2 &= 1 << (49 - 32);
    if (irqpe2 == 0) {
        return 0;
    }
    else {
        return 1;
    }
}

// p97 set to detect rising edge (0->1) on <pin>.
// as the broadcom doc states, it  detects by sampling based on the clock.
// it looks for "011" (low, hi, hi) to suppress noise.  i.e., its triggered only
// *after* a 1 reading has been sampled twice, so there will be delay.
// if you want lower latency, you should us async rising edge (p99)
void gpio_int_rising_edge(unsigned pin) {
    if(pin>=32)
        return;
    // todo("implement: detect rising edge\n");
    dev_barrier();
    OR32(GPREN0, 1 << pin);
    dev_barrier();
    PUT32(ENIRQ2, 1 << (49 - 32));
    dev_barrier();
}

// p98: detect falling edge (1->0).  sampled using the system clock.  
// similarly to rising edge detection, it suppresses noise by looking for
// "100" --- i.e., is triggered after two readings of "0" and so the 
// interrupt is delayed two clock cycles.   if you want  lower latency,
// you should use async falling edge. (p99)
void gpio_int_falling_edge(unsigned pin) {
    if(pin>=32)
        return;
        
    dev_barrier();
    // todo("implement: detect falling edge\n");
    OR32(GPFEN0, 1 << pin);
    dev_barrier();
    PUT32(ENIRQ2, 1 << (49 - 32));
    dev_barrier();
}

// p96: a 1<<pin is set in EVENT_DETECT if <pin> triggered an interrupt.
// if you configure multiple events to lead to interrupts, you will have to 
// read the pin to determine which caused it.
int gpio_event_detected(unsigned pin) {
    if(pin>=32)
        return 0;
    dev_barrier();
    // todo("implement: is an event detected?\n");
    unsigned int event = GET32(GPEDS0) & (1 << pin);
    dev_barrier();
    if (event != 0) {
        return 1;
    }
    else {
        return 0;
    }
}

// p96: have to write a 1 to the pin to clear the event.
void gpio_event_clear(unsigned pin) {
    if(pin>=32)
        return;
    // todo("implement: clear event on <pin>\n");
    dev_barrier();
    PUT32(GPEDS0, 1 << pin);
    dev_barrier();
}
