#ifndef INTERRUPTS_H
#define INTERRUPTS_H

#include <stdint.h>

// flags
extern volatile int key_flags;
extern volatile int emergency_flag;
extern volatile int live_flag;

// init functions
void init_key_irq(void);
void tx_pulse(int base, alt_u32 flag);
void init_rx_irq(void);

#endif
