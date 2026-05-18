#ifndef INTERRUPTS_H
#define INTERRUPTS_H

#include <stdint.h>

// flags
extern volatile int key_flags;
extern volatile int emergency_flag;
extern volatile int tilt_flag;
extern volatile int capture_flag;

// tilting
#define TILT_NORMAL    0x4
#define TILT_LEFT      0x8
#define TILT_RIGHT     0xC
#define TILT_FORWARD   0x10
#define TILT_BACKWARD  0x20

// init functions
void init_rx_3_2_irq(void);
void init_rx_1_2_irq(void);

#endif
