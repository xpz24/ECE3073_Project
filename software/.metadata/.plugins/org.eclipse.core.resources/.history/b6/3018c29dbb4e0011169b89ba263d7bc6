// libraries
#include <stdio.h>
#include "../M2_core2_bsp/system.h"
#include "../M2_core2_bsp/HAL/inc/alt_types.h"
#include "../M2_core2_bsp/drivers/inc/altera_avalon_pio_regs.h"
#include "../M2_core2_bsp/HAL/inc/io.h"
#include "../M2_core2_bsp/HAL/inc/includes.h"
#include "interrupts.h"

// forward declarations
void vga_init(void);

// start task
#define START_PRIORITY  1
#define START_STACKSIZE 2048
OS_STK start_stk[START_STACKSIZE];

void start_task(void* pdata) {
    printf("Hello from CPU2\n");

    OSTimeDly(2);

    init_rx_3_2_irq();
	init_rx_1_2_irq();
	alt_irq_enable_all(1);

	vga_init();

    printf("Start task deleting itself\n");
    OSTaskDel(OS_PRIO_SELF);
}

// main
int main(void) {
    OSInit();
    OSTaskCreateExt(start_task,
                    NULL,
                    (void*)&start_stk[START_STACKSIZE - 1],
                    START_PRIORITY,
                    START_PRIORITY,
                    start_stk,
                    START_STACKSIZE,
                    NULL,
                    0);
    OSStart();
}
