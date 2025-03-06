/*
 * Implement the following routines to set GPIO pins to input or 
 * output, and to read (input) and write (output) them.
 *  - DO NOT USE loads and stores directly: only use GET32 and 
 *    PUT32 to read and write memory.  
 *  - DO USE the minimum number of such calls.
 * (Both of these matter for the next lab.)
 *
 * See rpi.h in this directory for the definitions.
 */
#include "rpi.h"

// see broadcomm documents for magic addresses.
//
// if you pass addresses as:
//  - pointers use put32/get32.
//  - integers: use PUT32/GET32.
//  semantics are the same.
enum {
    GPIO_BASE = 0x20200000,
    gpio_set0  = (GPIO_BASE + 0x1C),
    gpio_clr0  = (GPIO_BASE + 0x28),
    gpio_lev0  = (GPIO_BASE + 0x34)

    // <you may need other values.>
};

//
// Part 1 implement gpio_set_on, gpio_set_off, gpio_set_output
//

// set <pin> to be an output pin.
//
// note: fsel0, fsel1, fsel2 are contiguous in memory, so you
// can (and should) use array calculations!
void gpio_set_output(unsigned pin) {
    if(pin >= 32 && pin != 47)
        return;
    gpio_set_function(pin, 1);

}

// set GPIO <pin> on.
void gpio_set_on(unsigned pin) {
    if(pin >= 32 && pin != 47)
        return;
  if (pin < 32) {
    PUT32(0x2020001C, (1 << pin));
  }
  else {
    PUT32(0x20200020, (1 << (pin-32)));
  }
}

// set GPIO <pin> off
void gpio_set_off(unsigned pin) {
    if(pin >= 32 && pin != 47)
        return;
  // implement this
  // use <gpio_clr0>
  if (pin < 32) {
    PUT32(0x20200028, (1 << pin));
  }
  else {
    PUT32(0x2020002C, (1 << (pin-32)));
  }
}

// set <pin> to <v> (v \in {0,1})
void gpio_write(unsigned pin, unsigned v) {
    if(v)
        gpio_set_on(pin);
    else
        gpio_set_off(pin);
}

//
// Part 2: implement gpio_set_input and gpio_read
//

// set <pin> to input.
void gpio_set_input(unsigned pin) {
  // implement.
  if(pin >= 32 && pin != 47)
        return;

  gpio_set_function(pin, 0);

}

// return the value of <pin>
int gpio_read(unsigned pin) {
  if(pin >= 32 && pin != 47)
        return -1;
  unsigned v = 0;
  // implement!
  if (pin < 32) {
    volatile unsigned value = GET32(gpio_lev0);
  v = (value >> pin) & 1;
  }
  else {
    volatile unsigned value = GET32(gpio_lev0 + 4);
    v = (value >> (pin-32)) & 1;
  }
  return DEV_VAL32(v);
}


void gpio_set_function(unsigned pin, gpio_func_t function) {
  if(pin >= 32 && pin != 47)
        return;
  if(function > 7 || function < 0) {
    return;
  }
  unsigned command = 0b000;
  if (function == 0) {
    command = 0b000;
  }
  else if (function == 1) {
    command = 0b001;
  }
  else if (function == 4) {
    command = 0b100;
  }
  else if (function == 5) {
    command = 0b101;
  }
  else if (function == 6) {
    command = 0b110;
  }
  else if (function == 7) {
    command = 0b111;
  }
  else if (function == 3) {
    command = 0b011;
  }
  else if (function == 2) {
    command = 0b010;
  }

  unsigned new_pin = pin % 10;
  unsigned new_addr = GPIO_BASE + 4 * (pin / 10);
  unsigned mask = 0b111 << new_pin*3;
  unsigned value = GET32(new_addr);
  value &= ~mask;
  value |= command << (new_pin*3);
  PUT32(new_addr, value);
    
}