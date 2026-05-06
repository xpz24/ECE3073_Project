#include "alt_types.h"
#include "system.h"

typedef struct {
	alt_16 x;
	alt_16 y;
	alt_16 z;
} accl_context_t;

//#define IRQ_CONTROLLER_ID 0
// void accelerometer_isr(void *context, alt_u32 id); // Second param only exists for legacy API
//	alt_up_accelerometer_spi_dev *accl;
	//	accl = alt_up_accelerometer_spi_open_dev(ACCELEROMETER_SPI_0_NAME);
	//	alt_up_accelerometer_spi_write(accl, 0x2E, 0x80); // Enable DATA_READY Interrupt
	//	alt_up_accelerometer_spi_write(accl, 0x2F, 0x00); // Enable INT1

// Setup IRQ
	//	accl_ctx.acclInst = accl;
	// Enhanced API
	//	alt_ic_isr_register(IRQ_CONTROLLER_ID, ACCELEROMETER_SPI_0_IRQ,
	//			accelerometer_isr, &accl_ctx, NULL);
	// Legacy API
	//	alt_irq_register(ACCELEROMETER_SPI_0_IRQ, &accl_ctx, accelerometer_isr);
