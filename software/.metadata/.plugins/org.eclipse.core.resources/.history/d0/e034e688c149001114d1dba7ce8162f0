#ifndef INTERRUPTS_H
#define INTERRUPTS_H

#include <stdint.h>

// flags
extern volatile int key_flags;
extern volatile int emergency_active;

// init functions
void init_rx_irq(void);
void tx_pulse(int base, alt_u32 flag);

#endif
