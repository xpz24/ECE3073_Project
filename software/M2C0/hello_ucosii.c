#include <stdio.h>
#include "shared_defs.h"
#include "includes.h"
#include "adxl345.h"
#include "hex.h"
#include "altera_avalon_pio_regs.h"
#include "system.h"
#include "esp32SPI.h"
#include <string.h>
#include "custom_defs.h"
//#include "vga_controller.h"

void task1(void *pdata); // basic uart
void task2(void *pdata); // SPI accelerometer
void task3(void *pdata); // Display HEX
void task4(void *pdata); // SPI ESP32 communication
//void task5(void *pdata); // VGA

/* Definition of Task Stacks */
#define   TASK_STACKSIZE       2048
OS_STK task1_stk[TASK_STACKSIZE];
OS_STK task2_stk[TASK_STACKSIZE];
OS_STK task3_stk[TASK_STACKSIZE];
OS_STK task4_stk[TASK_STACKSIZE];
//OS_STK task5_stk[TASK_STACKSIZE];

/* Definition of Task Priorities */
#define TASK1_PRIORITY      1
#define TASK2_PRIORITY      2
#define TASK3_PRIORITY		3
#define TASK4_PRIORITY 		4
//#define TASK5_PRIORITY 		5

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

	OSTaskCreateExt(task1,
	NULL, (void *) &task1_stk[TASK_STACKSIZE - 1],
	TASK1_PRIORITY,
	TASK1_PRIORITY, task1_stk,
	TASK_STACKSIZE,
	NULL, 0);

	OSStart();
	return 0;
}

/* STARTUP TASK Prints "Hello World" and sleeps for three seconds */
void task1(void* pdata) {
	OSStatInit();
	static accl_context_t accl_ctx;

	init_frame_mutex();
	init_esp32_ready_irq();

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

	OSTaskCreateExt(task4,
	NULL, (void *) &task4_stk[TASK_STACKSIZE - 1],
	TASK4_PRIORITY,
	TASK4_PRIORITY, task4_stk,
	TASK_STACKSIZE,
	NULL, 0);

//	OSTaskCreateExt(task5,
//	NULL, (void *) &task5_stk[TASK_STACKSIZE - 1],
//	TASK5_PRIORITY,
//	TASK5_PRIORITY, task5_stk,
//	TASK_STACKSIZE,
//	NULL, 0);
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
	int speed = 3;
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

		OSTimeDlyHMSM(0, 0, 0, 100);
	}
}

void task4(void *pdata) {
	volatile alt_u8 *shared_fb = (volatile alt_u8 *) SHARED_FB_BASE_ADDR;

	printf("[ESP32 SPI] Framebuffer RX task started\n");
	printf("[ESP32 SPI] Shared FB base: 0x%08X\n", (unsigned int) shared_fb);
	printf("[ESP32 SPI] Frame size: %d bytes\n", ESP32_FB_SIZE);
	// Start with flag cleared
	IOWR_ALTERA_AVALON_PIO_DATA(PIO_NEW_FRAME_FLAG_BASE, 0);

	while (1) {
//    	int ready = IORD_ALTERA_AVALON_PIO_DATA(PIO_SPI_READY_BASE) & 0x1;
//
//    	if (!ready) {
//    	    printf("[ESP32 SPI] READY polled high\n");
//    	    esp32_frame_ready = 1;
//    	}
//    	else {
//    		esp32_frame_ready = 0;
//    		printf("LOWLOWLOW\n");
//    	}
//    	static alt_u8 test_rx_buf[ESP32_FB_SIZE];

		if (esp32_frame_ready) {
			printf("[ESP32 SPI] ESP32 ready, reading framebuffer...\n");

			esp32_frame_ready = 0;

			if (frame_mutex != NULL) {
				altera_avalon_mutex_lock(frame_mutex, FRAME_SPI_OWNER_ID);
			}

			int rc = esp32_spi_read_framebuffer((alt_u8 *) shared_fb);
//             int rc = esp32_spi_read_framebuffer(test_rx_buf);

			if (frame_mutex != NULL) {
				altera_avalon_mutex_unlock(frame_mutex);
			}

			if (rc == 0) {
				printf("[ESP32 SPI] Framebuffer copied to SDRAM\n");
				IOWR_ALTERA_AVALON_PIO_DATA(PIO_NEW_FRAME_FLAG_BASE, 1);

			} else {
				printf("[ESP32 SPI] Framebuffer read FAILED\n");
				IOWR_ALTERA_AVALON_PIO_DATA(PIO_NEW_FRAME_FLAG_BASE, 0);

			}
		}

		OSTimeDlyHMSM(0, 0, 0, 1);
	}
}

//void task5(void *pdata) {
//
//	while (1) {
//		draw_color_stripes();
//		OSTimeDlyHMSM(0, 0, 1, 0);   // show for 1 sec
//
//		draw_vertical_stripes();
//		OSTimeDlyHMSM(0, 0, 1, 0);   // show for 1 sec
//	}
//}

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

