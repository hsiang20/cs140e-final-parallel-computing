#include "rpi.h"

// defined in <interrupts-asm.S>
void disable_interrupts(void);
void enable_interrupts(void);

void interrupt_init(void);

void timer_init(uint32_t prescale, uint32_t ncycles);

void fast_interrupt_vector(unsigned pc);

void syscall_vector(unsigned pc);
void reset_vector(unsigned pc);
void undefined_instruction_vector(unsigned pc);
void prefetch_abort_vector(unsigned pc);
void data_abort_vector(unsigned pc);

void interrupt_vector(unsigned pc);

int get_overflow_count();