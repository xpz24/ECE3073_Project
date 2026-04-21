#include <stdio.h>
#include "includes.h"
#include "adxl345.h"
#include "hex.h"
#include "altera_avalon_pio_regs.h"
#include "system.h"

//#define IRQ_CONTROLLER_ID 0

typedef struct {
	alt_16 x;
	alt_16 y;
	alt_16 z;
} accl_context_t;

// void accelerometer_isr(void *context, alt_u32 id); // Second param only exists for legacy API
void task1(void *pdata); // basic uart
void task2(void *pdata); // SPI accelerometer
void task3(void *pdata); // Display HEX

/* Definition of Task Stacks */
#define   TASK_STACKSIZE       2048
OS_STK task1_stk[TASK_STACKSIZE];
OS_STK task2_stk[TASK_STACKSIZE];
OS_STK task3_stk[TASK_STACKSIZE];

/* Definition of Task Priorities */

#define TASK1_PRIORITY      1
#define TASK2_PRIORITY      2
#define TASK3_PRIORITY		3

/* The main function creates two task and starts multi-tasking */
int main(void) {

	printf("MicroC/OS-II Licensing Terms\n");
	printf("============================\n");
	printf(
			"uC/OS-II is a portable, ROMable, scalable, preemptive, real-time deterministic \n");
	printf(
			"multitasking kernel for microprocessors, microcontrollers and DSPs.\n");
	printf("This RTOS can be used free of charge under APACHE 2.0 License.\n");
	printf(
			"for more information please see the license files included in the BSP project or contact Micrium.\n");
	printf(
			"For the complete documentation, visit https://doc.micrium.com/display/ucos/\n\n\n");

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

	accl_context_t accl_ctx;

	OSTaskCreateExt(task1,
	NULL, (void *) &task1_stk[TASK_STACKSIZE - 1],
	TASK1_PRIORITY,
	TASK1_PRIORITY, task1_stk,
	TASK_STACKSIZE,
	NULL, 0);

	OSTaskCreateExt(task2, &accl_ctx, (void *) &task2_stk[TASK_STACKSIZE - 1],
	TASK2_PRIORITY,
	TASK2_PRIORITY, task2_stk,
	TASK_STACKSIZE,
	NULL, 0);

	OSTaskCreateExt(task3,
	NULL, (void *) &task3_stk[TASK_STACKSIZE - 1],
	TASK3_PRIORITY,
	TASK3_PRIORITY, task3_stk,
	TASK_STACKSIZE,
	NULL, 0);


	OSStart();
	return 0;
}

/* Prints "Hello World" and sleeps for three seconds */
void task1(void* pdata) {
	OSStatInit();
	while (1) {
		printf("Hello from task1, every 3 seconds\n");
		OSTimeDlyHMSM(0, 0, 3, 0);
	}
}

/* Prints "Hello World" and sleeps for three seconds */
void task2(void* pdata) {

	accl_context_t *ctx = (accl_context_t *) pdata;
	int rc = 0;
	rc = adxl345_init();

	if (rc != 0) {
		printf("[ACCEL] Init failed\n");
		while (1) {
			OSTimeDlyHMSM(0, 0, 1, 0);
		}
	}
	printf("[ACCEL] Init OK\n");

	while (1) {
		rc = adxl345_read_xyz(&ctx->x, &ctx->y, &ctx->z);
		if (rc != 0) {
			printf("[ACCEL] Read failed\n");
		} else {
			printf("X=%d Y=%d Z=%d\n", ctx->x, ctx->y, ctx->z);
		}

		OSTimeDlyHMSM(0, 0, 1, 0);
	}
}

void task3(void *pdata) {
	display_char processed[100];
	int processed_len = 0;
	const char stringToDisplay[] = "Hello. how aRe you.";
	process_string(stringToDisplay, processed, &processed_len);

	int speaker = 0;
	int speed = 300;
	int scroll_pos = 0;
	int scroll_counter = 0;
	int total = processed_len + DISPLAY_WIDTH;
	int print_counter = 0;
	int prevCpuFlag = 0;

	int cpuFlag = 0;

	while (1) {
		int sw = IORD_ALTERA_AVALON_PIO_DATA(PIO_SWITCH_BASE) & 0xF;
		IOWR_ALTERA_AVALON_PIO_DATA(PIO_LED_BASE, sw & 0x7);

		if (sw & 0x1) {

			if (sw & 0x4) {
				// Implement display CPU utilization
				cpuFlag = 1;
				if (++print_counter >= 1000) { // ~1s
					print_counter = 0;
					printf("CPU Usage: %d%%\n", OSCPUUsage);
				}
			} else {
				cpuFlag = 0;
			}

			total = cpuFlag ?
					processed_len + DISPLAY_WIDTH - 2 :
					processed_len + DISPLAY_WIDTH;

			if (cpuFlag != prevCpuFlag) {
				scroll_pos = 0;
				scroll_counter = 0;
			}
			prevCpuFlag = cpuFlag;

			scroll_step(scroll_pos, processed, processed_len, cpuFlag);

			if (sw & 0x2) {
				scroll_counter++;

				if (scroll_counter >= speed) {
					scroll_counter = 0;
					scroll_pos++;

					if (scroll_pos >= total) {
						scroll_pos = 0;
					}

					if (sw & 0x8) {
						speaker = !speaker;
						IOWR_ALTERA_AVALON_PIO_DATA(PIO_SPEAKER_BASE, speaker);
					} else {
						speaker = 0;
						IOWR_ALTERA_AVALON_PIO_DATA(PIO_SPEAKER_BASE, 0);
					}
				}
			} else {
				speaker = 0;
				IOWR_ALTERA_AVALON_PIO_DATA(PIO_SPEAKER_BASE, 0);
			}

		} else {
			clear_hex();
			scroll_pos = 0;
			scroll_counter = 0;
			speaker = 0;
			IOWR_ALTERA_AVALON_PIO_DATA(PIO_SPEAKER_BASE, 0);
		}

		OSTimeDlyHMSM(0, 0, 0, 1);
	}
}
//void accelerometer_isr(void *context, alt_u32 id) {
//	accl_context_t *ctx = (accl_context_t *) context;
//	printf("ACCL Readings from IRQ\n");
//	INT32S x, y, z;
//
//	if (ctx->acclInst == NULL) {
//		printf("Failed to open accelerometer\n");
//		return;
//	}
//
//	alt_up_accelerometer_spi_read_x_axis(ctx->acclInst, &x);
//	alt_up_accelerometer_spi_read_y_axis(ctx->acclInst, &y);
//	alt_up_accelerometer_spi_read_z_axis(ctx->acclInst, &z);
//
//	printf("ACCL From IRQ:\n");
//	printf("X=%ld Y=%ld Z=%ld\n", x, y, z);
//
//	return;
//
//}

