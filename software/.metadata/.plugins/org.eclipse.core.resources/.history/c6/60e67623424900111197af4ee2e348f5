// libraries
#include <stdio.h>
#include "../M2_core1_bsp/system.h"
#include "../M2_core1_bsp/drivers/inc/altera_avalon_pio_regs.h"
#include "../M2_core1_bsp/HAL/inc/sys/alt_irq.h"
#include "interrupts.h"

// base addresses
#define TX_1_3_BASE CORE_1_3_TX_BASE
#define RX_1_3_BASE CORE_1_3_RX_BASE

// RX flag values — match Core 3's TX_KEY* defines
#define RX_IDLE     0x0
#define RX_KEY0     0x1
#define RX_KEY1     0x2

volatile int key_flags = 0;

static void rx_irq_handler(void *context, alt_u32 id)
{
    int val = IORD_ALTERA_AVALON_PIO_EDGE_CAP(RX_1_3_BASE) & 0x7;
    IOWR_ALTERA_AVALON_PIO_EDGE_CAP(RX_1_3_BASE, 0);

    // decode incoming RX pulse into key_flags bits
    if (val & RX_KEY0)
        key_flags |= (1 << 0);   // maps to "CAPTURING IMAGE"
    if (val & RX_KEY1)
        key_flags |= (1 << 1);   // maps to "DISPLAYING IMAGE"
}

void init_rx_irq(void)
{
    IOWR_ALTERA_AVALON_PIO_EDGE_CAP(RX_1_3_BASE, 0);
    IOWR_ALTERA_AVALON_PIO_IRQ_MASK(RX_1_3_BASE, 0x7);
    alt_irq_register(CORE_1_3_RX_IRQ, NULL, rx_irq_handler);
    alt_irq_enable_all(1);
}

void tx_pulse(int base, alt_u32 flag)
{
    IOWR_ALTERA_AVALON_PIO_DATA(base, flag);
    for (volatile int i = 0; i < 50; i++);
    IOWR_ALTERA_AVALON_PIO_DATA(base, 0);
}
