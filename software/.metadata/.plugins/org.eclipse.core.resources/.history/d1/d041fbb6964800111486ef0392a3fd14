// libraries
#include <stdio.h>
#include "../M2_core2_bsp/system.h"
#include "../M2_core2_bsp/HAL/inc/alt_types.h"
#include "../M2_core2_bsp/drivers/inc/altera_avalon_pio_regs.h"
#include "../M2_core2_bsp/HAL/inc/io.h"
#include "../M2_core2_bsp/HAL/inc/includes.h"

// forward declarations
void vga_init(void);

// main polling loop
int main(void) {
	OSInit();
	printf("Hello from CPU2\n");

	vga_init();

	OSStart();
}
