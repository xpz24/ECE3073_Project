// libraries
#include <stdio.h>
#include "../M2_core2_bsp/system.h"
#include "../M2_core2_bsp/drivers/inc/altera_avalon_pio_regs.h"
#include "../M2_core2_bsp/HAL/inc/sys/alt_irq.h"

// base addresses
#define TX_BASE CORE_2_1_TX_BASE
#define RX_BASE CORE_2_1_RX_BASE

// flags
volatile int key_flags = 0;
